import assert from 'node:assert/strict';
import {after, test} from 'node:test';
import {useState} from 'react';

import {Group, Menu, MenuBar, MenuItem, render, Separator, StringView, Window} from '../src/index.js';
import {native} from '../src/native.js';
import {findAll, findNode, frameOf, waitFor} from './helpers.js';

const zooms = ['50%', '100%', '200%'];
const accept = {zoom: true};
const calls: Record<string, number> = {};
let update: (patch: {extra?: boolean}) => void = () => {};

function record(name: string): void {
	calls[name] = (calls[name] ?? 0) + 1;
}

function App() {
	const [extra, setExtra] = useState(false);
	const [zoom, setZoom] = useState('100%');
	const [wrap, setWrap] = useState(false);
	update = (patch) => {
		if (patch.extra !== undefined) {
			setExtra(patch.extra);
		}
	};

	return (
		<Window title="Menus test">
			<MenuBar>
				<Menu label="File">
					<MenuItem shortcut="N" onClick={() => record('new')}>New</MenuItem>
					<MenuItem shortcut="S" modifiers={['shift']} onClick={() => record('saveAs')}>Save as…</MenuItem>
					{extra && <MenuItem shortcut="E" onClick={() => record('extra')}>Extra</MenuItem>}
					<Separator/>
					<MenuItem shortcut="D" enabled={false} onClick={() => record('disabled')}>Disabled</MenuItem>
					<Menu label="Recent">
						<MenuItem onClick={() => record('recent')}>one.txt</MenuItem>
					</Menu>
				</Menu>
				<Menu label="View" radioMode>
					{zooms.map((item) => (
						<MenuItem
							key={item}
							marked={zoom === item}
							onClick={() => {
								record('zoom');
								if (accept.zoom) {
									setZoom(item);
								}
							}}
						>
							{item}
						</MenuItem>
					))}
				</Menu>
				<Menu label="Options">
					<MenuItem marked={wrap} onClick={() => setWrap(!wrap)}>Word wrap</MenuItem>
				</Menu>
			</MenuBar>
			<Group insets="window">
				<StringView>Zoom {zoom}</StringView>
			</Group>
		</Window>
	);
}

const root = render(<App/>, {signature: 'application/x-vnd.HaikuReact-MenusTest'});
after(() => root.quit());

const dump = () => root.dump();
const window = () => dump()[0];
const byLabel = (label: string) => findNode(dump(), (node) => node.label === label)!;
const count = (name: string) => calls[name] ?? 0;
const labels = (menu: string) => byLabel(menu).children.map((child) => child.label || child.type);
const delay = (ms: number) => new Promise((resolve) => setTimeout(resolve, ms));

test('menu bar spans the top of the window', async () => {
	await waitFor('window shown', () => dump().length === 1 && window().hidden === false);
	const menuBar = findAll(dump(), (node) => node.type === 'MenuBar')[0];
	assert.equal(frameOf(menuBar).x, 0);
	assert.equal(frameOf(menuBar).y, 0);
	assert.equal(frameOf(menuBar).width, frameOf(window()).width);
	assert.deepEqual(menuBar.children.map((menu) => menu.label), ['File', 'View', 'Options']);
});

test('menus contain items, separators and submenus in order', () => {
	assert.deepEqual(labels('File'), ['New', 'Save as…', 'Separator', 'Disabled', 'Recent']);
	assert.deepEqual(labels('Recent'), ['one.txt']);
	assert.equal(byLabel('Disabled').enabled, false);
	assert.equal(byLabel('New').shortcut, 'N');
});

test('choosing an item calls onClick', async () => {
	native.invoke(byLabel('New').id);
	await waitFor('new', () => count('new') === 1);
	native.invoke(byLabel('one.txt').id);
	await waitFor('recent', () => count('recent') === 1);
});

test('Command shortcut chooses the item', async () => {
	native.keyDown(window().id, 'n', ['command']);
	await waitFor('new by shortcut', () => count('new') === 2);
});

test('shortcut modifiers must match', async () => {
	native.keyDown(window().id, 's', ['command']);
	await delay(100);
	assert.equal(count('saveAs'), 0);
	native.keyDown(window().id, 's', ['command', 'shift']);
	await waitFor('save as by shortcut', () => count('saveAs') === 1);
});

test('disabled items ignore their shortcut', async () => {
	native.keyDown(window().id, 'd', ['command']);
	await delay(100);
	assert.equal(count('disabled'), 0);
});

test('items added later are inserted in order and get their shortcut', async () => {
	update({extra: true});
	await waitFor('extra item', () => labels('File').includes('Extra'));
	assert.deepEqual(labels('File'), ['New', 'Save as…', 'Extra', 'Separator', 'Disabled', 'Recent']);
	native.keyDown(window().id, 'e', ['command']);
	await waitFor('extra by shortcut', () => count('extra') === 1);

	update({extra: false});
	await waitFor('extra removed', () => !labels('File').includes('Extra'));
});

test('radio-mode menu marks the chosen item and snaps back when rejected', async () => {
	const marked = () => zooms.map((item) => byLabel(item).marked);
	assert.deepEqual(marked(), [false, true, false]);

	native.invoke(byLabel('200%').id);
	await waitFor('zoom 200%', () => findNode(dump(), (node) => node.text === 'Zoom 200%'));
	assert.deepEqual(marked(), [false, false, true]);

	accept.zoom = false;
	const before = count('zoom');
	native.invoke(byLabel('50%').id);
	await waitFor('zoom handler', () => count('zoom') > before);
	assert.deepEqual(marked(), [false, false, true]);
});

test('check mark follows state', async () => {
	assert.equal(byLabel('Word wrap').marked, false);
	native.invoke(byLabel('Word wrap').id);
	await waitFor('marked', () => byLabel('Word wrap').marked === true);
	native.invoke(byLabel('Word wrap').id);
	await waitFor('unmarked', () => byLabel('Word wrap').marked === false);
});
