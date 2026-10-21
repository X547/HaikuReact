// Routes native events to props handlers and tracks the React update
// priority of the event being dispatched.

import ReconcilerConstants from 'react-reconciler/constants.js';

import {controlledProperties, groupedElements} from './elements.js';
import {findInstance, instancesOfType, type Instance, type Props} from './instances.js';
import {native, type NativeEventHandler, type NativeEventValue} from './native.js';

const {DefaultEventPriority, DiscreteEventPriority, NoEventPriority} = ReconcilerConstants;

let currentUpdatePriority: number = NoEventPriority;
let currentEventName: string | null = null;
let currentEventTime = 0;

export function setCurrentUpdatePriority(priority: number): void {
	currentUpdatePriority = priority;
}

export function getCurrentUpdatePriority(): number {
	return currentUpdatePriority;
}

export function resolveUpdatePriority(): number {
	if (currentUpdatePriority !== NoEventPriority) {
		return currentUpdatePriority;
	}
	return currentEventName !== null ? DiscreteEventPriority : DefaultEventPriority;
}

export function resolveEventType(): string | null {
	return currentEventName;
}

export function resolveEventTimeStamp(): number {
	return currentEventName !== null ? currentEventTime : performance.now();
}

/** 'click' -> 'onClick' */
function handlerProp(eventName: string): string {
	return `on${eventName[0].toUpperCase()}${eventName.slice(1)}`;
}

/** Native event values that handlers receive in a friendlier shape. */
const eventValueTransforms: Readonly<Record<string, (value: NativeEventValue) => unknown>> = {
	move(value) {
		const [x, y] = value as number[];
		return {x, y};
	},
	resize(value) {
		const [width, height] = value as number[];
		return {width, height};
	},
};

/** Sends the controlled value props of a still mounted instance to its node again. */
function restoreControlledValue(instance: Instance): void {
	const keys = controlledProperties[instance.type];
	if (keys === undefined || findInstance(instance.id) !== instance) {
		return;
	}
	const values: Props = {};
	let controlled = false;
	for (const key of keys) {
		if (instance.props[key] !== undefined) {
			values[key] = instance.props[key];
			controlled = true;
		}
	}
	if (controlled) {
		native.updateNode(instance.id, values);
	}
}

/**
 * The native control changed before JS saw the event. Once React has applied
 * the handler's updates, controlled elements show their props again, so a
 * change the handler did not accept is undone (as in React DOM).
 */
function restoreControlledValues(target: Instance): void {
	if (groupedElements.has(target.type)) {
		// Selecting a radio button or a radio-mode menu item also turned off
		// its siblings.
		for (const instance of instancesOfType(target.type)) {
			restoreControlledValue(instance);
		}
		return;
	}
	restoreControlledValue(target);
}

export interface EventRouting {
	/** Events addressed to the application (nodeId 0). */
	application(name: string, value: NativeEventValue): void;
	/** Events whose element has no handler for them. */
	unhandled(instance: Instance, name: string): void;
	/** Commits the updates scheduled by an event handler. */
	flushUpdates(): void;
}

export function createEventDispatcher(routing: EventRouting): NativeEventHandler {
	return (nodeId: number, name: string, value: NativeEventValue) => {
		if (nodeId === 0) {
			routing.application(name, value);
			return;
		}
		const instance = findInstance(nodeId);
		if (instance === undefined) {
			return;
		}

		const previousPriority = currentUpdatePriority;
		const previousEventName = currentEventName;
		currentUpdatePriority = DiscreteEventPriority;
		currentEventName = name;
		currentEventTime = performance.now();
		try {
			const handler = instance.props[handlerProp(name)];
			if (typeof handler === 'function') {
				const transform = eventValueTransforms[name];
				handler(transform !== undefined ? transform(value) : value);
			} else {
				routing.unhandled(instance, name);
			}
			routing.flushUpdates();
		} finally {
			currentUpdatePriority = previousPriority;
			currentEventName = previousEventName;
		}
		restoreControlledValues(instance);
	};
}
