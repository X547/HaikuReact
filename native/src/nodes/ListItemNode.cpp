#include "ListItemNode.h"

#include <stdexcept>

#include "../BitmapCache.h"
#include "../PropSet.h"
#include "../ReactListItem.h"
#include "../ReactListView.h"


namespace HaikuReact {


static constexpr const char *kDefaultLoadingLabel = "Loading…";


namespace ListChildren {


int32 InsertionIndex(const ReactListView &list, const std::vector<std::unique_ptr<Node>> &children, int32 index,
	int32 endIndex)
{
	if (index < static_cast<int32>(children.size())) {
		return list.FullListIndexOf(static_cast<ListItemNode&>(*children[index]).Item());
	}
	return endIndex;
}


ListItemNode &AsItem(Node &parent, Node &child)
{
	auto *item = dynamic_cast<ListItemNode*>(&child);
	if (item == nullptr) {
		throw std::invalid_argument(std::string("<") + parent.TypeName() + "> can only contain <ListItem>, not <"
			+ child.TypeName() + ">");
	}
	return *item;
}


} // ListChildren


ListItemNode::ListItemNode(uint32 id):
	Node(id),
	fItem(std::make_unique<ReactListItem>(id)),
	fLoadingLabel(kDefaultLoadingLabel)
{
}


// The list view may be gone already; it never owns or deletes the items.
ListItemNode::~ListItemNode() = default;


bool ListItemNode::NeedsPlaceholder() const
{
	return fLoading || (fHasChildren && Children().empty());
}


bool ListItemNode::IsExpanded() const
{
	return fList != nullptr ? fItem->IsExpanded() : fExpanded;
}


void ListItemNode::RenewItems()
{
	auto item = std::make_unique<ReactListItem>(Id());
	item->SetItemId(fItem->ItemId());
	item->SetLabel(fLabel);
	item->SetIcon(fItem->Icon());
	fItem = std::move(item);
	fPlaceholder.reset();
}


void ListItemNode::UpdatePlaceholder()
{
	if (fList == nullptr) {
		return;
	}
	const bool needed = NeedsPlaceholder();
	if (needed && fPlaceholder == nullptr) {
		fPlaceholder = std::make_unique<ReactListItem>(0, true);
		fPlaceholder->SetLabel(fLoadingLabel);
		fPlaceholder->SetOutlineLevel(fItem->OutlineLevel() + 1);
		fList->AddItem(fPlaceholder.get(), fList->FullListIndexOf(fItem.get()) + 1);
	} else if (!needed && fPlaceholder != nullptr) {
		fList->RemoveItem(fPlaceholder.get());
		fPlaceholder.reset();
		InvalidateLatch();
	} else if (fPlaceholder != nullptr && fPlaceholder->Label() != fLoadingLabel) {
		fPlaceholder->SetLabel(fLoadingLabel);
		fList->ItemsChanged();
	}
}


void ListItemNode::SetExpanded(bool expanded)
{
	if (fList == nullptr) {
		fExpanded = expanded;
		return;
	}
	if (fItem->IsExpanded() == expanded) {
		return;
	}
	if (expanded) {
		fList->Expand(fItem.get());
	} else {
		fList->Collapse(fItem.get());
	}
}


int32 ListItemNode::AddToList(ReactListView &list, int32 fullIndex, uint32 level)
{
	fList = &list;
	fItem->SetOutlineLevel(level);
	fItem->SetExpanded(fExpanded);
	list.AddItem(fItem.get(), fullIndex++);
	if (NeedsPlaceholder()) {
		fPlaceholder = std::make_unique<ReactListItem>(0, true);
		fPlaceholder->SetLabel(fLoadingLabel);
		fPlaceholder->SetOutlineLevel(level + 1);
		list.AddItem(fPlaceholder.get(), fullIndex++);
	}
	for (const std::unique_ptr<Node> &child : Children()) {
		fullIndex = static_cast<ListItemNode&>(*child).AddToList(list, fullIndex, level + 1);
	}
	return fullIndex;
}


int32 ListItemNode::EndIndex() const
{
	return fList->FullListIndexOf(fItem.get()) + 1 + fList->CountItemsUnder(fItem.get(), false);
}


void ListItemNode::RemoveFromList()
{
	if (fList == nullptr) {
		return;
	}
	// Last item first: BOutlineListView deletes the visible descendants of a
	// removed item, but the nodes own them.
	const int32 start = fList->FullListIndexOf(fItem.get());
	for (int32 i = EndIndex() - 1; i >= start; i--) {
		fList->RemoveItem(i);
	}

	const auto forget = [](auto &self, ListItemNode &node) -> void {
		node.fExpanded = node.fItem->IsExpanded();
		node.fList = nullptr;
		node.RenewItems();
		for (const std::unique_ptr<Node> &child : node.Children()) {
			self(self, static_cast<ListItemNode&>(*child));
		}
	};
	forget(forget, *this);
}


void ListItemNode::AttachChild(Node &child, int32 index)
{
	ListItemNode &item = ListChildren::AsItem(*this, child);
	if (fList != nullptr) {
		const int32 fullIndex = ListChildren::InsertionIndex(*fList, Children(), index, EndIndex());
		item.AddToList(*fList, fullIndex, fItem->OutlineLevel() + 1);
	}
}


void ListItemNode::DetachChild(Node &child)
{
	static_cast<ListItemNode&>(child).RemoveFromList();
}


void ListItemNode::InvalidateLatch()
{
	// BOutlineListView redraws the expand arrow when an item gets its first
	// subitem, but not when it loses its last one.
	const int32 index = fList->IndexOf(fItem.get());
	if (index >= 0) {
		fList->InvalidateItem(index);
	}
}


void ListItemNode::ChildListChanged()
{
	UpdatePlaceholder();
	if (fList != nullptr) {
		InvalidateLatch();
		fList->RestoreSelection();
	}
}


void ListItemNode::ApplyLabelAndIcon()
{
	fItem->SetLabel(fLabel);
	fItem->SetIcon(BitmapCache::Get(fIconSource, fItem->IconSize()));
	if (fList != nullptr) {
		fList->ItemsChanged();
	}
}


void ListItemNode::ApplyOwnProps(const PropSet &props)
{
	if (auto id = props.Get<std::string>("id", "")) {
		fItem->SetItemId(*id);
		if (fList != nullptr) {
			fList->RestoreSelection();
		}
	}

	bool appearanceChanged = false;
	if (auto label = props.Get<std::string>("label", ""); label && *label != fLabel) {
		fLabel = *label;
		appearanceChanged = true;
	}
	if (auto icon = props.Get<IconSource>("icon", {}); icon && !(*icon == fIconSource)) {
		fIconSource = *icon;
		appearanceChanged = true;
	}
	if (appearanceChanged) {
		ApplyLabelAndIcon();
	}

	bool placeholderChanged = false;
	if (auto hasChildren = props.Get<bool>("hasChildren", false)) {
		fHasChildren = *hasChildren;
		placeholderChanged = true;
	}
	if (auto loading = props.Get<bool>("loading", false)) {
		fLoading = *loading;
		placeholderChanged = true;
	}
	if (auto loadingLabel = props.Get<std::string>("loadingLabel", kDefaultLoadingLabel)) {
		fLoadingLabel = *loadingLabel;
		placeholderChanged = true;
	}
	if (placeholderChanged) {
		UpdatePlaceholder();
	}

	if (auto expanded = props.Get<bool>("expanded", false)) {
		SetExpanded(*expanded);
	}
}


void ListItemNode::SimulateInput(const EventValue &value)
{
	const bool *expand = std::get_if<bool>(&value);
	if (expand == nullptr || fList == nullptr) {
		throw std::invalid_argument("<ListItem> input must be a boolean (expand), and the item must be in a list");
	}
	fList->PostExpand(fItem.get(), *expand);
}


void ListItemNode::Dump(Napi::Object &out) const
{
	Node::Dump(out);
	Napi::Env env = out.Env();
	out.Set("itemId", fItem->ItemId());
	out.Set("label", fLabel);
	out.Set("hasIcon", fItem->Icon() != nullptr);
	out.Set("expanded", IsExpanded());
	out.Set("placeholder", fPlaceholder != nullptr ? Napi::Value(Napi::String::New(env, fPlaceholder->Label()))
		: env.Null());
	out.Set("visible", fList != nullptr && fList->HasItem(fItem.get()));
	if (fList != nullptr) {
		out.Set("level", fItem->OutlineLevel());
	}
}


} // HaikuReact
