import type {ReactNode} from 'react';
import Reconciler from 'react-reconciler';
import ReconcilerConstants from 'react-reconciler/constants.js';

import {cancelAlerts, resolveAlert} from './alert.js';
import {createEventDispatcher} from './events.js';
import {hostConfig, type Container} from './hostConfig.js';
import {native, type NativeDump} from './native.js';

const {ConcurrentRoot} = ReconcilerConstants;

const reconciler = Reconciler(hostConfig);

export interface RenderOptions {
	/** MIME application signature. Default 'application/x-vnd.HaikuReact-App'. */
	signature?: string;
	/**
	 * Quit was requested from outside (Command+Q, Deskbar, shutdown). Return
	 * false to refuse; by default the application quits.
	 */
	onQuitRequest?: () => boolean | void;
	/** Enables React Strict Mode checks for the whole tree. */
	strictMode?: boolean;
	/** An error caught by an error boundary. Default: logged to the console. */
	onCaughtError?: (error: unknown, info: {componentStack?: string | null}) => void;
	/**
	 * An error no error boundary caught. Default: logged, the exit code set
	 * to 1 and the application quit.
	 */
	onUncaughtError?: (error: unknown, info: {componentStack?: string | null}) => void;
}

export interface HaikuRoot {
	render(element: ReactNode): void;
	/** Unmounts everything and ends the application; Node exits once idle. */
	quit(): void;
	/** Native node trees of the mounted windows, for tests and debugging. */
	dump(): NativeDump[];
}

let currentRoot: HaikuRoot | null = null;

/** Starts the Haiku application and renders `element`, which must consist of <Window> elements. */
export function render(element: ReactNode, options: RenderOptions = {}): HaikuRoot {
	if (currentRoot !== null) {
		throw new Error('render() can only be called once per process; use root.render() to update');
	}

	const container: Container = {windows: new Set()};
	let fiberRoot: ReturnType<typeof reconciler.createContainer> | null = null;

	const quit = () => {
		if (fiberRoot === null) {
			return;
		}
		const root = fiberRoot;
		fiberRoot = null;
		reconciler.updateContainerSync(null, root, null, null);
		reconciler.flushSyncWork();
		native.quit();
	};

	const reportError = (error: unknown) => {
		console.error(error);
	};

	native.start(options.signature ?? 'application/x-vnd.HaikuReact-App', createEventDispatcher({
		application(name, value) {
			switch (name) {
				case 'quitRequest':
					if (options.onQuitRequest?.() !== false) {
						quit();
					}
					break;
				case 'alert':
					resolveAlert(value);
					break;
				case 'quit':
					cancelAlerts();
					break;
			}
		},
		unhandled(_instance, name) {
			if (name === 'closeRequest') {
				quit();
			}
		},
		flushUpdates() {
			reconciler.flushSyncWork();
		},
	}));

	fiberRoot = reconciler.createContainer(
		container,
		ConcurrentRoot,
		null,
		options.strictMode ?? false,
		null,
		'',
		options.onUncaughtError ?? ((error) => {
			reportError(error);
			process.exitCode = 1;
			queueMicrotask(quit);
		}),
		options.onCaughtError ?? reportError,
		reportError,
		() => {},
		null
	);

	const root: HaikuRoot = {
		render(next) {
			if (fiberRoot !== null) {
				reconciler.updateContainer(next, fiberRoot, null, null);
			}
		},
		quit,
		dump() {
			return [...container.windows].map((window) => native.dump(window.id));
		},
	};
	currentRoot = root;
	root.render(element);
	return root;
}

/** Quits the application started by render(). */
export function quit(): void {
	currentRoot?.quit();
}
