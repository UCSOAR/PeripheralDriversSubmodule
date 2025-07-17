#include "FDCan.h"
#include "CanAutoNode.hpp"


class CanAutoNodeFSB : public CanAutoNode {

public:

	bool CheckForJoinRequest();


private:
	bool ReceiveJoinRequest(uint8_t* msg);

	bool SendAck(acknowledgementStatus status);

	bool SendFullUpdate();



};
