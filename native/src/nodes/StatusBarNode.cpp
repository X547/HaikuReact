#include "StatusBarNode.h"

#include <StatusBar.h>

#include "../PropSet.h"


namespace HaikuReact {


template<typename T>
static bool Update(const PropSet &props, const char *key, T &value, const T &defaultValue)
{
	const std::optional<T> newValue = props.Get<T>(key, defaultValue);
	if (!newValue.has_value() || *newValue == value) {
		return false;
	}
	value = *newValue;
	return true;
}


StatusBarNode::StatusBarNode(uint32 id):
	ViewNode(id, new BStatusBar("StatusBar", "", ""), kFillWidthMaxSize),
	fStatusBar(static_cast<BStatusBar*>(View()))
{
}


void StatusBarNode::ApplyOwnProps(const PropSet &props)
{
	ViewNode::ApplyOwnProps(props);

	bool labelsChanged = false;
	labelsChanged |= Update(props, "label", fLabel, std::string());
	labelsChanged |= Update(props, "trailingLabel", fTrailingLabel, std::string());
	bool stateChanged = labelsChanged;
	stateChanged |= Update(props, "text", fText, std::string());
	stateChanged |= Update(props, "trailingText", fTrailingText, std::string());
	stateChanged |= Update(props, "value", fValue, 0.0f);
	stateChanged |= Update(props, "maxValue", fMaxValue, 100.0f);
	if (!stateChanged) {
		return;
	}

	// The labels can only be replaced by Reset(), which also clears the
	// value, maximum and texts; apply those again afterwards.
	if (labelsChanged) {
		fStatusBar->Reset(fLabel.c_str(), fTrailingLabel.c_str());
	}
	fStatusBar->SetMaxValue(fMaxValue);
	fStatusBar->SetTo(fValue, fText.c_str(), fTrailingText.c_str());
}


void StatusBarNode::Dump(Napi::Object &out) const
{
	ViewNode::Dump(out);
	out.Set("label", fStatusBar->Label());
	out.Set("value", fStatusBar->CurrentValue());
	out.Set("maxValue", fStatusBar->MaxValue());
	out.Set("text", fStatusBar->Text());
}


} // HaikuReact
