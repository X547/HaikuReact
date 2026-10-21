#include "ReactApplication.h"

#include <atomic>
#include <memory>
#include <stdexcept>

#include <OS.h>

#include "AlertManager.h"
#include "EventBridge.h"
#include "StatusCheck.h"


namespace HaikuReact {


static std::string sSignature;
static sem_id sReadySem = -1;
static status_t sStartResult = B_OK;
static std::atomic<bool> sRunning {false};
static std::atomic<bool> sQuitAllowed {false};


static status_t AppThread(void *data)
{
	(void)data;

	status_t error = B_OK;
	auto app = std::make_unique<ReactApplication>(sSignature.c_str(), &error);
	if (error < B_OK) {
		sStartResult = error;
		release_sem(sReadySem);
		return error;
	}

	sRunning = true;
	app->Run();
	sRunning = false;
	app.reset();
	AlertManager::Clear();

	EventBridge::Post(Event{0, "quit", {}});
	EventBridge::Stop();
	return B_OK;
}


ReactApplication::ReactApplication(const char *signature, status_t *error):
	BApplication(signature, error)
{
}


void ReactApplication::ReadyToRun()
{
	release_sem(sReadySem);
}


bool ReactApplication::QuitRequested()
{
	if (sQuitAllowed) {
		return true;
	}
	EventBridge::Post(Event{0, "quitRequest", {}});
	return false;
}


void ReactApplication::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case AlertManager::kMsgAlertResult:
			AlertManager::HandleResult(*message);
			break;
		default:
			BApplication::MessageReceived(message);
			break;
	}
}


void ReactApplication::Start(const std::string &signature)
{
	if (sReadySem >= 0) {
		throw std::logic_error("the application can only be started once per process");
	}
	sSignature = signature;

	sReadySem = create_sem(0, "HaikuReact ready");
	CheckThrow(sReadySem);

	const thread_id thread = spawn_thread(AppThread, "HaikuReact app", B_NORMAL_PRIORITY, nullptr);
	CheckThrow(thread);
	CheckThrow(resume_thread(thread));

	while (acquire_sem(sReadySem) == B_INTERRUPTED) {
	}
	CheckThrow(sStartResult);
}


void ReactApplication::RequestQuit()
{
	if (!sRunning) {
		return;
	}
	sQuitAllowed = true;
	be_app->PostMessage(B_QUIT_REQUESTED);
}


bool ReactApplication::IsRunning()
{
	return sRunning;
}


void ReactApplication::PostQuitRequest()
{
	if (!sRunning) {
		return;
	}
	CheckThrow(be_app->PostMessage(B_QUIT_REQUESTED));
}


int32 ReactApplication::WindowCount()
{
	return sRunning ? be_app->CountWindows() : 0;
}


} // HaikuReact
