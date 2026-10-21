#pragma once

#include <memory>
#include <string>

#include <SupportDefs.h>


namespace HaikuReact {


class Node;
class PropSet;


class NodeFactory {
public:
	// The props are the initial ones; node types with creation-only props
	// read them here, all props are applied afterwards.
	static std::unique_ptr<Node> Create(const std::string &type, uint32 id, const PropSet &props);
};


} // HaikuReact
