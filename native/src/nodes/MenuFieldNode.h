#pragma once

#include <string>
#include <vector>

#include "ViewNode.h"

class BMenuField;
class BPopUpMenu;


namespace HaikuReact {


// <MenuField>: BMenuField with a pop-up menu of text items; the marked item
// is shown as the field's value. Events: change (index).
class MenuFieldNode: public ViewNode {
private:
	BMenuField *fMenuField;
	BPopUpMenu *fMenu;
	std::vector<std::string> fItems;
	int32 fValue = -1;

	void SetItems(const std::vector<std::string> &items);
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
