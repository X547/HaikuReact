#pragma once

#include <string>

#include "ViewNode.h"

class BStatusBar;


namespace HaikuReact {


// <StatusBar>: BStatusBar, a progress bar with labels above it.
class StatusBarNode: public ViewNode {
private:
	BStatusBar *fStatusBar;
	std::string fLabel;
	std::string fTrailingLabel;
	std::string fText;
	std::string fTrailingText;
	float fValue = 0;
	float fMaxValue = 100;

protected:
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit StatusBarNode(uint32 id);

	const char *TypeName() const final {return "StatusBar";}

	void Dump(Napi::Object &out) const final;
};


} // HaikuReact
