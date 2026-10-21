#pragma once

#include <vector>

#include <ListView.h>


namespace HaikuReact {


// List view backing <ListView>. Reports selection changes made by the user.
class ReactListView: public BListView {
private:
	const uint32 fNodeId;

public:
	explicit ReactListView(uint32 nodeId);

	std::vector<int32> Selection() const;
	void SetSelection(const std::vector<int32> &selection);

	void SelectionChanged() final;
	void MessageReceived(BMessage *message) final;

	// Selects the items on the window thread, like a user would; for tests.
	void PostSelect(const std::vector<int32> &selection);
};


} // HaikuReact
