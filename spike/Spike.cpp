// Phase 0 spike: validates the threading model of the HaikuReact add-on.
//
// - BApplication constructed and run on a thread spawned by the add-on, while
//   the Node main thread keeps running libuv.
// - BWindow constructed on the Node thread, started hidden with Run(), then
//   mutated from the Node thread under the looper lock.
// - Window/application threads report events to JS through a non-blocking
//   napi_threadsafe_function and never wait for JS.
// - Node exits by itself once the application thread is gone.
// - std::system_error thrown in native code arrives in JS as an Error.

#include <stdio.h>

#include <atomic>
#include <memory>
#include <string>
#include <system_error>

#include <Application.h>
#include <Autolock.h>
#include <Button.h>
#include <LayoutBuilder.h>
#include <OS.h>
#include <Window.h>

#include <napi.h>


constexpr uint32 kMsgClick = 'clik';

struct SpikeEvent {
	std::string kind;
	int32 value {};
};

class SpikeWindow;

static Napi::ThreadSafeFunction sEventFn;
static sem_id sReadySem = -1;
static status_t sStartResult = B_OK;
static std::atomic<bool> sQuitAllowed {false};
static SpikeWindow *sWindow = nullptr;


static void CheckThrow(status_t res)
{
	if (res < B_OK) {
		throw std::system_error(res, std::generic_category());
	}
}


static void CallJs(Napi::Env env, Napi::Function callback, SpikeEvent *event)
{
	std::unique_ptr<SpikeEvent> eventDeleter(event);
	if (env == nullptr || callback.IsEmpty()) {
		return;
	}
	try {
		callback.Call({
			Napi::String::New(env, event->kind),
			Napi::Number::New(env, event->value)
		});
	} catch (const Napi::Error &e) {
		e.ThrowAsJavaScriptException();
	}
}


// Callable from any thread; never blocks.
static void PostEvent(const char *kind, int32 value)
{
	auto event = std::make_unique<SpikeEvent>(SpikeEvent{kind, value});
	const napi_status status = sEventFn.NonBlockingCall(event.get(), CallJs);
	if (status != napi_ok) {
		fprintf(stderr, "HaikuReact: failed to post event \"%s\": %d\n", kind, status);
		return;
	}
	event.release();
}


//#pragma mark - SpikeApplication

class SpikeApplication: public BApplication {
public:
	SpikeApplication(const char *signature, status_t *error);

	void ReadyToRun() final;
	bool QuitRequested() final;
};


SpikeApplication::SpikeApplication(const char *signature, status_t *error):
	BApplication(signature, error)
{
}


void SpikeApplication::ReadyToRun()
{
	release_sem(sReadySem);
}


bool SpikeApplication::QuitRequested()
{
	if (sQuitAllowed) {
		return true;
	}
	PostEvent("appQuitRequest", 0);
	return false;
}


static status_t AppThread(void *data)
{
	const auto *signature = static_cast<const std::string*>(data);

	status_t error = B_OK;
	auto app = std::make_unique<SpikeApplication>(signature->c_str(), &error);
	if (error < B_OK) {
		sStartResult = error;
		release_sem(sReadySem);
		return error;
	}

	PostEvent("appThread", find_thread(nullptr));
	app->Run();
	app.reset();

	PostEvent("appQuit", 0);
	sEventFn.Release();
	return B_OK;
}


//#pragma mark - SpikeWindow

class SpikeWindow: public BWindow {
private:
	BButton *fButton;
	int32 fClicks {};

public:
	explicit SpikeWindow(const char *title);

	BButton *Button() const {return fButton;}

	void MessageReceived(BMessage *message) final;
	bool QuitRequested() final;
};


SpikeWindow::SpikeWindow(const char *title):
	BWindow(
		BRect(100, 100, 300, 200),
		title,
		B_TITLED_WINDOW,
		B_AUTO_UPDATE_SIZE_LIMITS | B_ASYNCHRONOUS_CONTROLS
	),
	fButton(new BButton("button", "Click me", new BMessage(kMsgClick)))
{
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_SPACING)
		.Add(fButton)
		.AddGlue();
}


void SpikeWindow::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case kMsgClick:
			PostEvent("click", ++fClicks);
			break;
		default:
			BWindow::MessageReceived(message);
			break;
	}
}


bool SpikeWindow::QuitRequested()
{
	PostEvent("closeRequest", find_thread(nullptr));
	return false;
}


//#pragma mark - JS bindings

static SpikeWindow &CurrentWindow()
{
	if (sWindow == nullptr) {
		throw std::system_error(B_BAD_VALUE, std::generic_category(), "no window");
	}
	return *sWindow;
}


