#include "CanAutoNode.hpp"
#include <cstring>


//on startup:
	// broadcast on decided ID to request ID range
	// if preempted by something else, try retransmit
	// if bus fault/conflict, ?????? idk hehe

CanAutoNode::CanAutoNode(FDCanController &contr, uint16_t msgIDsToRequestStartID, uint16_t msgIDsToRequestAmount)
: controller(contr), idRange(msgIDsToRequestStartID,msgIDsToRequestAmount) {

}

const CanAutoNode::state CanAutoNode::GetCurrentState() const {
	return currentState;
}


bool CanAutoNode::TryRequestingJoiningNetwork() {

	srand(HAL_GetTick());
	uint16_t tries = 0;

	while(1) {
		if(!RequestToJoinNetwork(JOIN_REQUEST_ID)) {
			HAL_Delay(rand()%500);
			tries++;
			if(tries > 8) {
				currentState = ERROR;
				return false;
			}
			currentState = REQUESTED_FAILED_WAITING_TO_RETRY;
		} else {
			currentState = REQUESTED_WAITING_FOR_RESPONSE;
			return true;
		}
	}

}

// request format:
// 32 bits unique board ID
// 32 bits starting index of msgs this board intends to send in
// 32 bits ending index
bool CanAutoNode::RequestToJoinNetwork(uint16_t requestID) {


	uint8_t msg[12] = {0};
	memcpy(msg,&uniqueBoardID,sizeof(uniqueBoardID));
	memcpy(msg+4, &idRange.start, sizeof(idRange.start));
	memcpy(msg+8, &idRange.end, sizeof(idRange.end));

	if(controller.SendByMsgID(msg, sizeof(msg), requestID)) {
		return true;
	} else {
		return false;
	}

}

CanAutoNode::~CanAutoNode() {
}

// an acknowledgeent looks like this:
// 8 bits: acknowledgementStatus enum
//


bool CanAutoNode::CheckForAcknowledgement() {
	uint8_t msg[64] = {123};
	uint32_t id = 0;
	while(controller.GetRxFIFO(msg, &id) == HAL_OK) {
		if(id != ACK_ID) {
			continue;
		}

		acknowledgementStatus incomingStatus = static_cast<acknowledgementStatus>(msg[0]);
		switch(incomingStatus) {
		case ACK_GOOD:
			currentState = ACKED_WAITING_FOR_UPDATE;
			return true;

		case ACK_REQUESTED_IDS_TAKEN:
			currentState = ERROR;
			return false;

		default:
			// invalid ack
			currentState = ERROR;
			return false;
		}

	}
	// not received
	return false;
}

bool CanAutoNode::ReceiveJoinRequest(uint8_t* msg) {

	Node newNode;
	newNode.uniqueID = shift8to32(msg);
	newNode.requestedRange.start = shift8to32(msg+4);
	newNode.requestedRange.end = shift8to32(msg+8);

	if(newNode.requestedRange.end <= newNode.requestedRange.start) {
		return false;
	}


	for(uint16_t i = 0; i < nodesInNetwork; i++) {
		Node thisAlreadyExistingNode = nodes[i];

		if(newNode.uniqueID == thisAlreadyExistingNode.uniqueID) {
			// already have this board somehow
			SendAck(ACK_BOARD_ALREADY_EXISTS);
			return false;
		}

		if(IDRangesOverlap(newNode.requestedRange, thisAlreadyExistingNode.requestedRange) ||
				IDRangesOverlap(newNode.requestedRange, this->idRange)) {
			// ID conflict
			SendAck(ACK_REQUESTED_IDS_TAKEN);
			return false;
		}

	}



	// found no issues
	SendAck(ACK_GOOD);
	nodes[nodesInNetwork] = newNode;
	nodesInNetwork++;
	return true;

}


uint32_t CanAutoNode::shift8to32(const uint8_t *in) {
	return (static_cast<uint32_t>(in[0])<<24) | (static_cast<uint32_t>(in[1])<<16) | (static_cast<uint32_t>(in[2])<<8) | in[3];
}

bool CanAutoNode::SendAck(acknowledgementStatus status) {

	uint8_t msg[] = {static_cast<uint8_t>(status)};
	return controller.SendByMsgID(msg, sizeof(msg), ACK_ID);

}

uint16_t CanAutoNode::shift8to16(const uint8_t *in) {
	return (static_cast<uint16_t>(in[0])<<8) | in[1];
}

// update same format as request format, but with 1 byte in front which is 0 if still more, 1 if done
bool CanAutoNode::SendFullUpdate() {

	uint8_t msg[13];
	memcpy(msg+1,&this->uniqueBoardID,sizeof(this->uniqueBoardID));
	memcpy(msg+5,&this->idRange.start,sizeof(this->idRange.start));
	memcpy(msg+9,&this->idRange.end,sizeof(this->idRange.end));

	msg[0] = nodesInNetwork == 0;

	if(!controller.SendByMsgID(msg, sizeof(msg), UPDATE_ID)) {
		return false;
	}

	for(uint16_t i = 0; i < nodesInNetwork; i++) {
		msgFromNode(nodes[i], msg+1);
		msg[0] = (i == nodesInNetwork-1);
		if(!controller.SendByMsgID(msg, sizeof(msg), UPDATE_ID)) {
			return false;
		}
	}

	return true;

}

bool CanAutoNode::CheckForJoinRequest() {

	uint8_t msg[64] = {123};
	uint32_t id = 0;
	while(controller.GetRxFIFO(msg, &id) == HAL_OK) {
		if(id != JOIN_REQUEST_ID) {
			continue;
		}

		return ReceiveJoinRequest(msg);

	}
	// not received
	return false;

}

bool CanAutoNode::CheckForUpdate() {

	uint8_t msg[64] = {123};
	uint32_t id = 0;
	while(controller.GetRxFIFO(msg, &id) == HAL_OK) {
		if(id != UPDATE_ID) {
			continue;
		}

		return ReceiveUpdate(msg);

	}
	// not received
	return false;

}

bool CanAutoNode::IDRangesOverlap(IDRange a, IDRange b) {
	return a.start < b.end && a.end > b.start;
}

CanAutoNode::Node CanAutoNode::nodeFromMsg(const uint8_t *msg) {

	Node newNode;
	newNode.uniqueID = shift8to32(msg);
	newNode.requestedRange.start = shift8to32(msg+4);
	newNode.requestedRange.end = shift8to32(msg+8);
	return newNode;

}

bool CanAutoNode::ReceiveUpdate(uint8_t *msg) {

	Node receivedNode = nodeFromMsg(msg+1);

	this->nodes[this->nodesInNetwork++] = receivedNode;

	if(msg[0] == 1) {
		currentState = READY;
	}

	return true;

}

void CanAutoNode::msgFromNode(Node node, uint8_t *msgout) {

	memcpy(msgout,&node.uniqueID,sizeof(node.uniqueID));
	memcpy(msgout+4,&node.requestedRange.start,sizeof(node.requestedRange.start));
	memcpy(msgout+8,&node.requestedRange.end,sizeof(node.requestedRange.end));

}
