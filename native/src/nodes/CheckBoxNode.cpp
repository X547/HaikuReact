#include "CheckBoxNode.h"

#include <stdexcept>

#include <CheckBox.h>

#include "../NodeEventMessage.h"


namespace HaikuReact {


CheckBoxNode::CheckBoxNode(uint32 id):
	ControlNode(id, new BCheckBox("CheckBox", "", MakeNodeMessage(id, "change", EventValueKind::ControlBool)))
{
}


void CheckBoxNode::ApplyOwnProps(const PropSet &props)
{
	ControlNode::ApplyOwnProps(props);
	ApplyChecked(props);
}


void CheckBoxNode::SimulateInput(const EventValue &value)
{
	const bool *checked = std::get_if<bool>(&value);
	if (checked == nullptr) {
		throw std::invalid_argument("<CheckBox> input must be a boolean");
	}
	SimulateChecked(*checked);
}


} // HaikuReact
