#include "SubmenuNode.h"

#include <stdexcept>
#include <utility>

#include <MenuBar.h>
#include <Messenger.h>
#include <Window.h>

#include "../IconSource.h"
#include "../MenuChildren.h"
#include "../PropSet.h"
#include "../ReactMenu.h"
#include "../ReactMenuItem.h"
#include "../StatusCheck.h"


namespace HaikuReact {


static constexpr const char *kDefaultLoadingLabel = "Loading…";


// The superitem owns the menu.
SubmenuNode::SubmenuNode(uint32 id):
	MenuEntryNode(id, new ReactMenuItem(new ReactMenu(id))),
	fMenu(static_cast<ReactMenu*>(Item()->Submenu())),
	fLoadingLabel(kDefaultLoadingLabel)
{
}


void SubmenuNode::UpdateContent()
{
	std::vector<BMenuItem*> items;
	for (const std::unique_ptr<Node> &child : Children()) {
		items.push_back(static_cast<MenuEntryNode&>(*child).Item());
	}
	const bool placeholder = fLoading || (fLoadsOnOpen && items.empty());
	fMenu->SetContent(items, placeholder, fLoadingLabel);
}


// The item is added in ChildListChanged(), which also handles moves and
// removals; while the menu is shown, the menu's thread applies them.
void SubmenuNode::AttachChild(Node &child, int32 index)
{
	(void)index;
	MenuChildren::Validate(child, ScaledIconSize(fIconSize));
}


void SubmenuNode::ChildListChanged()
{
	UpdateContent();
}


void SubmenuNode::ApplyOwnProps(const PropSet &props)
{
	MenuEntryNode::ApplyOwnProps(props);

	if (auto radioMode = props.Get<bool>("radioMode", false)) {
		fMenu->SetRadioMode(*radioMode);
	}
	MenuChildren::ApplyIconSize(props, fIconSize, Children());

	bool contentChanged = false;
	if (auto loading = props.Get<bool>("loading", false)) {
		fLoading = *loading;
		contentChanged = true;
	}
	if (auto loadsOnOpen = props.Get<bool>("loadsOnOpen", false)) {
		fLoadsOnOpen = *loadsOnOpen;
		contentChanged = true;
	}
	if (auto loadingLabel = props.Get<std::string>("loadingLabel", kDefaultLoadingLabel)) {
		fLoadingLabel = *loadingLabel;
		contentChanged = true;
	}
	if (contentChanged) {
		UpdateContent();
	}
}


static void SendKey(const BMessenger &target, char key)
{
	BMessage message(B_KEY_DOWN);
	const char bytes[2] = {key, '\0'};
	message.AddString("bytes", bytes);
	message.AddInt32("raw_char", key);
	message.AddInt32("modifiers", 0);
	CheckThrow(target.SendMessage(&message));
}


void SubmenuNode::SimulateInput(const EventValue &value)
{
	if (const auto *key = std::get_if<std::string>(&value)) {
		// Keyboard navigation in the open menu.
		static const std::pair<const char*, char> kKeys[] = {
			{"up", B_UP_ARROW}, {"down", B_DOWN_ARROW}, {"left", B_LEFT_ARROW}, {"right", B_RIGHT_ARROW},
			{"enter", B_ENTER},
		};
		for (const auto &[name, code] : kKeys) {
			if (*key == name && fMenu->IsShown()) {
				SendKey(BMessenger(fMenu), code);
				return;
			}
		}
		throw std::invalid_argument("<Menu> key input must be up, down, left, right or enter, for an open menu");
	}

	const bool *open = std::get_if<bool>(&value);
	if (open != nullptr && !*open && fMenu->IsShown()) {
		// Escape in a menu opened from the keyboard returns to its parent.
		SendKey(BMessenger(fMenu), B_ESCAPE);
		return;
	}
	auto *menuBar = dynamic_cast<ReactMenuBar*>(fMenu->Supermenu());
	if (open == nullptr || menuBar == nullptr) {
		throw std::invalid_argument("<Menu> input must be a boolean or a key, and the menu must be in a <MenuBar>");
	}
	if (*open) {
		menuBar->OpenMenu(menuBar->IndexOf(Item()));
		return;
	}
	// With the menu closed, Escape leaves the menu bar's keyboard mode.
	SendKey(BMessenger(menuBar), B_ESCAPE);
}


void SubmenuNode::Dump(Napi::Object &out) const
{
	MenuEntryNode::Dump(out);
	Napi::Env env = out.Env();
	out.Set("open", fMenu->IsShown());
	const std::vector<std::string> labels = fMenu->ItemLabels();
	Napi::Array items = Napi::Array::New(env, labels.size());
	for (uint32 i = 0; i < labels.size(); i++) {
		items.Set(i, labels[i]);
	}
	out.Set("menuItems", items);
}


} // HaikuReact
