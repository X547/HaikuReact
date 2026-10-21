import {native, type NativeEventValue} from './native.js';

export type AlertType = 'empty' | 'info' | 'idea' | 'warning' | 'stop';

export interface AlertOptions {
	title?: string;
	text: string;
	/** One to three labels, left to right. Default ['OK']. */
	buttons?: readonly string[];
	/** Icon. Default 'info'. */
	type?: AlertType;
	/** Index of the button Escape chooses. Default 0 (the leftmost, as in Haiku). */
	escapeButton?: number;
}

const pending = new Map<number, (button: number) => void>();

/**
 * Shows a Haiku alert without blocking and resolves with the index of the
 * chosen button, or -1 if the application quits first. Call it after
 * render() has started the application.
 */
export function showAlert(options: AlertOptions): Promise<number> {
	const id = native.showAlert(
		options.title ?? '',
		options.text,
		options.buttons ?? ['OK'],
		options.type ?? 'info',
		options.escapeButton ?? 0
	);
	return new Promise((resolve) => pending.set(id, resolve));
}

/** Application event "alert" with [alertId, buttonIndex]. */
export function resolveAlert(value: NativeEventValue): void {
	const [id, button] = value as number[];
	pending.get(id)?.(button);
	pending.delete(id);
}

export function cancelAlerts(): void {
	for (const resolve of pending.values()) {
		resolve(-1);
	}
	pending.clear();
}
