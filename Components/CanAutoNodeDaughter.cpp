#include <CanAutoNodeDaughter.hpp>
#include <cstring>




/* Gets the state of this daughter board.
 * @return The current state.
 */
const CanAutoNodeDaughter::daughterState CanAutoNodeDaughter::GetCurrentState() const {
	return currentState;
}

/* Attempt to join the network. Will block until all attempts fail or an acknowledgment is received.
 * @return true if a positive acknowledgment is received.
 */
bool CanAutoNodeDaughter::TryRequestingJoiningNetwork() {

	if(GetCurrentState() != UNINITIALIZED) {
		return false;
	}
	srand(GetThisBoardUniqueID().u1);
	uint16_t tries = 0;

	while(1) {
		if(!RequestToJoinNetwork()) {
#ifdef CANAUTONODEDEBUG
		SOAR_PRINT("Failed try %d/%d\n",tries+1,MAX_JOIN_ATTEMPTS);
#endif
			HAL_Delay(rand()%500+100);
			tries++;
			if(tries >= MAX_JOIN_ATTEMPTS) {
				ChangeState(ERROR);
				return false;
			}
			ChangeState(REQUESTED_FAILED_WAITING_TO_RETRY);
		} else {
#ifdef CANAUTONODEDEBUG
		SOAR_PRINT("Successfully sent request\n");
#endif
			ChangeState(REQUESTED_WAITING_FOR_RESPONSE);
			HAL_Delay(rand()%500+100);
			if(CheckForAcknowledgement()) {
#ifdef CANAUTONODEDEBUG
		SOAR_PRINT("Received good ACK\n");
#endif
				return true;
			} else {
#ifdef CANAUTONODEDEBUG
		SOAR_PRINT("Non good ACK received\n");
#endif
				ChangeState(REQUESTED_FAILED_WAITING_TO_RETRY);
			}
		}
	}

}

/* Send a request to join the network on the reserved join request CAN ID.
 * @return true if request successfully sent.
 */
bool CanAutoNodeDaughter::RequestToJoinNetwork() {

	JoinRequest request;
	request.uniqueID = thisNode.uniqueID;
	request.boardType = thisNode.boardType;
	request.slotNumber = thisNode.slotNumber;
	request.numberOfLogs = numLogs;
	for(uint16_t i = 0; i < numLogs; i++) {
		request.logSizesInBytes[i] = (logsToInit[i].sizeInBytes);
	}
	strcpy(request.nodeName,thisNode.nodeName);

	uint8_t msg[sizeof(JoinRequest)];
	DataToMsg<JoinRequest>(request, msg);

	return controller->SendByMsgID(msg, sizeof(msg), JOIN_REQUEST_ID);
}

CanAutoNodeDaughter::~CanAutoNodeDaughter() {

}


/* Exhausts the FIFO until any acknowledgment is received.
 * return true if a good acknowledgment is found.
 */
bool CanAutoNodeDaughter::CheckForAcknowledgement() {
	if(GetCurrentState() != REQUESTED_WAITING_FOR_RESPONSE) {
		return false;
	}
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
#ifdef CANAUTONODEDEBUG
		SOAR_PRINT("No room!\n");
#endif
			return false;

		default:
			// invalid ack
			ChangeState(ERROR);
#ifdef CANAUTONODEDEBUG
		SOAR_PRINT("Invalid ACK!\n");
#endif
			return false;
		}

	}
	// not received
	return false;
}


/* Exhausts the FIFO until an update is found, or an update is partly received and times out.
 * return true if successfully received update.
 */
bool CanAutoNodeDaughter::CheckForUpdate() {

	if(GetCurrentState() != WAITING_FOR_UPDATE) {
		return false;
	}
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
#ifdef CANAUTONODEDEBUG
		SOAR_PRINT("Update timed out!\n");
#endif
		ChangeState(ERROR);
	}
	return false;

}

/* Exhausts the FIFO until kick, join, or heartbeat is encountered while ready.
 * @return true if any are found.
 */
