#include "AlertManager.h"

#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>

#include <Alert.h>
#include <Application.h>
#include <Invoker.h>
#include <Messenger.h>

#include "EventBridge.h"
#include "StatusCheck.h"


namespace HaikuReact {


// Message BAlert's buttons send to the alert (private to BAlert).
constexpr uint32 kMsgAlertButton = 'ALTB';
constexpr const char *kFieldAlert = "hr:alert";
constexpr const char *kFieldWhich = "which";


struct PendingAlert {
	BMessenger alert;
	// BAlert::Go() does not take ownership of the invoker.
	std::unique_ptr<BInvoker> invoker;
};

// Accessed from the JS thread (Show, PressButton) and the application
// thread (HandleResult, Clear).
static std::mutex sLock;
static std::map<uint32, PendingAlert> sAlerts;
static uint32 sLastId = 0;


static alert_type ParseAlertType(const std::string &type)
{
	if (type == "empty") {
		return B_EMPTY_ALERT;
	}
	if (type == "info") {
		return B_INFO_ALERT;
	}
	if (type == "idea") {
		return B_IDEA_ALERT;
	}
	if (type == "warning") {
		return B_WARNING_ALERT;
	}
	if (type == "stop") {
		return B_STOP_ALERT;
	}
	throw std::invalid_argument("alert type: expected \"empty\", \"info\", \"idea\", \"warning\" or \"stop\"");
}


uint32 AlertManager::Show(
	const std::string &title,
	const std::string &text,
	const std::vector<std::string> &buttons,
	const std::string &type,
	int32 escapeButton
)
{
	if (buttons.empty() || buttons.size() > 3) {
		throw std::invalid_argument("an alert has 1 to 3 buttons");
	}
	const auto button = [&buttons](size_t index) {
		return index < buttons.size() ? buttons[index].c_str() : nullptr;
	};

	auto *alert = new BAlert(
		title.c_str(),
		text.c_str(),
		button(0), button(1), button(2),
		B_WIDTH_AS_USUAL,
		ParseAlertType(type)
	);
	if (escapeButton >= 0 && escapeButton < static_cast<int32>(buttons.size())) {
		alert->SetShortcut(escapeButton, B_ESCAPE);
	}

	std::lock_guard<std::mutex> lock(sLock);
	const uint32 id = ++sLastId;
	auto *result = new BMessage(kMsgAlertResult);
	result->AddUInt32(kFieldAlert, id);
	PendingAlert &pending = sAlerts[id];
	pending.alert = BMessenger(alert);
	pending.invoker = std::make_unique<BInvoker>(result, be_app);
	CheckThrow(alert->Go(pending.invoker.get()));
	return id;
}


void AlertManager::HandleResult(const BMessage &message)
{
	const uint32 id = message.GetUInt32(kFieldAlert, 0);
	const int32 which = message.GetInt32(kFieldWhich, -1);
	{
		std::lock_guard<std::mutex> lock(sLock);
		sAlerts.erase(id);
	}
	EventBridge::Post(Event{0, "alert", std::vector<int32>{static_cast<int32>(id), which}});
}


void AlertManager::PressButton(uint32 alertId, int32 index)
{
	BMessenger alert;
	{
		std::lock_guard<std::mutex> lock(sLock);
		const auto it = sAlerts.find(alertId);
		if (it == sAlerts.end()) {
			throw std::invalid_argument("no open alert with id " + std::to_string(alertId));
		}
		alert = it->second.alert;
	}
	BMessage message(kMsgAlertButton);
	message.AddInt32(kFieldWhich, index);
	CheckThrow(alert.SendMessage(&message));
}


std::vector<uint32> AlertManager::OpenAlerts()
{
	std::lock_guard<std::mutex> lock(sLock);
	std::vector<uint32> ids;
	for (const auto &[id, alert] : sAlerts) {
		ids.push_back(id);
	}
	return ids;
}


void AlertManager::Clear()
{
	std::lock_guard<std::mutex> lock(sLock);
	sAlerts.clear();
}


} // HaikuReact
