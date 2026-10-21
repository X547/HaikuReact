#include "MenuBarNode.h"

#include <MenuBar.h>

#include "../MenuChildren.h"


namespace HaikuReact {


MenuBarNode::MenuBarNode(uint32 id):
	ViewNode(id, new BMenuBar("MenuBar"), kFillWidthMaxSize),
	fMenuBar(static_cast<BMenuBar*>(View()))
{
}


void MenuBarNode::AttachChild(Node &child, int32 index)
{
	MenuChildren::Attach(*fMenuBar, child, index);
}


void MenuBarNode::DetachChild(Node &child)
{
	MenuChildren::Detach(*fMenuBar, child);
}


} // HaikuReact
