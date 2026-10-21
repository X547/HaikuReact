#pragma once

#include "ViewNode.h"


namespace HaikuReact {


class ReactTextView;


// <TextView>: multi-line text; place it in a <Scroll> to scroll.
// Events: change (text).
class TextViewNode: public ViewNode {
private:
	ReactTextView *fTextView;

protected:
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit TextViewNode(uint32 id);

	const char *TypeName() const final {return "TextView";}
	bool ScrollsItself() const final {return true;}

	void SimulateInput(const EventValue &value) final;
	void Dump(Napi::Object &out) const final;
};


} // HaikuReact
