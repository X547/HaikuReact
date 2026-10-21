#pragma once

#include <memory>
#include <string>
#include <vector>

#include <InterfaceDefs.h>
#include <Point.h>

#include <napi.h>

#include "../EventBridge.h"

class BLayoutItem;
class BLooper;
class BView;


namespace HaikuReact {


class PropSet;


// Properties a node exposes to the layout of its parent. Each container
// reads the ones that apply to it.
struct LayoutHints {
	float weight = 1.0f;
	int32 column = 0;
	int32 row = 0;
	int32 columnSpan = 1;
	int32 rowSpan = 1;
	bool collapsible = true;
};


// Native counterpart of a React host instance. Nodes form a tree that owns
// its children; parentless nodes are owned by the NodeRegistry. All methods
// run on the JS thread with the owning window locked (see LooperLock).
class Node {
private:
	const uint32 fId;
	Node *fParent = nullptr;
	std::vector<std::unique_ptr<Node>> fChildren;
	LayoutHints fLayoutHints;

	int32 IndexOf(const Node &child) const;
	int32 InsertionIndex(const Node *before) const;
	bool ApplyLayoutHints(const PropSet &props);

protected:
	// Native attach/detach of a child at a child-list index. Called before
	// the child list itself changes.
	virtual void AttachChild(Node &child, int32 index);
	virtual void DetachChild(Node &child);
	virtual void ChildLayoutHintsChanged(Node &child, int32 index);
	// Called after the child list changed.
	virtual void ChildListChanged();

	virtual void ApplyOwnProps(const PropSet &props) = 0;

	void NotifyChildrenOfOrientation(orientation direction);

public:
	explicit Node(uint32 id);
	virtual ~Node();

	Node(const Node &other) = delete;
	Node &operator=(const Node &other) = delete;

	uint32 Id() const {return fId;}
	Node *Parent() const {return fParent;}
	const std::vector<std::unique_ptr<Node>> &Children() const {return fChildren;}
	const LayoutHints &Hints() const {return fLayoutHints;}

	virtual const char *TypeName() const = 0;
	virtual BView *View() const {return nullptr;}
	virtual BLayoutItem *LayoutItem() const {return nullptr;}
	virtual BLooper *Looper() const;
	// Whether the view handles scrolling itself and becomes the target of an
	// enclosing <Scroll> directly (text and list views).
	virtual bool ScrollsItself() const {return false;}

	// The direction of the group or split layout the node is placed in.
	virtual void LayoutOrientationChanged(orientation direction);

	void ApplyProps(const PropSet &props);

	// On failure the child stays owned by the caller.
	void InsertChild(std::unique_ptr<Node> &&child, const Node *before);
	void MoveChild(Node &child, const Node *before);
	std::unique_ptr<Node> RemoveChild(Node &child);

	// Called once the node is attached to the visible tree for the first time.
	virtual void Mount();
	// Performs the primary user action (click, close, ...); used by tests.
	virtual void Invoke();
	// Drags with the mouse between two points in view coordinates; used by tests.
	virtual void Drag(BPoint from, BPoint to);
	// Changes the value the way a user would, so that the usual events are
	// sent; used by tests.
	virtual void SimulateInput(const EventValue &value);
	// Sends a key press to the window's focus, as the keyboard would; used
	// by tests.
	virtual void SimulateKey(const std::string &bytes, uint32 modifiers);
	virtual void Dump(Napi::Object &out) const;
};


} // HaikuReact
