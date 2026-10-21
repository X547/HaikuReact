#pragma once

#include "ControlNode.h"


namespace HaikuReact {


// <RadioButton>: BRadioButton. Selecting one turns off the radio buttons
// with the same parent view. Events: change (checked).
class RadioButtonNode: public ControlNode {
protected:
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit RadioButtonNode(uint32 id);

	const char *TypeName() const final {return "RadioButton";}

	// Selects the radio button; the value is ignored.
	void SimulateInput(const EventValue &value) final;
};


} // HaikuReact
