// Prints the native tree of the layout test app with layout constraints.
// Run: node dist/test/diagnose-layout.js

import {render, type NativeDump} from '../src/index.js';
import {LayoutApp} from './layoutApp.js';

function print(node: NativeDump, depth = 0): void {
	const {children, type, frame, layout, label, text} = node;
	console.log(`${'  '.repeat(depth)}${type} ${label ?? text ?? ''}`, JSON.stringify({frame, layout}));
	for (const child of children) {
		print(child, depth + 1);
	}
}

const root = render(<LayoutApp/>);

setTimeout(() => {
	for (const window of root.dump()) {
		print(window);
	}
	root.quit();
}, 500);
