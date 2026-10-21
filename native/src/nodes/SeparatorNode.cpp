#include "SeparatorNode.h"

#include <MenuItem.h>


namespace HaikuReact {


SeparatorNode::SeparatorNode(uint32 id):
	MenuEntryNode(id, new BSeparatorItem())
{
}


void SeparatorNode::ApplyOwnProps(const PropSet &props)
{
	(void)props;
}


} // HaikuReact
