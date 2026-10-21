#pragma once

#include "MenuEntryNode.h"


namespace HaikuReact {


// <MenuItem>: BMenuItem. Shortcuts use the Command key plus the given
// modifiers. Events: click.
class MenuItemNode: public MenuEntryNode {
private:
	char fShortcut = 0;
	uint32 fModifiers = 0;

protected:
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit MenuItemNode(uint32 id);

	const char *TypeName() const final {return "MenuItem";}

	void Invoke() final;
	void Dump(Napi::Object &out) const final;
};


} // HaikuReact
