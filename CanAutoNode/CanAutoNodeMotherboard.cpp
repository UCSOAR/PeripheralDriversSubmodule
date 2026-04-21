#include <CanAutoNodeMotherboard.hpp>
#include <cstring>

/* Kicks a board out of the network, updating the table, and sending an update to all remaining nodes.
 * @param uniqueBoardID Board to kick.
 * @return true if successful,
 */
bool CanAutoNodeMotherboard::KickNode(UniqueBoardID uniqueBoardID) {

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
#ifdef CANAUTONODEDEBUG
		SOAR_PRINT("Tried to kick node, but doesn't exist! (Attempted to kick ");
		PrintBoardID(uniqueBoardID);
		SOAR_PRINT(")\nNow just forcing send kick\n");
#endif




		//return true;
	} else {

	// to get rid of this node, first we will identify the MOTHERBOARD log indexes to remove ON THE MOTHERBOARD
	uint8_t indicesToRemove[daughterNodes[foundIndex].numberOfLogs];
	for(uint8_t i = 0; i < daughterNodes[foundIndex].numberOfLogs; i++) {
		indicesToRemove[i] = daughterNodes[foundIndex].startingLogIndexOnMotherboard+i;
	}
	controller->RemoveLogIndices(indicesToRemove, sizeof(indicesToRemove)/sizeof(indicesToRemove[0]));
	nextFreeMotherboardLogIndex -= daughterNodes[foundIndex].numberOfLogs;

	// next, we shall update all other nodes to shift their motherboard log index offsets down to fill in the gap.
	// this change will be reflected to all remaining daughters in the upcoming update
	// note that this does not reassign can ids. this is fine, now that this node is gone its space in the
	//can address space is freed and can be reassigned. i dont care about fragmentation, if you are connecting and kicking over 2000 nodes in a random order, there is some other problem
	for(uint8_t i = 0; i < nodesInNetwork; i++) {
		if(i != foundIndex && daughterNodes[i].startingLogIndexOnMotherboard > daughterNodes[foundIndex].startingLogIndexOnMotherboard) {
			daughterNodes[i].startingLogIndexOnMotherboard -= daughterNodes[foundIndex].numberOfLogs;
		}

	}

	// now actually remove the kicked node
	daughterNodes[foundIndex] = daughterNodes[--nodesInNetwork];

	for(uint16_t i = 0; i < recentlyJoinedNum; i++) {
		if(recentlyJoined[i]->uniqueID == uniqueBoardID) {
			recentlyJoined[i] = recentlyJoined[--recentlyJoinedNum];
			break;
		}
	}

	heartbeatGracePeriod[foundIndex] = heartbeatGracePeriod[nodesInNetwork];
	}
	if(!controller->SendByMsgID((uint8_t*)(&uniqueBoardID), sizeof(uniqueBoardID), KICK_REQUEST_ID)) {
#ifdef CANAUTONODEDEBUG
		SOAR_PRINT("Tried to kick node, but failed to send! (Attempted to kick ");
		PrintBoardID(uniqueBoardID);
		SOAR_PRINT(")\n");
#endif
		return false;
	}
#ifdef CANAUTONODEDEBUG
		SOAR_PRINT("Kicked node (");
		PrintBoardID(uniqueBoardID);
		SOAR_PRINT(")\n");
#endif


	return SendFullUpdate();
}

/* Kicks a board out of the network with a given name, updating the table, and sending an update to all remaining nodes.
 * @param uniqueBoardID Board to kick.
 * @return true if successful,
 */
bool CanAutoNodeMotherboard::KickNode(const char *boardName) {
	for(uint16_t i = 0; i < nodesInNetwork; i++) {
		const Node& thisNode = daughterNodes[i];
		if(strcmp(thisNode.nodeName,boardName) == 0) {
			return KickNode(thisNode.uniqueID);
		}
	}
	return false;
}

/* Kicks a board out of the network in a given slot number, updating the table, and sending an update to all remaining nodes.
 * @param uniqueBoardID Board to kick.
 * @return true if successful,
 */
bool CanAutoNodeMotherboard::KickNode(uint16_t slotNumber) {
	for(uint16_t i = 0; i < nodesInNetwork; i++) {
		const Node& thisNode = daughterNodes[i];
		if(thisNode.slotNumber == slotNumber) {
			return KickNode(thisNode.uniqueID);
		}
	}
	return false;
}

