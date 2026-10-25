#pragma once

#include <string>
#include <vector>

#include "../PropSet.h"
#include "ViewNode.h"

class BMenuField;
class BPopUpMenu;


namespace HaikuReact {


// <MenuField>: BMenuField with a pop-up menu of items (text, optionally an
// icon, which only the pop-up shows); the marked item is shown as the
// field's value. Events: change (index).
class MenuFieldNode: public ViewNode {
private:
	BMenuField *fMenuField;
	BPopUpMenu *fMenu;
	std::vector<LabeledItem> fItems;
	int32 fValue = -1;

	void SetItems(const std::vector<LabeledItem> &items);
	void MarkValue();

protected:
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit MenuFieldNode(uint32 id);

	const char *TypeName() const final {return "MenuField";}

	void SimulateInput(const EventValue &value) final;
	void Dump(Napi::Object &out) const final;
};


} // HaikuReact
