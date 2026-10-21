import assert from 'node:assert/strict';
import {test} from 'node:test';
import {useState} from 'react';

import {Button, Group, render, StringView, Window} from '../src/index.js';
import {native} from '../src/native.js';
import {findNode, frameOf, right, waitFor} from './helpers.js';

let closeRequests = 0;

function App() {
	const [count, setCount] = useState(0);
	const [open, setOpen] = useState(true);

	if (!open) {
		return null;
	}
	return (
		<Window
			title="HaikuReact test"
			onCloseRequest={() => {
				closeRequests++;
				setOpen(false);
			}}
		>
			<Group insets="window">
				<StringView>Clicked {count} times</StringView>
				{count % 2 === 1 && <StringView>Odd</StringView>}
				<Group orientation="horizontal">
					<Button onClick={() => setCount(count + 1)}>Increment</Button>
					<Button enabled={count > 0} onClick={() => setCount(0)}>Reset</Button>
				</Group>
			</Group>
		</Window>
	);
}

const root = render(<App/>, {signature: 'application/x-vnd.HaikuReact-Test'});

const byLabel = (label: string) => findNode(root.dump(), (node) => node.label === label);
const byText = (text: string) => findNode(root.dump(), (node) => node.text === text);

test('mounts and shows the window', async () => {
	const [window] = await waitFor('window', () => root.dump().length === 1 && root.dump());
	assert.equal(window.type, 'Window');
	assert.equal(window.title, 'HaikuReact test');
	await waitFor('window shown', () => root.dump()[0].hidden === false);
	assert.ok(byText('Clicked 0 times'));
	assert.equal(byLabel('Reset')?.enabled, false);
});

test('lays out children with the Haiku layout engine', () => {
	const [window] = root.dump();
	const outer = window.children[0];
	const [label, buttons] = outer.children;
	const [increment, reset] = buttons.children;

	assert.ok(frameOf(label).y < frameOf(buttons).y, 'label above buttons');
	assert.equal(frameOf(increment).y, frameOf(reset).y, 'buttons on one row');
	assert.ok(right(frameOf(increment)) <= frameOf(reset).x, 'buttons side by side');
	assert.ok(frameOf(outer).width <= frameOf(window).width, 'group fits the window');
	assert.ok(frameOf(label).width > 0 && frameOf(label).height > 0);
});

test('button click updates React state and native views', async () => {
	native.invoke(byLabel('Increment')!.id);
	await waitFor('counter 1', () => byText('Clicked 1 times'));
	assert.equal(byLabel('Reset')?.enabled, true);
});

test('conditional child is inserted in order', () => {
	const outer = root.dump()[0].children[0];
	assert.deepEqual(outer.children.map((child) => child.text ?? child.type), ['Clicked 1 times', 'Odd', 'Group']);
});

test('removing a child destroys it', async () => {
	native.invoke(byLabel('Increment')!.id);
	await waitFor('counter 2', () => byText('Clicked 2 times'));
	const outer = root.dump()[0].children[0];
	assert.deepEqual(outer.children.map((child) => child.text ?? child.type), ['Clicked 2 times', 'Group']);
	assert.equal(byText('Odd'), undefined);
});

test('close request unmounts the window', async () => {
	native.invoke(root.dump()[0].id);
	await waitFor('window removed', () => root.dump().length === 0);
	assert.equal(closeRequests, 1);
	root.quit();
});