static void Start(const Napi::CallbackInfo &info)
{
	static std::string signature;
	signature = info[0].As<Napi::String>().Utf8Value();

	sEventFn = Napi::ThreadSafeFunction::New(
		info.Env(),
		info[1].As<Napi::Function>(),
		"HaikuReactEvents",
		0,
		1
	);

	try {
		sReadySem = create_sem(0, "HaikuReact ready");
		CheckThrow(sReadySem);

		const thread_id thread = spawn_thread(AppThread, "HaikuReact app", B_NORMAL_PRIORITY, &signature);
		CheckThrow(thread);
		CheckThrow(resume_thread(thread));

		while (acquire_sem(sReadySem) == B_INTERRUPTED) {
		}
		CheckThrow(sStartResult);
	} catch (...) {
		sEventFn.Release();
		throw;
	}
}


static void Quit(const Napi::CallbackInfo &info)
{
	(void)info;
	sQuitAllowed = true;
	be_app->PostMessage(B_QUIT_REQUESTED);
}


static Napi::Value Threads(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();
	Napi::Object result = Napi::Object::New(env);
	result.Set("js", find_thread(nullptr));
	result.Set("app", be_app->Thread());
	if (sWindow != nullptr) {
		result.Set("window", sWindow->Thread());
	}
	return result;
}


static void CreateWindow(const Napi::CallbackInfo &info)
{
	const std::string title = info[0].As<Napi::String>().Utf8Value();
	sWindow = new SpikeWindow(title.c_str());
	// Constructed locked by this thread; Run() unlocks and starts the window
	// thread while the window stays hidden.
	CheckThrow(sWindow->Run());
}


static void SetButtonLabel(const Napi::CallbackInfo &info)
{
	const std::string label = info[0].As<Napi::String>().Utf8Value();
	SpikeWindow &window = CurrentWindow();
	BAutolock lock(&window);
	window.Button()->SetLabel(label.c_str());
}


static void Show(const Napi::CallbackInfo &info)
{
	(void)info;
	SpikeWindow &window = CurrentWindow();
	BAutolock lock(&window);
	window.Show();
}


static void ClickButton(const Napi::CallbackInfo &info)
{
	(void)info;
	SpikeWindow &window = CurrentWindow();
	BAutolock lock(&window);
	CheckThrow(window.Button()->Invoke());
}


static void RequestClose(const Napi::CallbackInfo &info)
{
	(void)info;
	CheckThrow(CurrentWindow().PostMessage(B_QUIT_REQUESTED));
}


static void DestroyWindow(const Napi::CallbackInfo &info)
{
	(void)info;
	SpikeWindow &window = CurrentWindow();
	window.Lock();
	window.Quit();
	sWindow = nullptr;
}


static Napi::Object RectToJs(Napi::Env env, const BRect &rect)
{
	Napi::Object result = Napi::Object::New(env);
	result.Set("left", rect.left);
	result.Set("top", rect.top);
	result.Set("width", rect.Width() + 1);
	result.Set("height", rect.Height() + 1);
	return result;
}


static Napi::Value DumpLayout(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();
	SpikeWindow &window = CurrentWindow();
	BAutolock lock(&window);

	Napi::Object result = Napi::Object::New(env);
	result.Set("window", RectToJs(env, window.Bounds()));
	result.Set("button", RectToJs(env, window.Button()->Frame()));
	result.Set("label", window.Button()->Label());
	return result;
}


static void FailTest(const Napi::CallbackInfo &info)
{
	(void)info;
	CheckThrow(B_NO_MEMORY);
}


static Napi::Object Init(Napi::Env env, Napi::Object exports)
{
	exports.Set("start", Napi::Function::New(env, Start));
	exports.Set("quit", Napi::Function::New(env, Quit));
	exports.Set("threads", Napi::Function::New(env, Threads));
	exports.Set("createWindow", Napi::Function::New(env, CreateWindow));
	exports.Set("setButtonLabel", Napi::Function::New(env, SetButtonLabel));
	exports.Set("show", Napi::Function::New(env, Show));
	exports.Set("clickButton", Napi::Function::New(env, ClickButton));
	exports.Set("requestClose", Napi::Function::New(env, RequestClose));
	exports.Set("destroyWindow", Napi::Function::New(env, DestroyWindow));
	exports.Set("dumpLayout", Napi::Function::New(env, DumpLayout));
	exports.Set("failTest", Napi::Function::New(env, FailTest));
	return exports;
}


NODE_API_MODULE(spike, Init)
