#pragma once

#include <string>

#include "ControlNode.h"

class BSlider;


namespace HaikuReact {


// <Slider>: BSlider. Events: change (value) while dragging, changeEnd
// (value) when released.
class SliderNode: public ControlNode {
private:
	BSlider *fSlider;
	std::string fMinLabel;
	std::string fMaxLabel;

protected:
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit SliderNode(uint32 id);

	const char *TypeName() const final {return "Slider";}

	void SimulateInput(const EventValue &value) final;
};


} // HaikuReact
