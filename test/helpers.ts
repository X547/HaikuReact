import type {NativeDump, NativeFrame} from '../src/index.js';

export function findNode(nodes: readonly NativeDump[], predicate: (node: NativeDump) => boolean): NativeDump | undefined {
	for (const node of nodes) {
		if (predicate(node)) {
			return node;
		}
		const found = findNode(node.children, predicate);
		if (found !== undefined) {
			return found;
		}
	}
	return undefined;
}

export function findAll(nodes: readonly NativeDump[], predicate: (node: NativeDump) => boolean): NativeDump[] {
	const result: NativeDump[] = [];
	for (const node of nodes) {
		if (predicate(node)) {
			result.push(node);
		}
		result.push(...findAll(node.children, predicate));
	}
	return result;
}

export function frameOf(node: NativeDump | undefined): NativeFrame {
	if (node?.frame === undefined) {
		throw new Error(`node has no frame: ${JSON.stringify(node)}`);
	}
	return node.frame;
}

export function right(frame: NativeFrame): number {
	return frame.x + frame.width;
}

export function bottom(frame: NativeFrame): number {
	return frame.y + frame.height;
}

export async function waitFor<T>(what: string, probe: () => T | undefined | false): Promise<T> {
	const deadline = Date.now() + 3000;
	for (;;) {
		const result = probe();
		if (result !== undefined && result !== false) {
			return result;
		}
		if (Date.now() > deadline) {
			throw new Error(`timed out waiting for ${what}`);
		}
		await new Promise((resolve) => setTimeout(resolve, 10));
	}
}
