#include "SubmenuNode.h"

#include <Menu.h>
#include <MenuItem.h>

#include "../MenuChildren.h"
#include "../PropSet.h"


namespace HaikuReact {


// The superitem owns the menu.
SubmenuNode::SubmenuNode(uint32 id):
	MenuEntryNode(id, new BMenuItem(new BMenu(""))),
	fMenu(Item()->Submenu())
{
}


void SubmenuNode::AttachChild(Node &child, int32 index)
{
	MenuChildren::Attach(*fMenu, child, index);
}


void SubmenuNode::DetachChild(Node &child)
{
	MenuChildren::Detach(*fMenu, child);
}


void SubmenuNode::ApplyOwnProps(const PropSet &props)
{
	MenuEntryNode::ApplyOwnProps(props);

	if (auto radioMode = props.Get<bool>("radioMode", false)) {
		fMenu->SetRadioMode(*radioMode);
	}
}


} // HaikuReact
