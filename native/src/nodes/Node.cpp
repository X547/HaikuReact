#include "Node.h"

#include <optional>
#include <stdexcept>
#include <string>

#include "../NodeRegistry.h"
#include "../PropSet.h"


namespace HaikuReact {


template<typename T>
static bool UpdateHint(const PropSet &props, const char *key, T &value, const T &defaultValue)
{
	const std::optional<T> newValue = props.Get<T>(key, defaultValue);
	if (!newValue.has_value() || *newValue == value) {
		return false;
	}
	value = *newValue;
	return true;
}


Node::Node(uint32 id):
	fId(id)
{
}


Node::~Node()
{
	NodeRegistry::Forget(fId);
}


int32 Node::IndexOf(const Node &child) const
{
	for (size_t i = 0; i < fChildren.size(); i++) {
		if (fChildren[i].get() == &child) {
			return static_cast<int32>(i);
		}
	}
	return -1;
}


int32 Node::InsertionIndex(const Node *before) const
{
	if (before == nullptr) {
		return static_cast<int32>(fChildren.size());
	}
	const int32 index = IndexOf(*before);
	if (index < 0) {
		throw std::invalid_argument("insertBefore: reference node is not a child of the parent");
	}
	return index;
}


void Node::AttachChild(Node &child, int32 index)
{
	(void)index;
	throw std::invalid_argument(std::string("<") + TypeName() + "> cannot contain <" + child.TypeName() + ">");
}


void Node::DetachChild(Node &child)
{
	(void)child;
}


void Node::ChildLayoutHintsChanged(Node &child, int32 index)
{
	(void)child;
	(void)index;
}


void Node::ChildListChanged()
{
}


void Node::NotifyChildrenOfOrientation(orientation direction)
{
	for (const std::unique_ptr<Node> &child : fChildren) {
		child->LayoutOrientationChanged(direction);
	}
}


void Node::LayoutOrientationChanged(orientation direction)
{
	(void)direction;
}


BLooper *Node::Looper() const
{
	if (fParent == nullptr) {
		return nullptr;
	}
	return fParent->Looper();
}


bool Node::ApplyLayoutHints(const PropSet &props)
{
	const LayoutHints defaults;
	bool changed = false;
	changed |= UpdateHint(props, "weight", fLayoutHints.weight, defaults.weight);
	changed |= UpdateHint(props, "column", fLayoutHints.column, defaults.column);
	changed |= UpdateHint(props, "row", fLayoutHints.row, defaults.row);
	changed |= UpdateHint(props, "columnSpan", fLayoutHints.columnSpan, defaults.columnSpan);
	changed |= UpdateHint(props, "rowSpan", fLayoutHints.rowSpan, defaults.rowSpan);
	changed |= UpdateHint(props, "collapsible", fLayoutHints.collapsible, defaults.collapsible);
	return changed;
}


void Node::ApplyProps(const PropSet &props)
{
	ApplyOwnProps(props);
	if (ApplyLayoutHints(props) && fParent != nullptr) {
		fParent->ChildLayoutHintsChanged(*this, fParent->IndexOf(*this));
	}
}


void Node::InsertChild(std::unique_ptr<Node> &&child, const Node *before)
{
	const int32 index = InsertionIndex(before);
	AttachChild(*child, index);
	child->fParent = this;
	fChildren.insert(fChildren.begin() + index, std::move(child));
	ChildListChanged();
}


void Node::MoveChild(Node &child, const Node *before)
{
	const int32 from = IndexOf(child);
	if (from < 0) {
		throw std::invalid_argument("insertBefore: node is not a child of the parent");
	}
	if (&child == before) {
		return;
	}

	std::unique_ptr<Node> owned = std::move(fChildren[from]);
	fChildren.erase(fChildren.begin() + from);
	DetachChild(*owned);

	try {
		const int32 index = InsertionIndex(before);
		AttachChild(*owned, index);
		fChildren.insert(fChildren.begin() + index, std::move(owned));
	} catch (...) {
		owned->fParent = nullptr;
		NodeRegistry::AddRoot(std::move(owned));
		ChildListChanged();
		throw;
	}
	ChildListChanged();
}


std::unique_ptr<Node> Node::RemoveChild(Node &child)
{
	const int32 index = IndexOf(child);
	if (index < 0) {
		throw std::invalid_argument("removeChild: node is not a child of the parent");
	}
	DetachChild(child);
	std::unique_ptr<Node> owned = std::move(fChildren[index]);
	fChildren.erase(fChildren.begin() + index);
	owned->fParent = nullptr;
	ChildListChanged();
	return owned;
}


void Node::Mount()
{
}


void Node::Invoke()
{
	throw std::invalid_argument(std::string("<") + TypeName() + "> has no action to invoke");
}


void Node::Drag(BPoint from, BPoint to)
{
	(void)from;
	(void)to;
	throw std::invalid_argument(std::string("<") + TypeName() + "> does not support dragging");
}


void Node::SimulateInput(const EventValue &value)
{
	(void)value;
	throw std::invalid_argument(std::string("<") + TypeName() + "> does not take input");
}


void Node::SimulateKey(const std::string &bytes, uint32 modifiers)
{
	(void)bytes;
	(void)modifiers;
	throw std::invalid_argument(std::string("<") + TypeName() + "> does not take key presses");
}


void Node::Dump(Napi::Object &out) const
{
	Napi::Env env = out.Env();
	out.Set("id", fId);
	out.Set("type", TypeName());

	Napi::Array children = Napi::Array::New(env, fChildren.size());
	for (uint32 i = 0; i < fChildren.size(); i++) {
		Napi::Object childOut = Napi::Object::New(env);
		fChildren[i]->Dump(childOut);
		children.Set(i, childOut);
	}
	out.Set("children", children);
}


} // HaikuReact
