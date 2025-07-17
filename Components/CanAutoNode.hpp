/*
 * CanAutoNode.h
 *
 *  Created on: Jul 16, 2025
 *      Author: Local user
 */

#ifndef AUTONODE_CANAUTONODE_HPP_
#define AUTONODE_CANAUTONODE_HPP_



#include "FDCan.h"


const uint32_t MAX_NODES_IN_NETWORK = 100;
const uint32_t JOIN_REQUEST_ID = 0;
const uint32_t ACK_ID = 1;
const uint32_t UPDATE_ID = 2;
const uint32_t KICK_REQUEST_ID = 3;

class CanAutoNode {
public:
	CanAutoNode();
	//CanAutoNode(FDCanController& contr, uint16_t msgIDsToRequestStartID, uint16_t msgIDsToRequestAmount);

	virtual ~CanAutoNode();
	CanAutoNode& operator=(CanAutoNode &&other) = delete;
	CanAutoNode(CanAutoNode &&other) = delete;
	CanAutoNode& operator=(const CanAutoNode &other) = delete;


	enum acknowledgementStatus {
		ACK_GOOD,
		ACK_REQUESTED_IDS_TAKEN,
		ACK_BOARD_ALREADY_EXISTS
	};

protected:
	struct IDRange {
		uint32_t start;
		uint32_t end;
	};

	struct Node {
		IDRange requestedRange;
		uint32_t uniqueID;
	};

	FDCanController* controller = nullptr;
	Node thisNode = {0};


	static uint32_t shift8to32(const uint8_t* in);
	static uint16_t shift8to16(const uint8_t *in);

	static bool IDRangesOverlap(IDRange a, IDRange b);

	static Node nodeFromMsg(const uint8_t* msg);
	static void msgFromNode(Node node, uint8_t* msgout);

	Node nodes[MAX_NODES_IN_NETWORK];
	uint16_t nodesInNetwork = 0;

private:
	CanAutoNode(const CanAutoNode &other) = delete;



};

#endif /* AUTONODE_CANAUTONODE_HPP_ */
