/*
 * CanAutoNode.cpp
 *
 *  Created on: Jul 16, 2025
 *      Author: Local user
 */

#include <CanAutoNode.hpp>
#include <cstring>



CanAutoNode::~CanAutoNode() {

}

/* Converts a byte sequence into a 32-bit integer (big-endian).
 * @param in Pointer to the beginning of the input bytes. Must be at least four bytes in length.
 * @return 32-bit integer represented by the four input bytes.
 */
uint32_t CanAutoNode::shift8to32(const uint8_t *in) {
	return (static_cast<uint32_t>(in[0])<<24) | (static_cast<uint32_t>(in[1])<<16) | (static_cast<uint32_t>(in[2])<<8) | in[3];
}

/* Converts a byte sequence into a 16-bit integer (big-endian).
 * @param in Pointer to the beginning of the input bytes. Must be at least two bytes in length.
 * @return 16-bit integer represented by the two input bytes.
 */
uint16_t CanAutoNode::shift8to16(const uint8_t *in) {
	return (static_cast<uint16_t>(in[0])<<8) | in[1];
}

/* Checks if there is overlap between two ranges. Note that range start is inclusive and range end is exclusive.
 * @param a First range.
 * @param b Second range.
 * @return true if there is any overlap.
 */
bool CanAutoNode::IDRangesOverlap(IDRange a, IDRange b) {
	return a.start < b.end && a.end > b.start;
}

/* Converts a byte sequence into a Node struct.
 * @param msg Input byte sequence. Must be as large as a Node struct.
 * @return Node represented by the bytes.
 */
CanAutoNode::Node CanAutoNode::nodeFromMsg(const uint8_t *msg) {

	return MsgToData<Node>(msg);

}

/* Converts a 32-bit integer into a byte sequence (big-endian).
 * @param in 32-bit integer to convert.
 * @param out Pointer to output buffer where the integer will be written. Must be four bytes long.
 */
void CanAutoNode::shift32to8(uint32_t in, uint8_t *out) {
	memcpy(out,&in,sizeof(in));
}

/* Converts a Node struct into a byte sequence.
 * @param Node Node to convert.
 * @param msgout Pointer to output buffer. Must be as long as a Node struct.
 */
void CanAutoNode::msgFromNode(Node node, uint8_t *msgout) {

	memcpy(msgout,&node,sizeof(Node));


}

CanAutoNode::CanAutoNode() {
}

/* Send a message to a given board, starting from the beginning of its CAN ID range.
 * If the message is too long for a single frame, will use multiple IDs.
 * The message must not be too long to fit in the board's claimed range.
 * @param boardID The unique board ID to send the message to. Must exist in the network.
 * @param msg Message to send.
 * @param len Length of message in bytes.
 * @return true if successfully sent.
 */
bool CanAutoNode::SendMessageToDaughterBoardID(UniqueBoardID boardID, const uint8_t *msg,
		uint16_t len, uint16_t CANIDOffset) {

	for(uint16_t i = 0; i < nodesInNetwork; i++) {
		const Node& thisNode = daughterNodes[i];
		if(thisNode.uniqueID == boardID) {
			if((len-1)/64u+1 > thisNode.canIDRange.end - thisNode.canIDRange.start) {
				return false;
			}
			return controller->SendByMsgID(msg, len, thisNode.canIDRange.start + CANIDOffset);
		}
	}
	return false;

}

/* Sends a message starting at a given CAN ID, regardless of which board has claimed it.
 * @param startingCanID CAN ID to start the message at.
 * @param msg Message to send.
 * @param len Length of message in bytes.
 * @return true if successfully sent.
 */
bool CanAutoNode::SendMessageByCANID(uint32_t startingCanID, const uint8_t *msg,
		uint16_t len) {
		return controller->SendByMsgID(msg, len, startingCanID);
}

CanAutoNode::UniqueBoardID CanAutoNode::GetThisBoardUniqueID() const {
	UniqueBoardID out;
	out.u0 = HAL_GetUIDw0();
	out.u1 = HAL_GetUIDw1();
	out.u2 = HAL_GetUIDw2();
	return out;
}

/* Sends a message to every daughter node of a given user-defined board type by a log index.
 * @param boardType The type to send to.
 * @param logIndex The log index to use.
 * @param msg The message to send. Size determined by the reserved log.
 * @return true if successfully sent at least one message.
 */
bool CanAutoNode::SendMessageToAllBoardsOfTypeByLogIndex(uint8_t boardType,
		uint8_t logIndex, const uint8_t *msg) {
	bool foundOne = false;
	for(uint16_t i = 0; i < nodesInNetwork; i++) {
		const Node& thisNode = daughterNodes[i];
		if(thisNode.boardType == boardType) {
			if(SendMessageToDaughterByLogIndex(thisNode.uniqueID, logIndex, msg)) {
				foundOne = true;
			}
		}
	}
	return foundOne;

}

#ifdef CANAUTONODEDEBUG
void CanAutoNode::PrintBoardID(CanAutoNode::UniqueBoardID id) {
	  for(uint16_t i = 0; i < sizeof(CanAutoNode::UniqueBoardID); i++) {
		  SOAR_PRINT("%x",((const uint8_t*)(&id))[i]);
		  if(i % 4 == 0 && i > 0 && i < sizeof(CanAutoNode::UniqueBoardID)-1) {
			  SOAR_PRINT(" ");
		  }
	  }
}
#endif

/* Broadcast a heartbeat to the entire network on the reserved heartbeat channel.
 * The caller is responsible for checking for responses.
 * @return true if successfully sent.
 */
bool CanAutoNode::SendHeartbeat() {

	HeartbeatInfo beat;
	beat.senderBoardID = thisNode.uniqueID;

	uint8_t msg[sizeof(HeartbeatInfo)] = {};
	memcpy(msg,&beat,sizeof(msg));
	return controller->SendByMsgID(msg, sizeof(msg), HEARTBEAT_ID);

}

/* Sends a message to a daughter board by a log index on that daughter board. Can be called from
 * the motherboard or another daughter board.
 * @param boardID Unique board ID to send to.
 * @param logIndex Index of the log on the target board to send.
 * @param msg Message to send. Size is determined by the size of the log being sent.
 * @return true if successfully sent.
 */
bool CanAutoNode::SendMessageToDaughterByLogIndex(UniqueBoardID boardID,
		uint8_t logIndex, const uint8_t *msg) {
	if(logIndex >= MAX_LOGS) {
		return false;
	}
	for(uint16_t i = 0; i < nodesInNetwork; i++) {
		const Node& thisDaughter = daughterNodes[i];
		if(thisDaughter.uniqueID == boardID) {
			return controller->SendByMsgID(msg, thisDaughter.logSizesInBytes[logIndex], thisDaughter.logOffsetsInCANIDs[logIndex]);
		}
	}
	return false;
}

/* Sends a message to a board in a given slot number by the log index on that daughter board.
 * @param slotNumber Slot to send to.
 * @param logIndex Index of the log on the target board to send.
 * @param msg Message to send. Size is determined by the size of the log being sent.
 * @return true if successfully sent.
 */
bool CanAutoNode::SendMessageToSlotNumberByLogIndex(uint8_t slotNumber,
		uint8_t logIndex, const uint8_t *msg) {

	for(uint16_t i = 0; i < nodesInNetwork; i++) {
		const Node& thisNode = daughterNodes[i];
		if(thisNode.slotNumber == slotNumber) {
			return SendMessageToDaughterByLogIndex(thisNode.uniqueID, logIndex, msg);
		}
	}
	return false;
}
