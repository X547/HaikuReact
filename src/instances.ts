// JS side of host instances: the native node id plus the latest props, which
// event dispatch reads handlers from.

import {native} from './native.js';

export type Props = Record<string, unknown>;

export interface Instance {
	readonly id: number;
	readonly type: string;
	props: Props;
}

// Weak, so that instances React creates but never commits (an interrupted
// render) can be collected; their native nodes are destroyed then.
const instancesById = new Map<number, WeakRef<Instance>>();

const orphans = new FinalizationRegistry<number>((id) => {
	instancesById.delete(id);
	native.destroyRoot(id);
});

export function registerInstance(instance: Instance): void {
	instancesById.set(instance.id, new WeakRef(instance));
	orphans.register(instance, instance.id, instance);
}

export function forgetInstance(instance: Instance): void {
	instancesById.delete(instance.id);
	orphans.unregister(instance);
}

export function findInstance(id: number): Instance | undefined {
	return instancesById.get(id)?.deref();
}

export function* instancesOfType(type: string): Generator<Instance> {
	for (const reference of instancesById.values()) {
		const instance = reference.deref();
		if (instance?.type === type) {
			yield instance;
		}
	}
}
