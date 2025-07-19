#include "FDCan.h"
#include "CanAutoNode.hpp"



class CanAutoNodeDaughter : public CanAutoNode {

public:

	enum daughterState {
		UNINITIALIZED,
		REQUESTED_WAITING_FOR_RESPONSE,
		REQUESTED_FAILED_WAITING_TO_RETRY,
		WAITING_FOR_UPDATE,
		READY,
		ERROR
	};

	enum acknowledgementStatus {
		ACK_GOOD,
		ACK_REQUESTED_IDS_TAKEN,
		ACK_BOARD_ALREADY_EXISTS
	};

	CanAutoNodeDaughter(FDCanController* contr, uint16_t msgIDsToRequestStartID, uint16_t msgIDsToRequestAmount);
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

protected:


//	uint32_t uniqueBoardID = HAL_GetDEVID();
	Node FSB = {0};
	bool CheckForAcknowledgement();
	bool CheckForUpdate();
	bool ProcessMessage();
	bool CheckForHeartbeat();

	daughterState currentState = UNINITIALIZED;

	bool RequestToJoinNetwork(uint16_t requestID);

	bool ReceiveUpdate(uint8_t* msg);

	uint32_t tickLastReceivedUpdatePart = 0;

//	const IDRange idRange;


};
