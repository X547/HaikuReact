#pragma once

#include "MenuEntryNode.h"

class BMenu;


namespace HaikuReact {


// <Menu>: a BMenu, shown through its superitem in the parent menu or menu
// bar. In radio mode, choosing an item marks it and unmarks the others.
class SubmenuNode: public MenuEntryNode {
private:
	BMenu *fMenu;

protected:
	void AttachChild(Node &child, int32 index) final;
	void DetachChild(Node &child) final;
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit SubmenuNode(uint32 id);

	const char *TypeName() const final {return "Menu";}
};


} // HaikuReact
