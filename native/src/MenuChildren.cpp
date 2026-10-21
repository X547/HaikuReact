#include "MenuChildren.h"

#include <stdexcept>
#include <string>

#include <Menu.h>

#include "nodes/MenuEntryNode.h"


namespace HaikuReact {


namespace MenuChildren {


static MenuEntryNode &AsEntry(Node &child)
{
	auto *entry = dynamic_cast<MenuEntryNode*>(&child);
	if (entry == nullptr) {
		throw std::invalid_argument(
			std::string("menus can only contain <Menu>, <MenuItem> and <Separator>, not <") + child.TypeName() + ">"
		);
	}
	return *entry;
}


void Attach(BMenu &menu, Node &child, int32 index)
{
	// Installs the item into the window: message target and shortcut.
	if (!menu.AddItem(AsEntry(child).Item(), index)) {
		throw std::runtime_error(std::string("failed to add <") + child.TypeName() + "> to the menu");
	}
}


void Detach(BMenu &menu, Node &child)
{
	// Uninstalls the item without deleting it.
	menu.RemoveItem(AsEntry(child).Item());
}


} // MenuChildren


} // HaikuReact
