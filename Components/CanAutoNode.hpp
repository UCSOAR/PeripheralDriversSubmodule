/*
 * CanAutoNode.h
 *
 *  Created on: Jul 16, 2025
 *      Author: Adam Godin
 */

#ifndef AUTONODE_CANAUTONODE_HPP_
#define AUTONODE_CANAUTONODE_HPP_

/*
 * Define this to enable debug prints across the driver. Uses SOAR_PRINT.
 */
//#define CANAUTONODEDEBUG

#include "FDCan.h"
#include <cstring>


constexpr uint32_t MAX_NODES_IN_NETWORK = 100;
constexpr uint8_t MAX_LOG_TYPES_PER_NODE = 5;
constexpr uint8_t MAX_NAME_STR_LEN = 20;
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
constexpr uint16_t MAX_RESERVED_CAN_ID = 4; // Make sure to update if adding a new reserved ID


#ifdef CANAUTONODEDEBUG
#include "Task.hpp"
#include "SystemDefines.hpp"
#endif

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
		CAN_UPDATE_MOTHERBOARD
	};
	enum acknowledgementStatus {
		ACK_GOOD,
		ACK_NO_ROOM,
		ACK_BOARD_ALREADY_EXISTS
	};

	virtual bool CheckCANCommands() = 0;

	struct UniqueBoardID {
		uint32_t u0;
		uint32_t u1;
		uint32_t u2;

		bool operator==(const UniqueBoardID&) const = default;
		bool operator!=(const UniqueBoardID&) const = default;

	};

	bool SendMessageToDaughterByLogIndex(UniqueBoardID boardID, uint8_t logIndex, const uint8_t* msg);
	bool SendMessageToAllBoardsOfTypeByLogIndex(uint8_t boardType, uint8_t logIndex, const uint8_t* msg);
	bool SendMessageToSlotNumberByLogIndex(uint8_t slotNumber, uint8_t logIndex, const uint8_t* msg);
	bool SendMessageToNameByLogIndex(const char* targetName, uint8_t logIndex, const uint8_t* msg);
	bool SendMessageByCANID(uint32_t startingCanID, const uint8_t* msg, uint16_t len);
	bool SendMessageToDaughterBoardByCANIDOffset(UniqueBoardID boardID, const uint8_t* msg, uint16_t len, uint16_t CANIDOffset);

	UniqueBoardID GetThisBoardUniqueID() const;

	virtual uint16_t GetNumberOfNodesInNetwork() const = 0;

	bool BoardExistsWithName(const char* name);

	uint16_t GetNamesOfAllBoards(char(*outputArr)[MAX_NAME_STR_LEN], uint16_t outputBufferLen);

#ifdef CANAUTONODEDEBUG
	static void PrintBoardID(UniqueBoardID id);
#endif

protected:

	struct IDRange {
		uint32_t start = 0;
		uint32_t end = 0;

		bool operator==(const IDRange&) const = default;
		bool operator!=(const IDRange&) const = default;
	};

	struct Node {
		IDRange canIDRange;
		UniqueBoardID uniqueID = {0};

		uint8_t numberOfLogs = 0;
		uint8_t boardType = 0;
		uint8_t slotNumber = 0;

		uint8_t logOffsetsInCANIDs[MAX_LOG_TYPES_PER_NODE];
		uint8_t logSizesInBytes[MAX_LOG_TYPES_PER_NODE];

		char nodeName[MAX_NAME_STR_LEN];

		uint16_t startingLogIndexOnMotherboard = 0;

		bool operator==(const Node&) const = default;
		bool operator!=(const Node&) const = default;

	};

	static_assert(sizeof(Node) <= 64, "Node entries must be at most 64 bytes large. Try reducing MAX_LOGS");

	struct HeartbeatInfo {
		UniqueBoardID senderBoardID;
	};

	struct JoinRequest {
		UniqueBoardID uniqueID;
		uint8_t slotNumber;
		uint8_t boardType;

		char nodeName[MAX_NAME_STR_LEN];
		uint8_t numberOfLogs;
		uint8_t logSizesInBytes[MAX_LOG_TYPES_PER_NODE];
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

	bool ReadMessageFromRXBuf(uint8_t logIndex, uint16_t logSize, uint8_t* out, uint16_t outLen);

private:
	CanAutoNode(const CanAutoNode &other) = delete;


};

#endif /* AUTONODE_CANAUTONODE_HPP_ */
