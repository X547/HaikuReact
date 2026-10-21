#pragma once

#include "MenuEntryNode.h"


namespace HaikuReact {


// <Separator>: BSeparatorItem, a line between menu items.
class SeparatorNode: public MenuEntryNode {
protected:
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit SeparatorNode(uint32 id);

	const char *TypeName() const final {return "Separator";}
};


} // HaikuReact
