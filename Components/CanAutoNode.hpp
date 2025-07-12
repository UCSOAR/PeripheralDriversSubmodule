#include "FDCan.h"


const uint32_t MAX_NODES_IN_NETWORK = 100;
const uint32_t JOIN_REQUEST_ID = 0;
const uint32_t ACK_ID = 1;
const uint32_t UPDATE_ID = 2;

class CanAutoNode {

public:

	enum state {
		UNINITIALIZED,
		REQUESTED_WAITING_FOR_RESPONSE,
		REQUESTED_FAILED_WAITING_TO_RETRY,
		ACKED_WAITING_FOR_UPDATE,
		READY,
		ERROR
	};

	enum acknowledgementStatus {
		ACK_GOOD,
		ACK_REQUESTED_IDS_TAKEN,
		ACK_BOARD_ALREADY_EXISTS
	};

	CanAutoNode(FDCanController& contr, uint16_t msgIDsToRequestStartID, uint16_t msgIDsToRequestAmount);
    ~CanAutoNode();
	CanAutoNode() = delete;

	CanAutoNode(const CanAutoNode &) = delete;
	CanAutoNode &operator=(const CanAutoNode &) = delete;


	const state GetCurrentState() const;


	bool TryRequestingJoiningNetwork();


	bool CheckForAcknowledgement();
	bool CheckForJoinRequest();
	bool CheckForUpdate();

protected:
	FDCanController& controller;

	state currentState = UNINITIALIZED;

	uint32_t uniqueBoardID = HAL_GetDEVID();


	bool RequestToJoinNetwork(uint16_t requestID);

	bool ReceiveJoinRequest(uint8_t* msg);

	bool ReceiveUpdate(uint8_t* msg);

	bool SendAck(acknowledgementStatus status);

	bool SendFullUpdate();

	struct IDRange {
		uint32_t start;
		uint32_t end;
	};

	struct Node {
		IDRange requestedRange;
		uint32_t uniqueID;
	};

	const IDRange idRange;

	Node nodes[MAX_NODES_IN_NETWORK];
	uint16_t nodesInNetwork = 0;

	static uint32_t shift8to32(const uint8_t* in);
	static uint16_t shift8to16(const uint8_t *in);

	static bool IDRangesOverlap(IDRange a, IDRange b);

	static Node nodeFromMsg(const uint8_t* msg);
	static void msgFromNode(Node node, uint8_t* msgout);

};
