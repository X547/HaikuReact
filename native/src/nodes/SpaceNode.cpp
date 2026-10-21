#include "SpaceNode.h"

#include <SpaceLayoutItem.h>


namespace HaikuReact {


SpaceNode::SpaceNode(uint32 id, BSpaceLayoutItem *item):
	Node(id),
	fItem(item)
{
}


SpaceNode::~SpaceNode()
{
	if (Parent() == nullptr) {
		delete fItem;
	}
}


BLayoutItem *SpaceNode::LayoutItem() const
{
	return fItem;
}


void SpaceNode::ApplyOwnProps(const PropSet &props)
{
	(void)props;
}


//#pragma mark - GlueNode

GlueNode::GlueNode(uint32 id):
	SpaceNode(id, BSpaceLayoutItem::CreateGlue())
{
}


} // HaikuReact
