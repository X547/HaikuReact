#pragma once

#include "ControlNode.h"


namespace HaikuReact {


// <Button>: BButton. Events: click.
class ButtonNode: public ControlNode {
public:
	explicit ButtonNode(uint32 id);

	const char *TypeName() const final {return "Button";}
};


} // HaikuReact
