#pragma once

#include "ControlNode.h"

class BTextControl;


namespace HaikuReact {


// <TextControl>: BTextControl, a labeled single-line text field.
// Events: change (text) on every edit, submit (text) on Enter or when focus
// leaves after an edit.
class TextControlNode: public ControlNode {
private:
	BTextControl *fTextControl;

protected:
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit TextControlNode(uint32 id);

	const char *TypeName() const final {return "TextControl";}

	void SimulateInput(const EventValue &value) final;
	void Dump(Napi::Object &out) const final;
};


} // HaikuReact
