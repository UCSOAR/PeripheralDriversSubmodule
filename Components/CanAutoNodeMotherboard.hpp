#include "FDCan.h"
#include "CanAutoNode.hpp"


class CanAutoNodeMotherboard : public CanAutoNode {

public:



	bool CheckMessages() override {
		return CheckForJoinRequest();
	}
	bool KickNode(UniqueBoardID uniqueBoardID);

	bool Heartbeat();

private:
	bool ReceiveJoinRequest(uint8_t* msg);

	bool SendAck(acknowledgementStatus status);

	bool SendFullUpdate();

	bool CheckForJoinRequest();


};
