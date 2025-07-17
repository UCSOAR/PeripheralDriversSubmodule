#include <CanAutoNodeDaughter.hpp>
#include <cstring>


//on startup:
	// broadcast on decided ID to request ID range
	// if preempted by something else, try retransmit
	// if bus fault/conflict, ?????? idk hehe


CanAutoNodeDaughter::CanAutoNodeDaughter(FDCanController *contr, uint16_t msgIDsToRequestStartID, uint16_t msgIDsToRequestAmount)
{

	controller = contr;
	thisNode = Node{{msgIDsToRequestStartID,msgIDsToRequestAmount},HAL_GetDEVID()};
}

const CanAutoNodeDaughter::daughterState CanAutoNodeDaughter::GetCurrentState() const {
	return currentState;
}


bool CanAutoNodeDaughter::TryRequestingJoiningNetwork() {

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
bool CanAutoNodeDaughter::RequestToJoinNetwork(uint16_t requestID) {

	uint8_t msg[12] = {0};
	msgFromNode(thisNode, msg);

	if(controller->SendByMsgID(msg, sizeof(msg), requestID)) {
		return true;
	} else {
		return false;
	}

}

CanAutoNodeDaughter::~CanAutoNodeDaughter() {

}

// an acknowledgeent looks like this:
// 8 bits: acknowledgementStatus enum
//


bool CanAutoNodeDaughter::CheckForAcknowledgement() {
	uint8_t msg[64] = {123};
	uint32_t id = 0;
	while(controller->GetRxFIFO(msg, &id) == HAL_OK) {
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



bool CanAutoNodeDaughter::CheckForUpdate() {

	uint8_t msg[64] = {123};
	uint32_t id = 0;
	while(controller->GetRxFIFO(msg, &id) == HAL_OK) {
		if(id != UPDATE_ID) {
			continue;
		}

		return ReceiveUpdate(msg);

	}
	// not received
	return false;

}


bool CanAutoNodeDaughter::ReceiveUpdate(uint8_t *msg) {

	Node receivedNode = nodeFromMsg(msg+1);

	this->nodes[this->nodesInNetwork++] = receivedNode;

	if(msg[0] == 1) {
		currentState = READY;
	}

	return true;

}


