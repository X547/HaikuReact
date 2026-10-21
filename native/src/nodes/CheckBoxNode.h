#pragma once

#include "ControlNode.h"


namespace HaikuReact {


// <CheckBox>: BCheckBox. Events: change (checked).
class CheckBoxNode: public ControlNode {
protected:
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit CheckBoxNode(uint32 id);

	const char *TypeName() const final {return "CheckBox";}

	void SimulateInput(const EventValue &value) final;
};


} // HaikuReact
