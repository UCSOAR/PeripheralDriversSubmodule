/*
 * CanAutoNode.h
 *
 *  Created on: Jul 16, 2025
 *      Author: Adam Godin
 */

#ifndef AUTONODE_CANAUTONODE_HPP_
#define AUTONODE_CANAUTONODE_HPP_



#include "FDCan.h"
#include <cstring>


constexpr uint32_t MAX_NODES_IN_NETWORK = 100;
constexpr uint8_t MAX_LOGS = 5;
constexpr uint8_t MAX_JOIN_ATTEMPTS = 8;
// Max 2047 for 11-bit standard FDCAN, max 536,870,911 for extended FDCAN.
// See transceiver and board capabilities before changing.
constexpr uint16_t MAX_CAN_ID = 2047;

// Reserved CAN IDs
constexpr uint16_t JOIN_REQUEST_ID = 0;
constexpr uint16_t ACK_ID = 1;
constexpr uint16_t UPDATE_ID = 2;
constexpr uint16_t KICK_REQUEST_ID = 3;
constexpr uint16_t HEARTBEAT_ID = 4;


class CanAutoNode {
public:
	CanAutoNode();
	//CanAutoNode(FDCanController& contr, uint16_t msgIDsToRequestStartID, uint16_t msgIDsToRequestAmount);

	~CanAutoNode();
	CanAutoNode& operator=(CanAutoNode &&other) = delete;
	CanAutoNode(CanAutoNode &&other) = delete;
	CanAutoNode& operator=(const CanAutoNode &other) = delete;


	enum updateType {
		CAN_UPDATE_DAUGHTER,
		CAN_UPDATE_LAST_DAUGHTER,
		CAN_UPDATE_FSB
	};
	enum acknowledgementStatus {
		ACK_GOOD,
		ACK_NO_ROOM,
		ACK_BOARD_ALREADY_EXISTS
	};

	virtual bool CheckMessages() = 0;

	struct UniqueBoardID {
		uint32_t u0;
		uint32_t u1;
		uint32_t u2;
		bool operator!=(const UniqueBoardID& other) const {
			return u0!=other.u0 || u1!=other.u1 || u2!=other.u2;
		}
		bool operator==(const UniqueBoardID& other) const {
			return u0 == other.u0 && u1 == other.u1 && u2 == other.u2;
		}
	};

	bool SendMessageToDaughterBoardID(UniqueBoardID boardID, const uint8_t* msg, uint16_t len, uint16_t CANIDOffset);
	bool SendMessageByCANID(uint32_t startingCanID, const uint8_t* msg, uint16_t len);
	bool SendMessageToDaughterByLogIndex(UniqueBoardID boardID, uint8_t logIndex, const uint8_t* msg);

	UniqueBoardID GetThisBoardUniqueID() const;

protected:

	struct IDRange {
		uint32_t start = 0;
		uint32_t end = 0;
		bool operator!=(const IDRange& other) const {
			return this->start != other.start
					|| this->end != other.end;
		}
	};

	struct Node {
		IDRange canIDRange;
		UniqueBoardID uniqueID = {0};

		bool operator!=(const Node& other) const {
			return other.canIDRange != this->canIDRange
					|| other.uniqueID != this->uniqueID
					|| other.numberOfLogs != this->numberOfLogs
					|| memcmp(other.logOffsetsInCANIDs,this->logOffsetsInCANIDs,sizeof(logOffsetsInCANIDs));
		}

		uint8_t numberOfLogs = 0;
		uint8_t logOffsetsInCANIDs[MAX_LOGS];
		uint8_t logSizesInBytes[MAX_LOGS];

	};

	static_assert(sizeof(Node) <= 64, "Node entries must be at most 64 bytes large. Try reducing MAX_LOGS");

	struct HeartbeatInfo {
		UniqueBoardID senderBoardID;
	};

	struct JoinRequest {
		UniqueBoardID uniqueID;
		uint8_t slotNumber;
		uint8_t boardType;

		uint8_t numberOfLogs;
		uint8_t logSizesInBytes[MAX_LOGS];
	};

	static_assert(sizeof(JoinRequest) <= 64, "Join request entries must be at most 64 bytes large. Try reducing MAX_LOGS");


	FDCanController* controller = nullptr;
	Node thisNode = {0};

	static uint32_t shift8to32(const uint8_t* in);
	static uint16_t shift8to16(const uint8_t *in);
	static void shift32to8(uint32_t in, uint8_t* out);

	static bool IDRangesOverlap(IDRange a, IDRange b);

	static Node nodeFromMsg(const uint8_t* msg);
	static void msgFromNode(Node node, uint8_t* msgout);

	Node daughterNodes[MAX_NODES_IN_NETWORK];
	uint16_t nodesInNetwork = 0;

	bool SendHeartbeat();

	template <typename T>
	static void DataToMsg(T data, uint8_t* out) {
		memcpy(out,&data,sizeof(T));
	}

	template <typename T>
	static T MsgToData(const uint8_t* in) {
		T out;
		memcpy(&out,in,sizeof(T));
		return out;
	}




private:
	CanAutoNode(const CanAutoNode &other) = delete;



};

#endif /* AUTONODE_CANAUTONODE_HPP_ */