CanAutoNodeMotherboard::CanAutoNodeMotherboard(FDCAN_HandleTypeDef *fdcan) {
	controller = new FDCanController(fdcan,nullptr,0);
	callbackcontroller = controller;
	//controller->RegisterFilterRXFIFO(0, MAX_RESERVED_CAN_ID);
	memset(heartbeatGracePeriod,0x00,sizeof(heartbeatGracePeriod));

	FDCanController::LogInitStruct reservedLogs[] = {{64,JOIN_REQUEST_ID},{64,ACK_ID},{64,UPDATE_ID},{64,KICK_REQUEST_ID},{64,HEARTBEAT_ID}};
	controller->RegisterLogs(reservedLogs, sizeof(reservedLogs)/sizeof(reservedLogs[0]));
}



/* Exhausts the FIFO until a join request is found and processed.
 * @return true if one was found and processed successfully.
 */
bool CanAutoNodeMotherboard::CheckForJoinRequest() {

	uint8_t msg[64] = {123};
	uint32_t id = 0;


	while(controller->ReceiveLogIndexFromRXBuf(msg, JOIN_REQUEST_ID)) {

		return ReceiveJoinRequest(msg);

	}
	// not received
	return false;

}

/* Attempts to incorporate a new node into the network. Will assign the new node a
 * consecutive range of CAN IDs of the requested size. Sends an acknowledgment, followed by a
 * complete update if successful.
 * @param msg The join request received.
 * @return true if successfully added the node and send an update.
 */
bool CanAutoNodeMotherboard::ReceiveJoinRequest(uint8_t* msg) {


	JoinRequest request = MsgToData<JoinRequest>(msg);
#ifdef CANAUTONODEDEBUG
	SOAR_PRINT("Received a join request from ");
	PrintBoardID(request.uniqueID);
	SOAR_PRINT("\n");
#endif
	uint16_t requiredTotalCANIDs = 0;
	for(uint16_t i = 0; i < request.numberOfLogs; i++) {
		requiredTotalCANIDs += (request.logSizesInBytes[i]-1)/64+1;
	}
	for(uint16_t i = 0; i < nodesInNetwork; i++) {
		const Node& thisAlreadyExistingNode = daughterNodes[i];

		if(request.uniqueID == thisAlreadyExistingNode.uniqueID) {
			// already have this board somehow
			SendAck(ACK_BOARD_ALREADY_EXISTS);
			return false;
		}

	}


	// Finding smallest possible gap to fit the newly requested ID range into
	Node sortedNodes[nodesInNetwork];
	memcpy(sortedNodes,daughterNodes,sizeof(sortedNodes));

	auto comp = [](const void* a, const void* b) {
		return static_cast<int>(static_cast<const Node*>(a)->canIDRange.start - static_cast<const Node*>(b)->canIDRange.start);
	};
	qsort(sortedNodes, nodesInNetwork, sizeof(Node), comp);


	uint32_t bestStartingFreeCANID = 0;
	bool foundRoom = false;
	uint16_t bestAmountOfRoom = MAX_CAN_ID;
	uint32_t previousEnd = MAX_RESERVED_CAN_ID+1;

	for(uint16_t i = 0; i < nodesInNetwork; i++) {
		const Node& thisNode = sortedNodes[i];
		uint16_t thisAmountOfRoom = thisNode.canIDRange.start-previousEnd;

		if(thisAmountOfRoom < bestAmountOfRoom && thisAmountOfRoom >= requiredTotalCANIDs) {
			bestAmountOfRoom = thisAmountOfRoom;
			bestStartingFreeCANID = previousEnd;
			foundRoom = true;
		}
		previousEnd = thisNode.canIDRange.end;
	}

	uint16_t thisAmountOfRoom = MAX_CAN_ID-previousEnd;

	if(thisAmountOfRoom < bestAmountOfRoom && thisAmountOfRoom >= requiredTotalCANIDs) {
		bestAmountOfRoom = thisAmountOfRoom;
		bestStartingFreeCANID = previousEnd;
		foundRoom = true;
	}


	if(foundRoom)  {
		// found no issues
		SendAck(ACK_GOOD);
#ifdef CANAUTONODEDEBUG
		SOAR_PRINT("Adding new node at ID range [%d,%d)\n",bestStartingFreeCANID,bestStartingFreeCANID+requiredTotalCANIDs);
#endif
		Node newNode;
		newNode = {{bestStartingFreeCANID,bestStartingFreeCANID+requiredTotalCANIDs}, // 1 too many?
				request.uniqueID,
				request.numberOfLogs,
				request.boardType,
				request.slotNumber};
		strncpy(newNode.nodeName,request.nodeName,MAX_NAME_STR_LEN);

		FDCanController::LogInitStruct newLogs[request.numberOfLogs];
		uint16_t thisID = bestStartingFreeCANID;
		for(uint16_t i = 0; i < request.numberOfLogs; i++) {
			newLogs[i] = {request.logSizesInBytes[i], thisID};
			newNode.logOffsetsInCANIDs[i] = thisID - bestStartingFreeCANID;
			newNode.logSizesInBytes[i] = request.logSizesInBytes[i];
			thisID += (request.logSizesInBytes[i]-1)/64+1;

		}
		daughterNodes[nodesInNetwork] = newNode;
		recentlyJoined[recentlyJoinedNum++] = &daughterNodes[nodesInNetwork];
		controller->RegisterLogs(newLogs, request.numberOfLogs);
		heartbeatGracePeriod[nodesInNetwork] = 0;
		nodesInNetwork++;
		newNode.startingLogIndexOnMotherboard = nextFreeMotherboardLogIndex;
		nextFreeMotherboardLogIndex += request.numberOfLogs;


		HAL_Delay(50);
		return SendFullUpdate();
	} else {
		SendAck(ACK_NO_ROOM);
#ifdef CANAUTONODEDEBUG
		SOAR_PRINT("No room for new node!\n");
#endif
		return false;
	}

}

