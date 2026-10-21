#include "NodeRegistry.h"

#include <stdexcept>
#include <string>
#include <unordered_map>

#include "nodes/Node.h"


namespace HaikuReact {


static uint32 sLastId = 0;
static std::unordered_map<uint32, Node*> sNodes;
static std::unordered_map<uint32, std::unique_ptr<Node>> sRoots;


uint32 NodeRegistry::NextId()
{
	return ++sLastId;
}


Node &NodeRegistry::Get(uint32 id)
{
	const auto it = sNodes.find(id);
	if (it == sNodes.end()) {
		throw std::invalid_argument("unknown node id " + std::to_string(id));
	}
	return *it->second;
}


void NodeRegistry::Forget(uint32 id)
{
	sNodes.erase(id);
}


void NodeRegistry::AddRoot(std::unique_ptr<Node> node)
{
	const uint32 id = node->Id();
	sNodes[id] = node.get();
	sRoots[id] = std::move(node);
}


std::unique_ptr<Node> NodeRegistry::TakeRoot(uint32 id)
{
	const auto it = sRoots.find(id);
	if (it == sRoots.end()) {
		throw std::invalid_argument("node " + std::to_string(id) + " is already attached");
	}
	std::unique_ptr<Node> node = std::move(it->second);
	sRoots.erase(it);
	return node;
}


void NodeRegistry::DestroyRoot(uint32 id)
{
	const auto it = sRoots.find(id);
	if (it == sRoots.end()) {
		return;
	}
	// Detach from the map before destruction: the destructor calls Forget().
	std::unique_ptr<Node> node = std::move(it->second);
	sRoots.erase(it);
	node.reset();
}


void NodeRegistry::DestroyAll()
{
	auto roots = std::move(sRoots);
	sRoots.clear();
	roots.clear();
}


size_t NodeRegistry::Count()
{
	return sNodes.size();
}


} // HaikuReact
