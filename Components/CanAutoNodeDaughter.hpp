#include "FDCan.h"
#include "CanAutoNode.hpp"


class CanAutoNodeDaughter : public CanAutoNode {

public:

	struct LogInit {
		uint16_t sizeInBytes;
	};
	enum daughterState {
		UNINITIALIZED,
		REQUESTED_WAITING_FOR_RESPONSE,
		REQUESTED_FAILED_WAITING_TO_RETRY,
		WAITING_FOR_UPDATE,
		READY,
		ERROR
	};

	//CanAutoNodeDaughter(FDCanController* contr, uint16_t msgIDsToRequestStartID, uint16_t msgIDsToRequestAmount);
    CanAutoNodeDaughter(FDCAN_HandleTypeDef *fdcan,
    		const LogInit *logs, uint16_t numLogs, uint8_t boardType, uint8_t slotNumber);
	~CanAutoNodeDaughter();
	CanAutoNodeDaughter() = delete;

	CanAutoNodeDaughter(const CanAutoNodeDaughter &) = delete;
	CanAutoNodeDaughter &operator=(const CanAutoNodeDaughter &) = delete;


	const daughterState GetCurrentState() const;

	void ChangeState(daughterState target);


	bool TryRequestingJoiningNetwork();

	bool CheckMessages() override {
		switch(currentState) {
		case REQUESTED_WAITING_FOR_RESPONSE:
			return CheckForAcknowledgement();
		case WAITING_FOR_UPDATE:
			return CheckForUpdate();
		case READY:
			return ProcessMessage();
		default:
			return false;
		}
	}

	/* Gets the total number of nodes in the network, including the motherboard and this node.
	 * Note that this is different from the size of the internal node table.
	 */
	uint16_t GetNumberOfNodesInNetwork() const override {
		return nodesInNetwork+2;
	}

	bool SendMessageToMotherboardByLogID(uint16_t logID, const uint8_t* msg);

protected:

//	uint32_t uniqueBoardID = HAL_GetDEVID();
	Node Motherboard = {0};
	bool CheckForAcknowledgement();
	bool CheckForUpdate();
	bool ProcessMessage();
	bool CheckForHeartbeat();

	daughterState currentState = UNINITIALIZED;

	bool RequestToJoinNetwork();

	bool ReceiveUpdate(const uint8_t* msg);

	uint32_t tickLastReceivedUpdatePart = 0;

	LogInit logsToInit[MAX_LOGS];
	uint16_t numLogs;

	FDCanController::LogInitStruct determinedLogs[MAX_LOGS];

	bool initializedLogs = false;

//	const uint8_t boardType;
//	const IDRange idRange;

};
