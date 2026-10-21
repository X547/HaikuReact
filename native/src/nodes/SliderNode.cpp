#include "SliderNode.h"

#include <stdexcept>

#include <Slider.h>

#include "../NodeEventMessage.h"
#include "../PropSet.h"
#include "../StatusCheck.h"


namespace HaikuReact {


static hash_mark_location ParseHashMarks(const std::string &name)
{
	if (name == "none") {
		return B_HASH_MARKS_NONE;
	}
	if (name == "top") {
		return B_HASH_MARKS_TOP;
	}
	if (name == "bottom") {
		return B_HASH_MARKS_BOTTOM;
	}
	if (name == "both") {
		return B_HASH_MARKS_BOTH;
	}
	throw std::invalid_argument("prop \"hashMarks\": expected \"none\", \"top\", \"bottom\" or \"both\"");
}


SliderNode::SliderNode(uint32 id):
	ControlNode(
		id,
		new BSlider("Slider", "", MakeNodeMessage(id, "changeEnd", EventValueKind::ControlInt), 0, 100, B_HORIZONTAL)
	),
	fSlider(static_cast<BSlider*>(&Control()))
{
	fSlider->SetModificationMessage(MakeNodeMessage(id, "change", EventValueKind::ControlInt));
}


void SliderNode::ApplyOwnProps(const PropSet &props)
{
	ControlNode::ApplyOwnProps(props);

	int32 min = 0;
	int32 max = 0;
	fSlider->GetLimits(&min, &max);
	const std::optional<int32> newMin = props.Get<int32>("min", 0);
	const std::optional<int32> newMax = props.Get<int32>("max", 100);
	if (newMin.has_value() || newMax.has_value()) {
		fSlider->SetLimits(newMin.value_or(min), newMax.value_or(max));
	}

	if (auto value = props.Get<int32>("value", 0)) {
		fSlider->SetValue(*value);
	}
	if (auto direction = props.Get<orientation>("orientation", B_HORIZONTAL)) {
		fSlider->SetOrientation(*direction);
	}
	if (auto hashMarks = props.Get<std::string>("hashMarks", "none")) {
		fSlider->SetHashMarks(ParseHashMarks(*hashMarks));
	}
	if (auto count = props.Get<int32>("hashMarkCount", 0)) {
		fSlider->SetHashMarkCount(*count);
	}

	const std::optional<std::string> minLabel = props.Get<std::string>("minLabel", "");
	const std::optional<std::string> maxLabel = props.Get<std::string>("maxLabel", "");
	if (minLabel.has_value() || maxLabel.has_value()) {
		fMinLabel = minLabel.value_or(fMinLabel);
		fMaxLabel = maxLabel.value_or(fMaxLabel);
		fSlider->SetLimitLabels(
			fMinLabel.empty() ? nullptr : fMinLabel.c_str(),
			fMaxLabel.empty() ? nullptr : fMaxLabel.c_str()
		);
	}
}


void SliderNode::SimulateInput(const EventValue &value)
{
	const double *number = std::get_if<double>(&value);
	if (number == nullptr) {
		throw std::invalid_argument("<Slider> input must be a number");
	}
	fSlider->SetValue(static_cast<int32>(*number));
	CheckThrow(fSlider->Invoke(fSlider->ModificationMessage()));
}


} // HaikuReact