/* Sends an acknowledgment to the network.
 * @apram status The acknowledgment to send.
 * @return true if successfully sent.
 */
bool CanAutoNodeMotherboard::SendAck(acknowledgementStatus status) {

	uint8_t msg[] = {static_cast<uint8_t>(status)};
	return controller->SendByMsgID(msg, sizeof(msg), ACK_ID);

}


/* Sends a full list of all nodes in the network to all nodes. Serves to keep
 * daughter nodes up-to-date with the network contents.
 * Sends in multiple frames, where each frame contains one node preceded by a status byte,
 * denoting whether the frame is a daughter node or the motherboard node, and whether it is the
 * last frame in the update.
 * @return true if successful.
 */
bool CanAutoNodeMotherboard::SendFullUpdate() {

	uint8_t msg[sizeof(Node)+1];
	msgFromNode(thisNode, msg+1);
	msg[0] = CAN_UPDATE_MOTHERBOARD;

	if(!controller->SendByMsgID(msg, sizeof(msg), UPDATE_ID)) {
#ifdef CANAUTONODEDEBUG
	SOAR_PRINT("Failed to send motherboard update frame!\n");
#endif
		return false;
	}
#ifdef CANAUTONODEDEBUG
	SOAR_PRINT("Sent motherboard update frame\n");
#endif

	for(uint16_t i = 0; i < nodesInNetwork; i++) {
		msgFromNode(daughterNodes[i], msg+1);
		msg[0] = (i == nodesInNetwork-1) ? CAN_UPDATE_LAST_DAUGHTER : CAN_UPDATE_DAUGHTER;
		if(!controller->SendByMsgID(msg, sizeof(msg), UPDATE_ID)) {
#ifdef CANAUTONODEDEBUG
		SOAR_PRINT("Tried but failed to send daughter update frame!\n");
#endif
			return false;
		}
#ifdef CANAUTONODEDEBUG
		SOAR_PRINT("Sent daughter update frame %d/%d\n",i+1,nodesInNetwork);
#endif
	}

	return true;

}

/* Sends a heartbeat, then monitors for responses. Will wait up to one second.
 * Any nodes that do not respond within this time will be kicked.
 * @return true if heartbeat successfully sent.
 */
