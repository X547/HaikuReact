// Typed binding to the native add-on (native/src/Addon.cpp).

import {createRequire} from 'node:module';

export type NativeEventValue = undefined | boolean | number | string | number[];

/** nodeId 0 addresses the application. */
export type NativeEventHandler = (nodeId: number, name: string, value: NativeEventValue) => void;

export interface NativeFrame {
	x: number;
	y: number;
	width: number;
	height: number;
}

export interface NativeDump {
	id: number;
	type: string;
	children: NativeDump[];
	frame?: NativeFrame;
	[property: string]: unknown;
}

export interface NativeBinding {
	/** Starts the BApplication thread; blocks until it is ready. */
	start(signature: string, onEvent: NativeEventHandler): void;
	/** Destroys all nodes and quits the application thread. */
	quit(): void;
	/** Opens an alert; the application event "alert" reports [alertId, buttonIndex]. */
	showAlert(title: string, text: string, buttons: readonly string[], type: string, escapeButton: number): number;

	createNode(type: string, props: Record<string, unknown>): number;
	/** Keys set to null or undefined reset the property to its default. */
	updateNode(id: number, props: Record<string, unknown>): void;
	/** beforeId 0 appends. Moves the child if it already belongs to the parent. */
	insertBefore(parentId: number, childId: number, beforeId: number): void;
	/** Removes and destroys the child subtree. */
	removeChild(parentId: number, childId: number): void;
	/** Destroys a parentless node; no-op for attached or unknown ids. */
	destroyRoot(id: number): void;
	commitMount(id: number): void;
	beginCommit(): void;
	endCommit(): void;

	/** Performs the node's primary user action (click, close request). */
	invoke(id: number): void;
	/** Drags with the mouse between two points in the node's view coordinates. */
	drag(id: number, fromX: number, fromY: number, toX: number, toY: number): void;
	/** Changes the node's value the way a user would, sending the usual events. */
	input(id: number, value: boolean | number | string | readonly number[]): void;
	/** Sends a key press to a window, as the keyboard would. */
	keyDown(windowId: number, bytes: string, modifiers: readonly ('shift' | 'option' | 'control' | 'command')[]): void;
	/** Clicks a button of an open alert. */
	pressAlertButton(alertId: number, index: number): void;
	/** Asks the application to quit, as Deskbar or a shutdown would. */
	postQuitRequest(): void;
	/** Live native nodes, application windows (including alerts) and open alert ids. */
	stats(): {nodes: number; windows: number; alerts: number[]};
	dump(id: number): NativeDump;
}

const require = createRequire(import.meta.url);

function loadNative(): NativeBinding {
	try {
		return require('../../build/haiku_react.node');
	} catch (error) {
		throw new Error(
			'haiku-react: the native add-on is not built. It runs on Haiku only; build it with '
				+ '"npm run build:native" (needs meson, ninja and nodejs*_devel).',
			{cause: error}
		);
	}
}

export const native: NativeBinding = loadNative();
