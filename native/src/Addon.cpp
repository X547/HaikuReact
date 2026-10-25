// Node-API entry point. Every function runs on the JS thread; node ids are
// the only handles that cross into JS.

#include <memory>
#include <stdexcept>
#include <string>

#include <napi.h>

#include "AlertManager.h"
#include "BitmapCache.h"
#include "CommitScope.h"
#include "EventBridge.h"
#include "KeyModifiers.h"
#include "LooperLock.h"
#include "NodeFactory.h"
#include "NodeRegistry.h"
#include "PropSet.h"
#include "ReactApplication.h"
#include "nodes/Node.h"


namespace HaikuReact {


static uint32 IdArg(const Napi::CallbackInfo &info, size_t index)
{
	const Napi::Value value = info[index];
	if (!value.IsNumber()) {
		throw Napi::TypeError::New(info.Env(), "expected a node id");
	}
	return value.As<Napi::Number>().Uint32Value();
}


static std::string StringArg(const Napi::CallbackInfo &info, size_t index)
{
	const Napi::Value value = info[index];
	if (!value.IsString()) {
		throw Napi::TypeError::New(info.Env(), "expected a string");
	}
	return value.As<Napi::String>().Utf8Value();
}


static Napi::Object ObjectArg(const Napi::CallbackInfo &info, size_t index)
{
	const Napi::Value value = info[index];
	if (!value.IsObject()) {
		throw Napi::TypeError::New(info.Env(), "expected an object");
	}
	return value.As<Napi::Object>();
}


static Napi::Function FunctionArg(const Napi::CallbackInfo &info, size_t index)
{
	const Napi::Value value = info[index];
	if (!value.IsFunction()) {
		throw Napi::TypeError::New(info.Env(), "expected a function");
	}
	return value.As<Napi::Function>();
}


//#pragma mark - Application

static void Start(const Napi::CallbackInfo &info)
{
	const std::string signature = StringArg(info, 0);
	EventBridge::Start(info.Env(), FunctionArg(info, 1));
	try {
		ReactApplication::Start(signature);
	} catch (...) {
		EventBridge::Stop();
		throw;
	}
}


static void Quit(const Napi::CallbackInfo &info)
{
	(void)info;
	NodeRegistry::DestroyAll();
	ReactApplication::RequestQuit();
}


// showAlert(title, text, buttons, type, escapeButton): alert id; the result
// arrives as the application event "alert" with [id, buttonIndex].
static Napi::Value ShowAlert(const Napi::CallbackInfo &info)
{
	const std::string title = StringArg(info, 0);
	const std::string text = StringArg(info, 1);
	std::vector<std::string> buttons;
	if (!info[2].IsArray()) {
		throw Napi::TypeError::New(info.Env(), "expected an array of button labels");
	}
	const Napi::Array array = info[2].As<Napi::Array>();
	for (uint32 i = 0; i < array.Length(); i++) {
		buttons.push_back(array.Get(i).ToString().Utf8Value());
	}
	const std::string type = StringArg(info, 3);
	const int32 escapeButton = info[4].IsNumber() ? info[4].As<Napi::Number>().Int32Value() : -1;

	return Napi::Number::New(info.Env(), AlertManager::Show(title, text, buttons, type, escapeButton));
}


//#pragma mark - Nodes

static Napi::Value CreateNode(const Napi::CallbackInfo &info)
{
	if (!ReactApplication::IsRunning()) {
		throw std::logic_error("the application is not running");
	}
	const std::string type = StringArg(info, 0);
	const PropSet props(ObjectArg(info, 1));

	std::unique_ptr<Node> node = NodeFactory::Create(type, NodeRegistry::NextId(), props);
	{
		LooperLock lock(node->Looper());
		node->ApplyProps(props);
	}
	const uint32 id = node->Id();
	NodeRegistry::AddRoot(std::move(node));
	return Napi::Number::New(info.Env(), id);
}


static void UpdateNode(const Napi::CallbackInfo &info)
{
	Node &node = NodeRegistry::Get(IdArg(info, 0));
	const PropSet props(ObjectArg(info, 1));

	LooperLock lock(node.Looper());
	node.ApplyProps(props);
}


// insertBefore(parent, child, before): before == 0 appends. Moves the child
// if it already belongs to the parent.
static void InsertBefore(const Napi::CallbackInfo &info)
{
	Node &parent = NodeRegistry::Get(IdArg(info, 0));
	Node &child = NodeRegistry::Get(IdArg(info, 1));
	const uint32 beforeId = IdArg(info, 2);
	const Node *before = beforeId == 0 ? nullptr : &NodeRegistry::Get(beforeId);

	LooperLock lock(parent.Looper());
	if (child.Parent() == &parent) {
		parent.MoveChild(child, before);
		return;
	}
	if (child.Parent() != nullptr) {
		throw std::logic_error("node is attached to a different parent");
	}

	std::unique_ptr<Node> owned = NodeRegistry::TakeRoot(child.Id());
	try {
		parent.InsertChild(std::move(owned), before);
	} catch (...) {
		if (owned != nullptr) {
			NodeRegistry::AddRoot(std::move(owned));
		}
		throw;
	}
}


// Removes and destroys the child subtree.
static void RemoveChild(const Napi::CallbackInfo &info)
{
	Node &parent = NodeRegistry::Get(IdArg(info, 0));
	Node &child = NodeRegistry::Get(IdArg(info, 1));

	LooperLock lock(parent.Looper());
	parent.RemoveChild(child).reset();
}


static void DestroyRoot(const Napi::CallbackInfo &info)
{
	NodeRegistry::DestroyRoot(IdArg(info, 0));
}


static void CommitMount(const Napi::CallbackInfo &info)
{
	Node &node = NodeRegistry::Get(IdArg(info, 0));

	LooperLock lock(node.Looper());
	node.Mount();
}


static void BeginCommit(const Napi::CallbackInfo &info)
{
	(void)info;
	CommitScope::Begin();
}


static void EndCommit(const Napi::CallbackInfo &info)
{
	(void)info;
	CommitScope::End();
}


//#pragma mark - Testing

static void Invoke(const Napi::CallbackInfo &info)
{
	Node &node = NodeRegistry::Get(IdArg(info, 0));

	LooperLock lock(node.Looper());
	node.Invoke();
}


static float NumberArg(const Napi::CallbackInfo &info, size_t index)
{
	const Napi::Value value = info[index];
	if (!value.IsNumber()) {
		throw Napi::TypeError::New(info.Env(), "expected a number");
	}
	return value.As<Napi::Number>().FloatValue();
}


static void Drag(const Napi::CallbackInfo &info)
{
	Node &node = NodeRegistry::Get(IdArg(info, 0));
	const BPoint from(NumberArg(info, 1), NumberArg(info, 2));
	const BPoint to(NumberArg(info, 3), NumberArg(info, 4));

	LooperLock lock(node.Looper());
	node.Drag(from, to);
}


static EventValue ValueArg(const Napi::CallbackInfo &info, size_t index)
{
	const Napi::Value value = info[index];
	if (value.IsBoolean()) {
		return value.As<Napi::Boolean>().Value();
	}
	if (value.IsNumber()) {
		return value.As<Napi::Number>().DoubleValue();
	}
	if (value.IsString()) {
		return value.As<Napi::String>().Utf8Value();
	}
	if (value.IsArray()) {
		const Napi::Array array = value.As<Napi::Array>();
		if (array.Length() > 0 && array.Get(0u).IsString()) {
			std::vector<std::string> strings;
			for (uint32 i = 0; i < array.Length(); i++) {
				strings.push_back(array.Get(i).ToString().Utf8Value());
			}
			return strings;
		}
		std::vector<int32> items;
		for (uint32 i = 0; i < array.Length(); i++) {
			items.push_back(array.Get(i).As<Napi::Number>().Int32Value());
		}
		return items;
	}
	if (value.IsUndefined()) {
		return {};
	}
	throw Napi::TypeError::New(info.Env(), "expected a boolean, number, string or array of numbers or strings");
}


static void Input(const Napi::CallbackInfo &info)
{
	Node &node = NodeRegistry::Get(IdArg(info, 0));
	const EventValue value = ValueArg(info, 1);

	LooperLock lock(node.Looper());
	node.SimulateInput(value);
}


static void KeyDown(const Napi::CallbackInfo &info)
{
	Node &node = NodeRegistry::Get(IdArg(info, 0));
	const std::string bytes = StringArg(info, 1);
	std::vector<std::string> modifierNames;
	if (info[2].IsArray()) {
		const Napi::Array array = info[2].As<Napi::Array>();
		for (uint32 i = 0; i < array.Length(); i++) {
			modifierNames.push_back(array.Get(i).ToString().Utf8Value());
		}
	}
	const uint32 modifiers = ParseModifiers(modifierNames);

	LooperLock lock(node.Looper());
	node.SimulateKey(bytes, modifiers);
}


static void PressAlertButton(const Napi::CallbackInfo &info)
{
	AlertManager::PressButton(IdArg(info, 0), static_cast<int32>(NumberArg(info, 1)));
}


static void PostQuitRequest(const Napi::CallbackInfo &info)
{
	(void)info;
	ReactApplication::PostQuitRequest();
}


static Napi::Value Stats(const Napi::CallbackInfo &info)
{
	Napi::Object out = Napi::Object::New(info.Env());
	out.Set("nodes", static_cast<double>(NodeRegistry::Count()));
	out.Set("windows", ReactApplication::WindowCount());
	out.Set("bitmaps", static_cast<double>(BitmapCache::Count()));

	const std::vector<uint32> alertIds = AlertManager::OpenAlerts();
	Napi::Array alerts = Napi::Array::New(info.Env(), alertIds.size());
	for (uint32 i = 0; i < alertIds.size(); i++) {
		alerts.Set(i, alertIds[i]);
	}
	out.Set("alerts", alerts);
	return out;
}


static Napi::Value Dump(const Napi::CallbackInfo &info)
{
	Node &node = NodeRegistry::Get(IdArg(info, 0));
	Napi::Object out = Napi::Object::New(info.Env());

	LooperLock lock(node.Looper());
	node.Dump(out);
	return out;
}


//#pragma mark - Module

static Napi::Object Init(Napi::Env env, Napi::Object exports)
{
	exports.Set("start", Napi::Function::New(env, Start));
	exports.Set("quit", Napi::Function::New(env, Quit));
	exports.Set("createNode", Napi::Function::New(env, CreateNode));
	exports.Set("updateNode", Napi::Function::New(env, UpdateNode));
	exports.Set("insertBefore", Napi::Function::New(env, InsertBefore));
	exports.Set("removeChild", Napi::Function::New(env, RemoveChild));
	exports.Set("destroyRoot", Napi::Function::New(env, DestroyRoot));
	exports.Set("commitMount", Napi::Function::New(env, CommitMount));
	exports.Set("beginCommit", Napi::Function::New(env, BeginCommit));
	exports.Set("endCommit", Napi::Function::New(env, EndCommit));
	exports.Set("invoke", Napi::Function::New(env, Invoke));
	exports.Set("drag", Napi::Function::New(env, Drag));
	exports.Set("input", Napi::Function::New(env, Input));
	exports.Set("keyDown", Napi::Function::New(env, KeyDown));
	exports.Set("showAlert", Napi::Function::New(env, ShowAlert));
	exports.Set("pressAlertButton", Napi::Function::New(env, PressAlertButton));
	exports.Set("postQuitRequest", Napi::Function::New(env, PostQuitRequest));
	exports.Set("stats", Napi::Function::New(env, Stats));
	exports.Set("dump", Napi::Function::New(env, Dump));
	return exports;
}


} // HaikuReact


// NODE_API_MODULE token-pastes the function name, so it must be unqualified.
static Napi::Object InitModule(Napi::Env env, Napi::Object exports)
{
	return HaikuReact::Init(env, exports);
}


NODE_API_MODULE(haiku_react, InitModule)
