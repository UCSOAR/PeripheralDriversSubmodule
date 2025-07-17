#include "FDCan.h"
#include "CanAutoNode.hpp"



class CanAutoNodeDaughter : public CanAutoNode {

public:

	enum daughterState {
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

	CanAutoNodeDaughter(FDCanController* contr, uint16_t msgIDsToRequestStartID, uint16_t msgIDsToRequestAmount);
    ~CanAutoNodeDaughter();
	CanAutoNodeDaughter() = delete;

	CanAutoNodeDaughter(const CanAutoNodeDaughter &) = delete;
	CanAutoNodeDaughter &operator=(const CanAutoNodeDaughter &) = delete;


	const daughterState GetCurrentState() const;


	bool TryRequestingJoiningNetwork();


	bool CheckForAcknowledgement();
	bool CheckForUpdate();

protected:


//	uint32_t uniqueBoardID = HAL_GetDEVID();

	daughterState currentState = UNINITIALIZED;


	bool RequestToJoinNetwork(uint16_t requestID);

	bool ReceiveUpdate(uint8_t* msg);

//	const IDRange idRange;


};
