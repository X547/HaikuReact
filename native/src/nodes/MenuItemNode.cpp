#include "MenuItemNode.h"

#include <string>
#include <vector>

#include <Menu.h>
#include <MenuItem.h>

#include "../KeyModifiers.h"
#include "../NodeEventMessage.h"
#include "../PropSet.h"
#include "../StatusCheck.h"


namespace HaikuReact {


MenuItemNode::MenuItemNode(uint32 id):
	MenuEntryNode(id, new BMenuItem("", MakeNodeMessage(id, "click")))
{
}


void MenuItemNode::ApplyOwnProps(const PropSet &props)
{
	MenuEntryNode::ApplyOwnProps(props);

	if (auto marked = props.Get<bool>("marked", false)) {
		Item()->SetMarked(*marked);
	}

	bool shortcutChanged = false;
	if (auto shortcut = props.Get<std::string>("shortcut", "")) {
		fShortcut = shortcut->empty() ? 0 : (*shortcut)[0];
		shortcutChanged = true;
	}
	if (auto modifiers = props.Get<std::vector<std::string>>("modifiers", {})) {
		fModifiers = ParseModifiers(*modifiers);
		shortcutChanged = true;
	}
	if (shortcutChanged) {
		// Command is implied by menu shortcuts. SetShortcut() re-registers the
		// shortcut with the window if the item is installed.
		Item()->SetShortcut(fShortcut, fShortcut != 0 ? fModifiers | B_COMMAND_KEY : 0);
	}
}


void MenuItemNode::Invoke()
{
	// What choosing the item does: mark it in a radio-mode menu and send its
	// message to its target, the window.
	BMenuItem &item = *Item();
	if (item.Menu() != nullptr && item.Menu()->IsRadioMode()) {
		item.SetMarked(true);
	}
	CheckThrow(item.Messenger().SendMessage(item.Message()));
}


void MenuItemNode::Dump(Napi::Object &out) const
{
	MenuEntryNode::Dump(out);
	out.Set("marked", Item()->IsMarked());

	uint32 modifiers = 0;
	const char shortcut = Item()->Shortcut(&modifiers);
	out.Set("shortcut", shortcut != 0 ? std::string(1, shortcut) : std::string());
}


} // HaikuReact
