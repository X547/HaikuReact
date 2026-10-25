#include "MenuChildren.h"

#include <stdexcept>
#include <string>

#include <Menu.h>

#include "PropSet.h"
#include "ReactMenuItem.h"
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


static void SetIconSize(Node &child, int32 iconSize)
{
	if (auto *item = dynamic_cast<ReactMenuItem*>(AsEntry(child).Item())) {
		item->SetIconSize(iconSize);
	}
}


void Validate(Node &child, int32 iconSize)
{
	SetIconSize(child, iconSize);
}


void Attach(BMenu &menu, Node &child, int32 index, int32 iconSize)
{
	Validate(child, iconSize);
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


bool ApplyIconSize(const PropSet &props, float &nominalSize, const std::vector<std::unique_ptr<Node>> &children)
{
	const std::optional<float> size = props.Get<float>("iconSize", 16);
	if (!size.has_value() || *size == nominalSize) {
		return false;
	}
	nominalSize = *size;
	for (const std::unique_ptr<Node> &child : children) {
		SetIconSize(*child, ScaledIconSize(nominalSize));
	}
	return true;
}


} // MenuChildren


} // HaikuReact
