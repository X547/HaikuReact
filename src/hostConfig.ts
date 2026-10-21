// react-reconciler host config (mutation mode). Instances are native nodes
// addressed by id; the root container is the application, whose children
// are windows.

import {createContext} from 'react';
import type {HostConfig} from 'react-reconciler';

import {defaultValueProperties, textProperties} from './elements.js';
import {
	getCurrentUpdatePriority,
	resolveEventTimeStamp,
	resolveEventType,
	resolveUpdatePriority,
	setCurrentUpdatePriority,
} from './events.js';
import {forgetInstance, registerInstance, type Instance, type Props} from './instances.js';
import {native} from './native.js';

export interface Container {
	readonly windows: Set<Instance>;
}

// The reconciler treats a null host context as missing, so use an object.
type HostContext = Readonly<Record<string, never>>;
const rootHostContext: HostContext = Object.freeze({});

type TimeoutHandle = ReturnType<typeof setTimeout>;

function isTextContent(children: unknown): children is string | number | ReadonlyArray<string | number> {
	if (typeof children === 'string' || typeof children === 'number') {
		return true;
	}
	return Array.isArray(children) && children.every((child) => typeof child === 'string' || typeof child === 'number');
}

/**
 * Props as the native node sees them: no children, refs, handlers or initial
 * values; text children folded into a property.
 */
function toNativeProps(type: string, props: Props): Props {
	const result: Props = {};
	for (const [key, value] of Object.entries(props)) {
		if (key === 'children' || key === 'ref' || key === 'key' || key in defaultValueProperties || typeof value === 'function') {
			continue;
		}
		result[key] = value;
	}
	const textProperty = textProperties[type];
	if (textProperty !== undefined && isTextContent(props.children)) {
		result[textProperty] = Array.isArray(props.children) ? props.children.join('') : String(props.children);
	}
	return result;
}

function sameValue(a: unknown, b: unknown): boolean {
	if (Object.is(a, b)) {
		return true;
	}
	return Array.isArray(a) && Array.isArray(b) && a.length === b.length && a.every((item, i) => Object.is(item, b[i]));
}

/** Native props for a new instance: initial values apply unless the value prop is set. */
function toInitialNativeProps(type: string, props: Props): Props {
	const result = toNativeProps(type, props);
	for (const [defaultKey, valueKey] of Object.entries(defaultValueProperties)) {
		if (props[defaultKey] !== undefined && props[valueKey] === undefined) {
			result[valueKey] = props[defaultKey];
		}
	}
	return result;
}

/** Changed native props; removed ones are sent as null (reset to default). */
function diffProps(type: string, prevProps: Props, nextProps: Props): Props | null {
	const prev = toNativeProps(type, prevProps);
	const next = toNativeProps(type, nextProps);
	let changes: Props | null = null;
	for (const key of Object.keys(next)) {
		if (!sameValue(prev[key], next[key])) {
			(changes ??= {})[key] = next[key];
		}
	}
	for (const key of Object.keys(prev)) {
		if (!(key in next)) {
			(changes ??= {})[key] = null;
		}
	}
	return changes;
}

function assertWindow(child: Instance): void {
	if (child.type !== 'Window') {
		throw new Error(`<${child.type}> must be placed inside a <Window>`);
	}
}

const HostTransitionContext = createContext<null>(null);

export const hostConfig: HostConfig<
	string,         // Type
	Props,          // Props
	Container,      // Container
	Instance,       // Instance
	never,          // TextInstance
	never,          // ActivityInstance
	never,          // SuspenseInstance
	never,          // HydratableInstance
	never,          // FormInstance
	Instance,       // PublicInstance
	HostContext,    // HostContext
	never,          // ChildSet
	TimeoutHandle,  // TimeoutHandle
	-1,             // NoTimeout
	null,           // TransitionStatus
	null,           // SuspendedState
	null,           // RendererInspectionConfig
	never,          // FormStateMarkerInstance
	never,          // HoistableRoot
	never           // Resource
