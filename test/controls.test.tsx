import assert from 'node:assert/strict';
import {after, test} from 'node:test';
import {useState} from 'react';

import {
	CheckBox,
	Group,
	ListView,
	MenuField,
	RadioButton,
	render,
	Scroll,
	Slider,
	StatusBar,
	TextControl,
	TextView,
	Window,
} from '../src/index.js';
import {native} from '../src/native.js';
import {findAll, findNode, waitFor} from './helpers.js';

// Whether the handlers accept the user's change.
const accept = {checkBox: true, radio: true, menu: true};
// Handler calls per event, accepted or not.
const calls: Record<string, unknown[]> = {};

function record(name: string, value: unknown): void {
	(calls[name] ??= []).push(value);
}

function App() {
	const [agree, setAgree] = useState(false);
	const [size, setSize] = useState('Medium');
	const [digits, setDigits] = useState('');
	const [notes, setNotes] = useState('');
	const [volume, setVolume] = useState(10);
	const [selection, setSelection] = useState<number[]>([]);
	const [color, setColor] = useState(1);

	return (
		<Window title="Controls test">
			<Group insets="window">
				<CheckBox
					checked={agree}
					onChange={(checked) => {
						record('agree', checked);
						if (accept.checkBox) {
							setAgree(checked);
						}
					}}
				>
					Agree
				</CheckBox>
				<CheckBox defaultChecked onChange={(checked) => record('remember', checked)}>Remember</CheckBox>
				<Group orientation="horizontal">
					{['Small', 'Medium', 'Large'].map((name) => (
						<RadioButton
							key={name}
							checked={size === name}
							onChange={() => {
								record('size', name);
								if (accept.radio) {
									setSize(name);
								}
							}}
						>
							{name}
						</RadioButton>
					))}
				</Group>
				<TextControl
					label="Digits"
					value={digits}
					onChange={(value) => {
						record('digits', value);
						setDigits(value.replace(/\D/g, ''));
					}}
					onSubmit={(value) => record('submit', value)}
				/>
				<TextControl label="Name" defaultValue="hello" onChange={(value) => record('name', value)}/>
				<Scroll maxHeight={80}>
					<TextView
						text={notes}
						onChange={(text) => {
							record('notes', text);
							setNotes(text);
						}}
					/>
				</Scroll>
				<Slider
					label="Volume"
					value={volume}
					onChange={(value) => {
						record('volume', value);
						setVolume(Math.min(value, 50));
					}}
				/>
				<StatusBar label="Level" value={volume}/>
				<Scroll maxHeight={80}>
					<ListView
						items={['Alpha', 'Beta', 'Gamma', 'Delta']}
						multiple
						selection={selection}
						onSelectionChange={(indices) => {
							record('selection', indices);
							setSelection(indices);
						}}
					/>
				</Scroll>
				<MenuField
					label="Color"
					items={['Red', 'Green', 'Blue']}
					value={color}
					onChange={(index) => {
						record('color', index);
						if (accept.menu) {
							setColor(index);
						}
					}}
				/>
			</Group>
		</Window>
	);
}

const root = render(<App/>, {signature: 'application/x-vnd.HaikuReact-ControlsTest'});
after(() => root.quit());

const dump = () => root.dump();
const byLabel = (label: string) => findNode(dump(), (node) => node.label === label)!;
const byType = (type: string, index = 0) => findAll(dump(), (node) => node.type === type)[index];
const callCount = (name: string) => calls[name]?.length ?? 0;

/** Sends user input and waits until its handler ran (and controlled values were restored). */
async function userInput(id: number, value: boolean | number | string | readonly number[], handler: string) {
	const before = callCount(handler);
	native.input(id, value);
	await waitFor(`${handler} handler`, () => callCount(handler) > before);
}

test('window is shown', async () => {
	await waitFor('window shown', () => dump().length === 1 && dump()[0].hidden === false);
});

test('controlled checkbox follows accepted changes', async () => {
	await userInput(byLabel('Agree').id, true, 'agree');
	assert.deepEqual(calls.agree, [true]);
	assert.equal(byLabel('Agree').value, 1);
});

test('controlled checkbox snaps back when the change is rejected', async () => {
	accept.checkBox = false;
	await userInput(byLabel('Agree').id, false, 'agree');
	assert.deepEqual(calls.agree, [true, false]);
	assert.equal(byLabel('Agree').value, 1);
});

test('uncontrolled checkbox starts from defaultChecked and keeps changes', async () => {
	assert.equal(byLabel('Remember').value, 1);
	await userInput(byLabel('Remember').id, false, 'remember');
	assert.equal(byLabel('Remember').value, 0);
});

test('radio buttons select one and snap back as a group', async () => {
	const values = () => ['Small', 'Medium', 'Large'].map((name) => byLabel(name).value);
	assert.deepEqual(values(), [0, 1, 0]);

	await userInput(byLabel('Small').id, true, 'size');
	assert.deepEqual(values(), [1, 0, 0]);

	accept.radio = false;
	await userInput(byLabel('Large').id, true, 'size');
	assert.deepEqual(calls.size, ['Small', 'Large']);
	assert.deepEqual(values(), [1, 0, 0]);
});

test('text control value is filtered by the handler', async () => {
	await userInput(byLabel('Digits').id, '12a3', 'digits');
	assert.deepEqual(calls.digits, ['12a3']);
	assert.equal(byLabel('Digits').text, '123');
});

test('text control submit reports the text', async () => {
	native.invoke(byLabel('Digits').id);
	await waitFor('submit', () => callCount('submit') === 1);
	assert.deepEqual(calls.submit, ['123']);
});

test('uncontrolled text control starts from defaultValue', () => {
	assert.equal(byLabel('Name').text, 'hello');
});

test('text view reports edits and is the scroll target', async () => {
	const textView = byType('TextView');
	assert.equal(byType('Scroll', 0).children[0].type, 'TextView');
	await userInput(textView.id, 'first line\nsecond line', 'notes');
	await waitFor('notes state', () => byType('TextView').text === 'first line\nsecond line');
	assert.equal(calls.notes.at(-1), 'first line\nsecond line');
});

test('slider is clamped by the handler and drives the status bar', async () => {
	await userInput(byLabel('Volume').id, 80, 'volume');
	assert.deepEqual(calls.volume, [80]);
	assert.equal(byLabel('Volume').value, 50);
	assert.equal(byType('StatusBar').value, 50);
});

test('list view selection is reported and controlled', async () => {
	await userInput(byType('ListView').id, [0, 2], 'selection');
	assert.deepEqual(calls.selection.at(-1), [0, 2]);
	assert.deepEqual(byType('ListView').selection, [0, 2]);
});

test('menu field reports choices and snaps back when rejected', async () => {
	assert.equal(byLabel('Color').value, 1);
	await userInput(byLabel('Color').id, 2, 'color');
	assert.equal(byLabel('Color').value, 2);

	accept.menu = false;
	await userInput(byLabel('Color').id, 0, 'color');
	assert.deepEqual(calls.color, [2, 0]);
	assert.equal(byLabel('Color').value, 2);
});
