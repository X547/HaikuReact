#include "ViewNode.h"

#include <View.h>

#include "../PropSet.h"


namespace HaikuReact {


static bool UpdateSize(const PropSet &props, const char *widthKey, const char *heightKey, BSize &size)
{
	bool changed = false;
	if (auto width = props.Get<SizeValue>(widthKey, SizeValue{B_SIZE_UNSET})) {
		size.width = width->value;
		changed = true;
	}
	if (auto height = props.Get<SizeValue>(heightKey, SizeValue{B_SIZE_UNSET})) {
		size.height = height->value;
		changed = true;
	}
	return changed;
}


ViewNode::ViewNode(uint32 id, BView *view, BSize defaultMaxSize):
	Node(id),
	fView(view),
	fDefaultMaxSize(defaultMaxSize)
{
	fView->SetExplicitMaxSize(fDefaultMaxSize);
}


ViewNode::~ViewNode()
{
	if (Parent() == nullptr) {
		delete fView;
	}
}


void ViewNode::SetHidden(bool hidden)
{
	if (hidden == fHidden) {
		return;
	}
	// BView::Hide()/Show() nest, so only call them on an actual change.
	fHidden = hidden;
	if (hidden) {
		fView->Hide();
	} else {
		fView->Show();
	}
}


void ViewNode::FitMaxSizeToMinSize()
{
	// Layouts cap a view at its maximum even below an explicit minimum (a
	// BButton's maximum is its preferred size), so raise the maximum along
	// with the minimum, unless the maximum is explicit too.
	const BSize minSize = fView->ExplicitMinSize();
	BSize maxSize = fView->ExplicitMaxSize();
	if (!fHasMaxWidth) {
		maxSize.width = fDefaultMaxSize.width;
	}
	if (!fHasMaxHeight) {
		maxSize.height = fDefaultMaxSize.height;
	}
	fView->SetExplicitMaxSize(maxSize);

	const BSize ownMaxSize = fView->MaxSize();
	if (!fHasMaxWidth && minSize.width != B_SIZE_UNSET && minSize.width > ownMaxSize.width) {
		maxSize.width = minSize.width;
	}
	if (!fHasMaxHeight && minSize.height != B_SIZE_UNSET && minSize.height > ownMaxSize.height) {
		maxSize.height = minSize.height;
	}
	fView->SetExplicitMaxSize(maxSize);
}


void ViewNode::ApplyExplicitLayout(const PropSet &props)
{
	bool limitsChanged = false;
	BSize minSize = fView->ExplicitMinSize();
	if (UpdateSize(props, "minWidth", "minHeight", minSize)) {
		fView->SetExplicitMinSize(minSize);
		limitsChanged = true;
	}
	BSize maxSize = fView->ExplicitMaxSize();
	if (UpdateSize(props, "maxWidth", "maxHeight", maxSize)) {
		fView->SetExplicitMaxSize(maxSize);
		fHasMaxWidth = maxSize.width != B_SIZE_UNSET;
		fHasMaxHeight = maxSize.height != B_SIZE_UNSET;
		limitsChanged = true;
	}
	if (limitsChanged) {
		FitMaxSizeToMinSize();
	}
	BSize preferredSize = fView->ExplicitPreferredSize();
	if (UpdateSize(props, "preferredWidth", "preferredHeight", preferredSize)) {
		fView->SetExplicitPreferredSize(preferredSize);
	}

	BAlignment explicitAlignment = fView->ExplicitAlignment();
	bool alignmentChanged = false;
	if (auto value = props.Get<alignment>("horizontalAlignment", B_ALIGN_HORIZONTAL_UNSET)) {
		explicitAlignment.horizontal = *value;
		alignmentChanged = true;
	}
	if (auto value = props.Get<vertical_alignment>("verticalAlignment", B_ALIGN_VERTICAL_UNSET)) {
		explicitAlignment.vertical = *value;
		alignmentChanged = true;
	}
	if (alignmentChanged) {
		fView->SetExplicitAlignment(explicitAlignment);
	}
}


void ViewNode::ApplyOwnProps(const PropSet &props)
{
	if (auto hidden = props.Get<bool>("hidden", false)) {
		SetHidden(*hidden);
	}
	ApplyExplicitLayout(props);
}


void ViewNode::Dump(Napi::Object &out) const
{
	Node::Dump(out);

	Napi::Env env = out.Env();
	const BRect frame = fView->Frame();
	Napi::Object frameOut = Napi::Object::New(env);
	frameOut.Set("x", frame.left);
	frameOut.Set("y", frame.top);
	frameOut.Set("width", frame.Width() + 1);
	frameOut.Set("height", frame.Height() + 1);
	out.Set("frame", frameOut);
	// The view's own state: set by the hidden prop or by its layout (cards).
	out.Set("hidden", fView->IsHidden(fView));

	// Layout constraints in BSize units (pixels - 1), for diagnostics.
	const auto sizeToJs = [env](BSize size) {
		Napi::Array result = Napi::Array::New(env, 2);
		result.Set(0u, size.width);
		result.Set(1u, size.height);
		return result;
	};
	Napi::Object layoutOut = Napi::Object::New(env);
	layoutOut.Set("min", sizeToJs(fView->MinSize()));
	layoutOut.Set("max", sizeToJs(fView->MaxSize()));
	layoutOut.Set("preferred", sizeToJs(fView->PreferredSize()));
	out.Set("layout", layoutOut);
}


} // HaikuReact
