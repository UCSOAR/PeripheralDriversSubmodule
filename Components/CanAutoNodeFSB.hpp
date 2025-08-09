#include "FDCan.h"
#include "CanAutoNode.hpp"


class CanAutoNodeFSB : public CanAutoNode {

public:



	bool CheckMessages() override {
		return CheckForJoinRequest();
	}
	bool KickNode(UniqueBoardID uniqueBoardID);

	bool Heartbeat();

//	bool SendToDaughterBoardByIndex(uint32_t uniqueID, uint8_t logIndex, const uint8_t* msg);

private:
	bool ReceiveJoinRequest(uint8_t* msg);

	bool SendAck(acknowledgementStatus status);

	bool SendFullUpdate();

	bool CheckForJoinRequest();


};
