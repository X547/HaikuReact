#include "NodeFactory.h"

#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <unordered_map>

#include "PropSet.h"
#include "nodes/BoxNode.h"
#include "nodes/ButtonNode.h"
#include "nodes/CardsNode.h"
#include "nodes/CheckBoxNode.h"
#include "nodes/GridNode.h"
#include "nodes/GroupNode.h"
#include "nodes/ListViewNode.h"
#include "nodes/MenuBarNode.h"
#include "nodes/MenuFieldNode.h"
#include "nodes/MenuItemNode.h"
#include "nodes/RadioButtonNode.h"
#include "nodes/ScrollNode.h"
#include "nodes/SeparatorNode.h"
#include "nodes/SliderNode.h"
#include "nodes/SpaceNode.h"
#include "nodes/SplitNode.h"
#include "nodes/StatusBarNode.h"
#include "nodes/StringViewNode.h"
#include "nodes/StrutNode.h"
#include "nodes/SubmenuNode.h"
#include "nodes/TabNode.h"
#include "nodes/TabsNode.h"
#include "nodes/TextControlNode.h"
#include "nodes/TextViewNode.h"
#include "nodes/WindowNode.h"


namespace HaikuReact {


using NodeCreator = std::unique_ptr<Node> (*)(uint32 id, const PropSet &props);


template<typename T>
static std::unique_ptr<Node> CreateNode(uint32 id, const PropSet &props)
{
	if constexpr (std::is_constructible_v<T, uint32, const PropSet&>) {
		return std::make_unique<T>(id, props);
	} else {
		(void)props;
		return std::make_unique<T>(id);
	}
}


static const std::unordered_map<std::string_view, NodeCreator> kCreators = {
	{"Box",         CreateNode<BoxNode>},
	{"Button",      CreateNode<ButtonNode>},
	{"Cards",       CreateNode<CardsNode>},
	{"CheckBox",    CreateNode<CheckBoxNode>},
	{"Glue",        CreateNode<GlueNode>},
	{"Grid",        CreateNode<GridNode>},
	{"Group",       CreateNode<GroupNode>},
	{"ListView",    CreateNode<ListViewNode>},
	{"Menu",        CreateNode<SubmenuNode>},
	{"MenuBar",     CreateNode<MenuBarNode>},
	{"MenuField",   CreateNode<MenuFieldNode>},
	{"MenuItem",    CreateNode<MenuItemNode>},
	{"RadioButton", CreateNode<RadioButtonNode>},
	{"Scroll",      CreateNode<ScrollNode>},
	{"Separator",   CreateNode<SeparatorNode>},
	{"Slider",      CreateNode<SliderNode>},
	{"Split",       CreateNode<SplitNode>},
	{"StatusBar",   CreateNode<StatusBarNode>},
	{"StringView",  CreateNode<StringViewNode>},
	{"Strut",       CreateNode<StrutNode>},
	{"Tab",         CreateNode<TabNode>},
	{"Tabs",        CreateNode<TabsNode>},
	{"TextControl", CreateNode<TextControlNode>},
	{"TextView",    CreateNode<TextViewNode>},
	{"Window",      CreateNode<WindowNode>},
};


std::unique_ptr<Node> NodeFactory::Create(const std::string &type, uint32 id, const PropSet &props)
{
	const auto it = kCreators.find(type);
	if (it == kCreators.end()) {
		throw std::invalid_argument("unknown element type <" + type + ">");
	}
	return it->second(id, props);
}


} // HaikuReact
