#include "FDCan.h"
#include "CanAutoNode.hpp"

class CanAutoNodeMotherboard : public CanAutoNode {

public:


	CanAutoNodeMotherboard(FDCAN_HandleTypeDef *fdcan);

	bool CheckCANCommands() override {
		return CheckForJoinRequest();
	}
	bool KickNode(UniqueBoardID uniqueBoardID);
	bool KickNode(const char* boardName);
	bool KickNode(uint16_t slotNumber);

	bool Heartbeat();

	uint32_t GetTicksSinceLastHeartbeat() const;

	/* Gets the total number of nodes in the network, including the motherboard.
	 * Note that this is different from the size of the internal node table.
	 */
	uint16_t GetNumberOfNodesInNetwork() const override {
		return nodesInNetwork+1;
	}

	bool ReadMessageFromDaughterByLogIndex(UniqueBoardID daughter, uint8_t logIndex, uint8_t *out, uint16_t outSize);

private:
	bool ReceiveJoinRequest(uint8_t* msg);

	bool SendAck(acknowledgementStatus status);

	bool SendFullUpdate();

	bool CheckForJoinRequest();

	uint32_t lastHeartbeatTick = 0;

	uint16_t nextFreeMotherboardLogIndex = 0;

};
