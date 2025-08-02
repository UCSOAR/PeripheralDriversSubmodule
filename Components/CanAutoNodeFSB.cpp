#include <CanAutoNodeFSB.hpp>
#include <cstring>

/* Kicks a board out of the network, updating the table, and sending an update to all remaining nodes.
 * @param uniqueBoardID Board to kick.
 * @return true if successful,
 */
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

//bool CanAutoNodeFSB::SendToDaughterBoardByIndex(uint32_t uniqueID,
//		uint8_t logIndex, const uint8_t *msg) {
//
//	for(uint16_t i = 0; i < nodesInNetwork; i++) {
//		const Node& thisDaughter = daughterNodes[i];
//		if(thisDaughter.uniqueID == uniqueID) {
//
//			for(uint16_t j = 0; j < )
//
//		}
//	}
//
//	return false;
//
//}

/* Exhausts the FIFO until a join request is found and processed.
 * @return true if one was found and processed successfully.
 */
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

/* Attempts to incorporate a new node into the network. Will assign the new node a
 * consecutive range of CAN IDs of the requested size. Sends an acknowledgment, followed by a
 * complete update if successful.
 * @param msg The join request received.
 * @return true if successfully added the node and send an update.
 */
bool CanAutoNodeFSB::ReceiveJoinRequest(uint8_t* msg) {

	JoinRequest request = MsgToData<JoinRequest>(msg);
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
	uint16_t bestAmountOfRoom = UINT16_MAX;
	uint32_t previousEnd = 0;

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
		daughterNodes[nodesInNetwork++] = {{bestStartingFreeCANID,bestStartingFreeCANID+requiredTotalCANIDs},request.uniqueID};
		FDCanController::LogInitStruct newLogs[request.numberOfLogs];
		uint16_t thisID = bestStartingFreeCANID;
		for(uint16_t i = 0; i < request.numberOfLogs; i++) {
			newLogs[i] = {request.logSizesInBytes[i], thisID};
			thisID += (request.logSizesInBytes[i]-1)/64+1;
		}
		controller->RegisterLogs(newLogs, request.numberOfLogs);
		HAL_Delay(50);
		return SendFullUpdate();
	} else {
		SendAck(ACK_NO_ROOM);
		return false;
	}

}

/* Sends an acknowledgment to the network.
 * @apram status The acknowledgment to send.
 * @return true if successfully sent.
 */
bool CanAutoNodeFSB::SendAck(acknowledgementStatus status) {

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

/* Sends a heartbeat, then monitors for responses. Will wait up to one second.
 * Any nodes that do not respond within this time will be kicked.
 * @return true if heartbeat successfully sent.
 */
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


