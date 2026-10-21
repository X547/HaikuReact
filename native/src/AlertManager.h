#pragma once

#include <string>
#include <vector>

#include <SupportDefs.h>

class BMessage;


namespace HaikuReact {


// Asynchronous BAlerts. The result goes to the application looper, which
// passes it on to JS as the application event "alert" with the value
// [alertId, buttonIndex].
class AlertManager {
public:
	static constexpr uint32 kMsgAlertResult = 'HRar';

	// type: "empty", "info", "idea", "warning" or "stop"; escapeButton is the
	// index of the button chosen by Escape, or -1.
	static uint32 Show(
		const std::string &title,
		const std::string &text,
		const std::vector<std::string> &buttons,
		const std::string &type,
		int32 escapeButton
	);

	// Called on the application thread.
	static void HandleResult(const BMessage &message);

	// Clicks a button like the user would; for tests.
	static void PressButton(uint32 alertId, int32 index);
	// Ids of the alerts waiting for an answer; for tests.
	static std::vector<uint32> OpenAlerts();

	// Releases the bookkeeping of alerts that never answered (application exit).
	static void Clear();
};


} // HaikuReact
