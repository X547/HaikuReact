#include "RadioButtonNode.h"

#include <RadioButton.h>

#include "../NodeEventMessage.h"


namespace HaikuReact {


RadioButtonNode::RadioButtonNode(uint32 id):
	ControlNode(id, new BRadioButton("RadioButton", "", MakeNodeMessage(id, "change", EventValueKind::ControlBool)))
{
}


void RadioButtonNode::ApplyOwnProps(const PropSet &props)
{
	ControlNode::ApplyOwnProps(props);
	ApplyChecked(props);
}


void RadioButtonNode::SimulateInput(const EventValue &value)
{
	(void)value;
	SimulateChecked(true);
}


} // HaikuReact