> = {
	supportsMutation: true,
	supportsPersistence: false,
	supportsHydration: false,
	isPrimaryRenderer: true,
	warnsIfNotActing: false,

	rendererVersion: '0.1.0',
	rendererPackageName: 'haiku-react',
	extraDevToolsConfig: null,

	createInstance(type, props) {
		const instance: Instance = {
			id: native.createNode(type, toInitialNativeProps(type, props)),
			type,
			props,
		};
		registerInstance(instance);
		return instance;
	},

	createTextInstance(text) {
		throw new Error(`Text "${text}" must be inside an element that accepts text, such as <StringView> or <Button>`);
	},

	appendInitialChild(parent, child) {
		native.insertBefore(parent.id, child.id, 0);
	},

	finalizeInitialChildren(_instance, type) {
		return type === 'Window';
	},

	shouldSetTextContent(type, props) {
		return textProperties[type] !== undefined && isTextContent(props.children);
	},

	getRootHostContext: () => rootHostContext,
	getChildHostContext: (parentHostContext) => parentHostContext,
	getPublicInstance: (instance) => instance,

	prepareForCommit() {
		native.beginCommit();
		return null;
	},

	resetAfterCommit() {
		native.endCommit();
	},

	preparePortalMount() {},

	scheduleTimeout: (fn, delay) => setTimeout(fn, delay),
	cancelTimeout: (id) => clearTimeout(id),
	noTimeout: -1,
	supportsMicrotasks: true,
	scheduleMicrotask: queueMicrotask,

	getInstanceFromNode: () => null,
	beforeActiveInstanceBlur() {},
	afterActiveInstanceBlur() {},
	prepareScopeUpdate() {},
	getInstanceFromScope: () => null,

	detachDeletedInstance(instance) {
		forgetInstance(instance);
	},

	// Mutation

	appendChild(parent, child) {
		native.insertBefore(parent.id, child.id, 0);
	},

	appendChildToContainer(container, child) {
		assertWindow(child);
		container.windows.add(child);
	},

	insertBefore(parent, child, beforeChild) {
		native.insertBefore(parent.id, child.id, beforeChild.id);
	},

	insertInContainerBefore(container, child) {
		assertWindow(child);
		container.windows.add(child);
	},

	removeChild(parent, child) {
		native.removeChild(parent.id, child.id);
	},

	removeChildFromContainer(container, child) {
		container.windows.delete(child);
		native.destroyRoot(child.id);
	},

	clearContainer(container) {
		for (const window of container.windows) {
			native.destroyRoot(window.id);
		}
		container.windows.clear();
	},

	resetTextContent(instance) {
		const textProperty = textProperties[instance.type];
		if (textProperty !== undefined) {
			native.updateNode(instance.id, {[textProperty]: null});
		}
	},

	commitMount(instance) {
		native.commitMount(instance.id);
	},

	commitUpdate(instance, type, prevProps, nextProps) {
		const changes = diffProps(type, prevProps, nextProps);
		if (changes !== null) {
			native.updateNode(instance.id, changes);
		}
		instance.props = nextProps;
	},

	hideInstance(instance) {
		native.updateNode(instance.id, instance.type === 'Window' ? {visible: false} : {hidden: true});
	},

	unhideInstance(instance, props) {
		native.updateNode(instance.id, instance.type === 'Window' ? {visible: props.visible ?? true} : {hidden: props.hidden ?? false});
	},

	// Priorities and scheduling

	setCurrentUpdatePriority,
	getCurrentUpdatePriority,
	resolveUpdatePriority,
	resolveEventType,
	resolveEventTimeStamp,
	trackSchedulerEvent() {},
	shouldAttemptEagerTransition: () => false,
	requestPostPaintCallback() {},

	// Transitions and forms (unused)

	NotPendingTransition: null,
	HostTransitionContext,
	resetFormInstance() {},

	// Suspended commits (no host component ever suspends)

	maySuspendCommit: () => false,
	maySuspendCommitOnUpdate: () => false,
	maySuspendCommitInSyncRender: () => false,
	preloadInstance: () => true,
	startSuspendingCommit: () => null,
	suspendInstance() {},
	suspendOnActiveViewTransition() {},
	waitForCommitToBeReady: () => null,
	getSuspendedCommitReason: () => null,

	bindToConsole(methodName, args) {
		const method = (console as unknown as Record<string, (...items: unknown[]) => unknown>)[methodName];
		return method.bind(console, ...args);
	},
};
