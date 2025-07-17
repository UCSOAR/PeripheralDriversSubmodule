/*
 * CanAutoNode.cpp
 *
 *  Created on: Jul 16, 2025
 *      Author: Local user
 */

#include <CanAutoNode.hpp>
#include <cstring>



CanAutoNode::~CanAutoNode() {

}

uint32_t CanAutoNode::shift8to32(const uint8_t *in) {
	return (static_cast<uint32_t>(in[0])<<24) | (static_cast<uint32_t>(in[1])<<16) | (static_cast<uint32_t>(in[2])<<8) | in[3];
}

uint16_t CanAutoNode::shift8to16(const uint8_t *in) {
	return (static_cast<uint16_t>(in[0])<<8) | in[1];
}

bool CanAutoNode::IDRangesOverlap(IDRange a, IDRange b) {
	return a.start < b.end && a.end > b.start;
}

CanAutoNode::Node CanAutoNode::nodeFromMsg(const uint8_t *msg) {

	Node newNode;
	newNode.uniqueID = shift8to32(msg);
	newNode.requestedRange.start = shift8to32(msg+4);
	newNode.requestedRange.end = shift8to32(msg+8);
	return newNode;

}

void CanAutoNode::msgFromNode(Node node, uint8_t *msgout) {

	memcpy(msgout,&node.uniqueID,sizeof(node.uniqueID));
	memcpy(msgout+4,&node.requestedRange.start,sizeof(node.requestedRange.start));
	memcpy(msgout+8,&node.requestedRange.end,sizeof(node.requestedRange.end));

}

CanAutoNode::CanAutoNode() {
}
