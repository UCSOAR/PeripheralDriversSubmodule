/*
 * CanAutoNode.h
 *
 *  Created on: Jul 16, 2025
 *      Author: Local user
 */

#ifndef AUTONODE_CANAUTONODE_HPP_
#define AUTONODE_CANAUTONODE_HPP_



#include "FDCan.h"
#include <cstring>

constexpr uint32_t MAX_NODES_IN_NETWORK = 100;

constexpr uint32_t JOIN_REQUEST_ID = 0;
constexpr uint32_t ACK_ID = 1;
constexpr uint32_t UPDATE_ID = 2;
constexpr uint32_t KICK_REQUEST_ID = 3;
constexpr uint32_t HEARTBEAT_ID = 4;

constexpr uint16_t MAX_LOGS = 32;
constexpr uint16_t MAX_CAN_ID = 2047;



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

	bool SendMessageToDaughterBoardID(uint32_t boardID, const uint8_t* msg, uint16_t len, uint16_t CANIDOffset);
	bool SendMessageByCANID(uint32_t startingCanID, const uint8_t* msg, uint16_t len);

protected:
	struct IDRange {
		uint32_t start;
		uint32_t end;
		bool operator!=(const IDRange& other) const {
			return this->start != other.start
					|| this->end != other.end;
		}
	};

	struct Node {
		IDRange canIDRange;
		uint32_t uniqueID;
		bool operator!=(const Node& other) const {
			return other.canIDRange != this->canIDRange
					|| other.uniqueID != this->uniqueID;
		}

	};

	struct HeartbeatInfo {
		uint32_t senderBoardID;
	};

	struct JoinRequest {
		uint32_t uniqueID;
		uint8_t slotNumber;
		uint8_t boardType;
//		uint16_t requiredTotalCANIDs;

		uint16_t numberOfLogs;
		uint8_t  logSizesInBytes[52];
	};

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
	uint32_t nodeTableVersion = 0; //todo

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
