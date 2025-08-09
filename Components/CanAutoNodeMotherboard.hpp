#include "FDCan.h"
#include "CanAutoNode.hpp"

class CanAutoNodeMotherboard : public CanAutoNode {

public:


	CanAutoNodeMotherboard(FDCAN_HandleTypeDef *fdcan);

	bool CheckMessages() override {
		return CheckForJoinRequest();
	}
	bool KickNode(UniqueBoardID uniqueBoardID);

	bool Heartbeat();

	uint32_t GetTicksSinceLastHeartbeat() const;

private:
	bool ReceiveJoinRequest(uint8_t* msg);

	bool SendAck(acknowledgementStatus status);

	bool SendFullUpdate();

	bool CheckForJoinRequest();

	uint32_t lastHeartbeatTick = 0;


};
