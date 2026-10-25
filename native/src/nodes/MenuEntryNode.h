#pragma once

#include "Node.h"

class BMenuItem;


namespace HaikuReact {


class ReactMenuItem;


// Node backed by a BMenuItem: an item, a submenu (through its superitem) or
// a separator. A menu deletes its items, so the node owns the item only
// while it has no parent.
class MenuEntryNode: public Node {
private:
	BMenuItem *fItem;

protected:
	// label, enabled and, for ReactMenuItems, icon.
	void ApplyOwnProps(const PropSet &props) override;

public:
	MenuEntryNode(uint32 id, BMenuItem *item);
	~MenuEntryNode() override;

	BMenuItem *Item() const {return fItem;}

	void Dump(Napi::Object &out) const override;
};


// Icon and size of an item, for dumps.
void DumpMenuItem(Napi::Object &out, ReactMenuItem &item);


} // HaikuReact
