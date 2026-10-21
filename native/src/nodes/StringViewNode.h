#pragma once

#include "ViewNode.h"

class BStringView;


namespace HaikuReact {


// <StringView>: BStringView, a static text label.
class StringViewNode: public ViewNode {
private:
	BStringView *fStringView;

protected:
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit StringViewNode(uint32 id);

	const char *TypeName() const final {return "StringView";}

	void Dump(Napi::Object &out) const final;
};


} // HaikuReact