bool CanAutoNodeMotherboard::Heartbeat() {

	this->lastHeartbeatTick = HAL_GetTick();

	if(!SendHeartbeat()) {
		return false;
	}



	bool received[nodesInNetwork];
	memset(received,0,sizeof(received));

	for(uint16_t i = 0; i < 1000; i++) {
		HAL_Delay(1);
		uint8_t out[64];
		uint32_t id = 0;
		if(controller->ReceiveLogIndexFromRXBuf(out, HEARTBEAT_ID)) {


				UniqueBoardID responseID = MsgToData<UniqueBoardID>(out);
				bool foundResponder = false;
#ifdef CANAUTONODEDEBUG
		SOAR_PRINT("Got a heartbeat from ");
		PrintBoardID(responseID);
		SOAR_PRINT(", there are %d daughter nodes in the network\n",nodesInNetwork);
#endif
				for(int node = 0; node < nodesInNetwork; node++) {
					if(daughterNodes[node].uniqueID == responseID) {
						if(received[node]) {
							// received multiple heartbeats from the same node?
#ifdef CANAUTONODEDEBUG
		SOAR_PRINT("Received multiple heartbeats from ");
		PrintBoardID(responseID);
		SOAR_PRINT(", kicking\n");
#endif
							KickNode(responseID);
						}
#ifdef CANAUTONODEDEBUG
		SOAR_PRINT("Found the source of this heartbeat at %d!\n",node);
#endif
						received[node] = true;
						foundResponder = true;
						break;
					}
				}
				bool gotAllOfThem = true;
				for(uint8_t j = 0; j < nodesInNetwork; j++) {
					if(!received[j]) {
						gotAllOfThem = false;
						break;
					}
				}

				if(!foundResponder) {
					// ??? a node that wasn't in the network just responded to the heartbeat?  get out
#ifdef CANAUTONODEDEBUG
		SOAR_PRINT("Received heartbeat from unrecognized node ");
		PrintBoardID(responseID);
		SOAR_PRINT(", kicking\n");
#endif
					KickNode(responseID);
				}


				if(gotAllOfThem) {
						return true; // just leave early, we got all the responses
					}




		}
		if(i > 10 && nodesInNetwork == 0) {
			return true; // don't bother waiting the whole timeout when there are no daughter nodes, we just want to see if there are any out-of-network responders
		}
	}

	for(size_t i = 0; i < nodesInNetwork; i++) {
		if(!received[i]) { // Found a node that never responded to the heartbeat
#ifdef CANAUTONODEDEBUG
		SOAR_PRINT("Received no heartbeat from ");
		PrintBoardID(daughterNodes[i].uniqueID);
		SOAR_PRINT(", kicking\n");
#endif
		if(heartbeatGracePeriod[i] > 0) {
			heartbeatGracePeriod[i]--;
		} else {
			KickNode(daughterNodes[i].uniqueID);
		}
		}
	}

	return true;

}

uint32_t CanAutoNodeMotherboard::GetTicksSinceLastHeartbeat() const {
	return HAL_GetTick() - lastHeartbeatTick;
}

/*
 * Reads an incoming message from a daughter node on a given log index.
 */
bool CanAutoNodeMotherboard::ReadMessageFromDaughterByLogIndex(
		UniqueBoardID daughter, uint8_t logIndex, uint8_t *out,
		uint16_t outSize) {

	for(uint16_t i = 0; i < nodesInNetwork; i++) {
		const Node& thisNode = daughterNodes[i];
		if(thisNode.uniqueID == daughter) {

		if(logIndex >= thisNode.numberOfLogs) {
#ifdef CANAUTONODEDEBUG
		SOAR_PRINT("Cannot read log index %d from daughter with max index %d",logIndex,thisNode.numberOfLogs);
#endif
			return false;
		}
			return ReadMessageFromRXBuf(thisNode.startingLogIndexOnMotherboard+logIndex, thisNode.logSizesInBytes[logIndex], out, outSize);
		}
	}
#ifdef CANAUTONODEDEBUG
	SOAR_PRINT("No node with that ID!\n");
#endif
	return false;

}

/* Returns an array of name strings of any boards that joined the network since the last call of this function.
 * @param outputArr A pointer to an array of char arrays to store the names.
 * @param outputBufferLen For safety, will not write more entries than this to the output.
 * @return The number of nodes returned in the array.
 *
 */
uint16_t CanAutoNodeMotherboard::GetNamesOfNewlyJoinedBoards(char(*outputArr)[MAX_NAME_STR_LEN], uint16_t outputBufferLen) {

	if(recentlyJoinedNum == 0) {
		return 0;
	}

	uint16_t num = 0;
	for(uint16_t i = 0; i < recentlyJoinedNum; i++) {
		strncpy(outputArr[num++],daughterNodes[i].nodeName,MAX_NAME_STR_LEN);
		if(i >= outputBufferLen) {
			break;
		}

	}
	memcpy(recentlyJoined,&recentlyJoined[recentlyJoinedNum-num],(num)*sizeof(Node*));
	recentlyJoinedNum -= num;

}
