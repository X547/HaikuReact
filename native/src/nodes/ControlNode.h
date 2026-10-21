#pragma once

#include "ViewNode.h"

class BControl;


namespace HaikuReact {


// Node backed by a BControl: label and enabled props; Invoke() sends the
// control's message like a user action would.
class ControlNode: public ViewNode {
private:
	BControl *fControl;

protected:
	void ApplyOwnProps(const PropSet &props) override;

	BControl &Control() const {return *fControl;}

	// For on/off controls: the checked prop, and a user-like change of it.
	void ApplyChecked(const PropSet &props);
	void SimulateChecked(bool checked);

public:
	ControlNode(uint32 id, BControl *control);

	void Invoke() override;
	void Dump(Napi::Object &out) const override;
};


} // HaikuReact
