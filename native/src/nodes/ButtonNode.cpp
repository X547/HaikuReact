#include "ButtonNode.h"

#include <Button.h>

#include "../NodeEventMessage.h"


namespace HaikuReact {


ButtonNode::ButtonNode(uint32 id):
	ControlNode(id, new BButton("Button", "", MakeNodeMessage(id, "click")))
{
}


} // HaikuReact
