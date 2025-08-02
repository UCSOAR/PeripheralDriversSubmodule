#include <CanAutoNodeDaughter.hpp>
#include <cstring>


//on startup:
	// broadcast on decided ID to request ID range
	// if preempted by something else, try retransmit
	// if bus fault/conflict, ?????? idk hehe


//CanAutoNodeDaughter::CanAutoNodeDaughter(FDCanController *contr, uint16_t msgIDsToRequestStartID, uint16_t msgIDsToRequestAmount)
//{
//	controller = contr;
//	thisNode = Node{{msgIDsToRequestStartID,msgIDsToRequestAmount},HAL_GetDEVID()};
//}

const CanAutoNodeDaughter::daughterState CanAutoNodeDaughter::GetCurrentState() const {
	return currentState;
}

bool CanAutoNodeDaughter::TryRequestingJoiningNetwork() {

	srand(HAL_GetTick());
	uint16_t tries = 0;

	while(1) {
		if(!RequestToJoinNetwork(JOIN_REQUEST_ID)) {
			HAL_Delay(rand()%500+100);
			tries++;
			if(tries > 8) {
				ChangeState(ERROR);
				return false;
			}
			ChangeState(REQUESTED_FAILED_WAITING_TO_RETRY);
		} else {
			ChangeState(REQUESTED_WAITING_FOR_RESPONSE);
			HAL_Delay(rand()%500+100);
			if(CheckForAcknowledgement()) {
				return true;
			} else {
				ChangeState(REQUESTED_FAILED_WAITING_TO_RETRY);
			}
		}
	}

}

// request format:
// 32 bits unique board ID
// 32 bits starting index of msgs this board intends to send in
// 32 bits ending index
bool CanAutoNodeDaughter::RequestToJoinNetwork(uint16_t requestID) {


	JoinRequest request;
	request.uniqueID = thisNode.uniqueID;
	request.boardType = 123;
	request.slotNumber = 123;
	request.numberOfLogs = numLogs;
	for(uint16_t i = 0; i < numLogs; i++) {
		request.logSizesInBytes[i] = (logsToInit[i].sizeInBytes);
	}

	uint8_t msg[sizeof(JoinRequest)];
	DataToMsg<JoinRequest>(request, msg);

	return controller->SendByMsgID(msg, sizeof(msg), JOIN_REQUEST_ID);
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
			ChangeState(WAITING_FOR_UPDATE);
			return true;

		case ACK_NO_ROOM:
			ChangeState(ERROR);
			return false;

		default:
			// invalid ack
			ChangeState(ERROR);
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
	if(HAL_GetTick() - tickLastReceivedUpdatePart > 1000) {
		ChangeState(ERROR);
	}
	return false;

}

bool CanAutoNodeDaughter::ProcessMessage() {
	uint8_t msg[64] = {123};
	uint32_t id = 0;
	bool gotOne = false;
	while(controller->GetRxFIFO(msg, &id) == HAL_OK) {
		gotOne = true;
		switch(id) {
		case KICK_REQUEST_ID: {
			uint32_t kickedBoard = MsgToData<uint32_t>(msg);
			if(kickedBoard == this->thisNode.uniqueID) {
				ChangeState(UNINITIALIZED);
				return true;
			}
		}
		case JOIN_REQUEST_ID:
		{
			ChangeState( WAITING_FOR_UPDATE);
			return true;
		}

		case HEARTBEAT_ID: {
			if(shift8to32(msg) == FSB.uniqueID) {
				SendHeartbeat();
				return true;
			}
		}
		}

	}
	return gotOne;
}

void CanAutoNodeDaughter::ChangeState(daughterState target) {
	currentState = target;
	switch(target) {
	case WAITING_FOR_UPDATE:
		tickLastReceivedUpdatePart = HAL_GetTick();
		break;

	case UNINITIALIZED:
		initializedLogs = false;
		nodesInNetwork = 0;
		thisNode.canIDRange = {0,0};
		break;

	default:
		break;
	}
}

CanAutoNodeDaughter::CanAutoNodeDaughter(FDCAN_HandleTypeDef *fdcan, LogInit *logs,
		uint16_t numLogs, uint8_t boardType) : boardType(boardType) {
	controller = new FDCanController(fdcan,nullptr,0);

	memcpy(logsToInit,logs,numLogs*sizeof(LogInit));
	this->numLogs = numLogs;
	callbackcontroller = controller;
	currentState = UNINITIALIZED;
	this->thisNode = {{0,0},HAL_GetDEVID()};

}

bool CanAutoNodeDaughter::SendMessageToFSBByLogID(uint16_t logID, const uint8_t *msg) {
	return controller->SendByLogIndex(msg, logID);
}

bool CanAutoNodeDaughter::ReceiveUpdate(uint8_t *msg) {

	tickLastReceivedUpdatePart = HAL_GetTick();

	Node receivedNode = nodeFromMsg(msg+1);

	switch(msg[0]) {

	case CAN_UPDATE_LAST_DAUGHTER: // shouldn't have to worry about last and fsb being the same because the update will only be sent after a new node is added
		ChangeState(READY);
	case CAN_UPDATE_DAUGHTER:
		if(receivedNode != thisNode) {
			this->daughterNodes[this->nodesInNetwork++] = receivedNode;
		} else {
			this->thisNode = receivedNode;
			if(!initializedLogs) {
				uint16_t canid = receivedNode.canIDRange.start;
				for(uint16_t i = 0; i < numLogs; i++) {
					uint16_t requiredIDs = (logsToInit[i].sizeInBytes-1)/64+1;
					determinedLogs[i] = {logsToInit[i].sizeInBytes, canid};
					canid += requiredIDs;
				}
				controller->RegisterLogs(determinedLogs, numLogs);
				initializedLogs = true;
			}

		}
		break;

	case CAN_UPDATE_FSB:
		this->FSB = receivedNode;
		break;

	default:
		return false;
	}

	return true;

}