bool CanAutoNodeDaughter::ProcessMessage() {
	if(GetCurrentState() != READY) {
		return false;
	}
	uint8_t msg[64] = {123};
	uint32_t id = 0;
	bool gotOne = false;
	while(controller->GetRxFIFO(msg, &id) == HAL_OK) {
		gotOne = true;
		switch(id) {
		case KICK_REQUEST_ID: {
			UniqueBoardID kickedBoard = MsgToData<UniqueBoardID>(msg);
			if(kickedBoard == this->thisNode.uniqueID) {
#ifdef CANAUTONODEDEBUG
	SOAR_PRINT("This node just got kicked!\n");
#endif
				ChangeState(UNINITIALIZED);
				return true;
			}
#ifdef CANAUTONODEDEBUG
	SOAR_PRINT("Saw ");
	PrintBoardID(kickedBoard);
	SOAR_PRINT("just get kicked\n");
#endif

			ChangeState(WAITING_FOR_UPDATE);
			return true;
		}
		case JOIN_REQUEST_ID:
		{
#ifdef CANAUTONODEDEBUG
	SOAR_PRINT("Saw a join request from another board\n");
#endif
			ChangeState( WAITING_FOR_UPDATE);
			return true;
		}

		case HEARTBEAT_ID: {

			if(MsgToData<UniqueBoardID>(msg) == Motherboard.uniqueID) {
#ifdef CANAUTONODEDEBUG
	SOAR_PRINT("Received heartbeat");
#endif
				SendHeartbeat();
			}
			return true;
		}
		}

	}
	return gotOne;
}

/* Change the state, updating timeout timers.
 * @param target The state to change to.
 */
void CanAutoNodeDaughter::ChangeState(daughterState target) {
	currentState = target;
#ifdef CANAUTONODEDEBUG
	SOAR_PRINT("Changing state to %d\n",target);
#endif
	switch(target) {
	case WAITING_FOR_UPDATE:
		tickLastReceivedUpdatePart = HAL_GetTick();
		nodesInNetwork = 0;
		break;

	case ERROR:
#ifdef CANAUTONODEDEBUG
		SOAR_PRINT(">>>ERROR!\n");
#endif
		//intentional fallthrough
	case UNINITIALIZED:
		initializedLogs = false;
		nodesInNetwork = 0;
		thisNode.canIDRange = {0,0};
		break;



	default:
		break;
	}
}

/* @param fdcan Pointer to the autogenerated FDCAN peripheral.
 * @param logs Pointer to an array of data types to be reserved in the network
 * @param numLogs Number of entries in logs.
 * @param boardType A user-defined byte to be stored with the board information.
 * @param slotNumber The number of the slot the board is placed in.
 * @param readableName An optional null-terminated string to identify the board. Can be nullptr.
 */
CanAutoNodeDaughter::CanAutoNodeDaughter(FDCAN_HandleTypeDef *fdcan, const LogInit *logs,
		uint16_t numLogs, uint8_t boardType, uint8_t slotNumber, const char* readableName) {
	controller = new FDCanController(fdcan,nullptr,0);

	memcpy(logsToInit,logs,numLogs*sizeof(LogInit));
	this->numLogs = numLogs;
	callbackcontroller = controller;
	currentState = UNINITIALIZED;
	this->thisNode.canIDRange = {0,0};
	this->thisNode.uniqueID = GetThisBoardUniqueID();
	this->thisNode.boardType = boardType;
	this->thisNode.slotNumber = slotNumber;
	if(readableName != nullptr && strlen(readableName) > 0 && strlen(readableName) < MAX_NAME_STR_LEN) {
		strcpy(this->thisNode.nodeName,readableName);
	} else {
		memset(this->thisNode.nodeName,0x00,MAX_NAME_STR_LEN);
	}
	controller->RegisterFilterRXFIFO(0, MAX_RESERVED_CAN_ID);

}

/* Sends a message to the motherboard by the log index.
 * @param logID The index of the log to send. Determines the size and type. Corresponds to the
 * order of logs given during initialization.
 * @param msg Data to send. Size must be at least as large as the log type being sent.
 * @return true if sent successfully.
 */
bool CanAutoNodeDaughter::SendMessageToMotherboardByLogID(uint16_t logID, const uint8_t *msg) {
	return controller->SendByLogIndex(msg, logID);
}

/* Handle receiving part of an update, updating internal node references and changing state when ready.
 * @param msg Update frame received.
 * @return true if successfully received the update part.
 */
bool CanAutoNodeDaughter::ReceiveUpdate(const uint8_t *msg) {

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
#ifdef CANAUTONODEDEBUG
	SOAR_PRINT("Received a daughter update part for ");
	PrintBoardID(receivedNode.uniqueID);
	SOAR_PRINT("\n");
#endif
		break;

	case CAN_UPDATE_MOTHERBOARD:
		this->Motherboard = receivedNode;
#ifdef CANAUTONODEDEBUG
	SOAR_PRINT("Received a motherboard update part for ");
	PrintBoardID(receivedNode.uniqueID);
	SOAR_PRINT("\n");
#endif
		break;

	default:
		return false;
	}

	return true;

}

bool CanAutoNodeDaughter::ReadMessageByLogIndex(uint8_t logIndex,
		uint8_t *out, uint16_t outLen) {
	if(GetCurrentState() != READY) {
		return false;
	}
	uint16_t logSize = determinedLogs[logIndex].byteLength;
	return ReadMessageFromRXBuf(logIndex, logSize, out, outLen);

}
