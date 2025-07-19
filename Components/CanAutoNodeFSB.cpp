#include <CanAutoNodeFSB.hpp>
#include <cstring>

bool CanAutoNodeFSB::KickNode(uint32_t uniqueBoardID) {

	bool exists = false;
	uint16_t foundIndex = 0;

	for(uint16_t i = 0; i < nodesInNetwork; i++) {
		const Node& thisNode = daughterNodes[i];
		if(thisNode.uniqueID == uniqueBoardID) {
			exists = true;
			foundIndex = i;
			break;
		}
	}

	if(!exists) {
		return false;
	}

	daughterNodes[foundIndex] = daughterNodes[--nodesInNetwork];

	if(!controller->SendByMsgID((uint8_t*)(&uniqueBoardID), sizeof(uniqueBoardID), KICK_REQUEST_ID)) {
		return false;
	}
	return SendFullUpdate();
}

bool CanAutoNodeFSB::CheckForJoinRequest() {

	uint8_t msg[64] = {123};
	uint32_t id = 0;
	while(controller->GetRxFIFO(msg, &id) == HAL_OK) {
		if(id != JOIN_REQUEST_ID) {
			continue;
		}

		return ReceiveJoinRequest(msg);

	}
	// not received
	return false;

}

bool CanAutoNodeFSB::ReceiveJoinRequest(uint8_t* msg) {

	Node newNode = nodeFromMsg(msg);

	if(newNode.requestedRange.end <= newNode.requestedRange.start) {
		return false;
	}


	for(uint16_t i = 0; i < nodesInNetwork; i++) {
		Node thisAlreadyExistingNode = daughterNodes[i];

		if(newNode.uniqueID == thisAlreadyExistingNode.uniqueID) {
			// already have this board somehow
			SendAck(ACK_BOARD_ALREADY_EXISTS);
			return false;
		}

		if(IDRangesOverlap(newNode.requestedRange, thisAlreadyExistingNode.requestedRange) ||
			IDRangesOverlap(newNode.requestedRange, thisNode.requestedRange)) {
			// ID conflict
			SendAck(ACK_REQUESTED_IDS_TAKEN);
			return false;
		}

	}

	// found no issues
	SendAck(ACK_GOOD);
	daughterNodes[nodesInNetwork] = newNode;
	nodesInNetwork++;
	SendFullUpdate();
	return true;

}

bool CanAutoNodeFSB::SendAck(acknowledgementStatus status) {

	uint8_t msg[] = {static_cast<uint8_t>(status)};
	return controller->SendByMsgID(msg, sizeof(msg), ACK_ID);

}


// update same format as request format, but with 1 byte in front which is:
//0 if still more, 1 if done, 2 if FSB ID
bool CanAutoNodeFSB::SendFullUpdate() {

	uint8_t msg[13];
	msgFromNode(thisNode, msg+1);
	msg[0] = CAN_UPDATE_FSB;

	if(!controller->SendByMsgID(msg, sizeof(msg), UPDATE_ID)) {
		return false;
	}

	for(uint16_t i = 0; i < nodesInNetwork; i++) {
		msgFromNode(daughterNodes[i], msg+1);
		msg[0] = (i == nodesInNetwork-1) ? CAN_UPDATE_LAST_DAUGHTER : CAN_UPDATE_DAUGHTER;
		if(!controller->SendByMsgID(msg, sizeof(msg), UPDATE_ID)) {
			return false;
		}
	}

	return true;

}

bool CanAutoNodeFSB::Heartbeat() {


	if(!SendHeartbeat()) {
		return false;
	}

	bool received[nodesInNetwork];
	memset(received,0,sizeof(received));

	for(uint16_t i = 0; i < 1000; i++) {
		HAL_Delay(1);
		uint8_t out[64];
		uint32_t id = 0;
		if(controller->GetRxFIFO(out, &id) == HAL_OK) {
			switch(id) {
			case HEARTBEAT_ID: {
				uint32_t responseID = shift8to32(out);
				bool foundResponder = false;
				for(int node = 0; node < nodesInNetwork; node++) {
					if(daughterNodes[node].uniqueID == responseID) {
						if(received[node]) {
							// received multiple heartbeats from the same node?
							KickNode(responseID);
						}
						received[node] = true;
						foundResponder = true;
						break;
					}
				}
				if(!foundResponder) {
					// ??? a node that wasn't in the network just responded to the heartbeat?  get out
					KickNode(responseID);
				}
				break;
			}
			default:
				break;
			}
		}
	}

	for(size_t i = 0; i < nodesInNetwork; i++) {
		if(!received[i]) { // Found a node that never responded to the heartbeat
			KickNode(daughterNodes[i].uniqueID);
		}
	}

	return true;

}


