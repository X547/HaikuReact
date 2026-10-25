#pragma once

#include <string>

#include "MenuEntryNode.h"


namespace HaikuReact {


class ReactMenu;


// <Menu>: a ReactMenu, shown through its superitem in the parent menu or menu
// bar. In radio mode, choosing an item marks it and unmarks the others.
// A disabled "Loading…" item shows while loading is set, or while the menu
// loads its items on open (onOpen) and has none. Events: open, close.
class SubmenuNode: public MenuEntryNode {
private:
	ReactMenu *fMenu;
	float fIconSize = 16;
	bool fLoading = false;
	bool fLoadsOnOpen = false;
	std::string fLoadingLabel;

	void UpdateContent();

protected:
	void AttachChild(Node &child, int32 index) final;
	void ChildListChanged() final;
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit SubmenuNode(uint32 id);

	const char *TypeName() const final {return "Menu";}

	// true opens the menu as from the keyboard (menus in a menu bar only);
	// false sends Escape to the open menu, or to the menu bar when it is
	// closed. A key name
	// ("up", "down", "left", "right", "enter") goes to the open menu.
	void SimulateInput(const EventValue &value) final;
	void Dump(Napi::Object &out) const final;
};


} // HaikuReact
