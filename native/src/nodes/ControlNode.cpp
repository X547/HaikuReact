#include "ControlNode.h"

#include <string>

#include <Control.h>

#include "../PropSet.h"
#include "../StatusCheck.h"


namespace HaikuReact {


ControlNode::ControlNode(uint32 id, BControl *control):
	ViewNode(id, control),
	fControl(control)
{
}


void ControlNode::ApplyOwnProps(const PropSet &props)
{
	ViewNode::ApplyOwnProps(props);

	if (auto label = props.Get<std::string>("label", "")) {
		fControl->SetLabel(label->c_str());
	}
	if (auto enabled = props.Get<bool>("enabled", true)) {
		fControl->SetEnabled(*enabled);
	}
}


void ControlNode::ApplyChecked(const PropSet &props)
{
	if (auto checked = props.Get<bool>("checked", false)) {
		fControl->SetValue(*checked ? B_CONTROL_ON : B_CONTROL_OFF);
	}
}


void ControlNode::SimulateChecked(bool checked)
{
	fControl->SetValue(checked ? B_CONTROL_ON : B_CONTROL_OFF);
	CheckThrow(fControl->Invoke());
}


void ControlNode::Invoke()
{
	CheckThrow(fControl->Invoke());
}


void ControlNode::Dump(Napi::Object &out) const
{
	ViewNode::Dump(out);
	out.Set("label", fControl->Label() != nullptr ? fControl->Label() : "");
	out.Set("enabled", fControl->IsEnabled());
	out.Set("value", fControl->Value());
}


} // HaikuReact
