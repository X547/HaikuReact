#include "MenuBarNode.h"

#include "../IconSource.h"
#include "../MenuChildren.h"
#include "../ReactMenu.h"


namespace HaikuReact {


MenuBarNode::MenuBarNode(uint32 id):
	ViewNode(id, new ReactMenuBar(), kFillWidthMaxSize),
	fMenuBar(static_cast<BMenuBar*>(View()))
{
}


void MenuBarNode::AttachChild(Node &child, int32 index)
{
	MenuChildren::Attach(*fMenuBar, child, index, ScaledIconSize(fIconSize));
}


void MenuBarNode::DetachChild(Node &child)
{
	MenuChildren::Detach(*fMenuBar, child);
}


void MenuBarNode::ApplyOwnProps(const PropSet &props)
{
	ViewNode::ApplyOwnProps(props);
	MenuChildren::ApplyIconSize(props, fIconSize, Children());
}


} // HaikuReact
