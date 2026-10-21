#pragma once

#include <memory>

#include <SupportDefs.h>


namespace HaikuReact {


class Node;


// Maps node ids to nodes and owns all parentless nodes ("roots"): freshly
// created nodes, detached subtrees and mounted windows. JS thread only.
class NodeRegistry {
public:
	static uint32 NextId();

	static Node &Get(uint32 id);
	static void Forget(uint32 id);

	static void AddRoot(std::unique_ptr<Node> node);
	static std::unique_ptr<Node> TakeRoot(uint32 id);
	// No-op unless the node is a root.
	static void DestroyRoot(uint32 id);
	static void DestroyAll();

	// Live nodes, attached or not; for leak checks.
	static size_t Count();
};


} // HaikuReact
