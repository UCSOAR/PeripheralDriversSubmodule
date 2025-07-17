#include <CanAutoNodeFSB.hpp>
#include <cstring>



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
		Node thisAlreadyExistingNode = nodes[i];

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
	nodes[nodesInNetwork] = newNode;
	nodesInNetwork++;
	return true;

}

bool CanAutoNodeFSB::SendAck(acknowledgementStatus status) {

	uint8_t msg[] = {static_cast<uint8_t>(status)};
	return controller->SendByMsgID(msg, sizeof(msg), ACK_ID);

}


// update same format as request format, but with 1 byte in front which is 0 if still more, 1 if done
bool CanAutoNodeFSB::SendFullUpdate() {

	uint8_t msg[13];
	msgFromNode(thisNode, msg+1);
	msg[0] = (nodesInNetwork == 0);

	if(!controller->SendByMsgID(msg, sizeof(msg), UPDATE_ID)) {
		return false;
	}

	for(uint16_t i = 0; i < nodesInNetwork; i++) {
		msgFromNode(nodes[i], msg+1);
		msg[0] = (i == nodesInNetwork-1);
		if(!controller->SendByMsgID(msg, sizeof(msg), UPDATE_ID)) {
			return false;
		}
	}

	return true;

}
