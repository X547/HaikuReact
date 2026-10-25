import assert from 'node:assert/strict';
import {after, test} from 'node:test';
import {useState} from 'react';

import {Group, Menu, MenuBar, MenuItem, render, StringView, Window} from '../src/index.js';
import {native} from '../src/native.js';
import {findNode, waitFor} from './helpers.js';

const calls: Record<string, unknown[]> = {};
let setRecent: (recent: string[] | null) => void = () => {};
let setRefreshing: (refreshing: boolean) => void = () => {};
let setInner: (inner: string[] | null) => void = () => {};

function record(name: string, value?: unknown): void {
	(calls[name] ??= []).push(value);
}

function App() {
	const [recent, setRecentList] = useState<string[] | null>(null);
	const [refreshing, setRefreshingState] = useState(false);
	setRecent = setRecentList;
	const [outer, setOuter] = useState(false);
	const [inner, setInnerList] = useState<string[] | null>(null);
	setRefreshing = setRefreshingState;
	setInner = setInnerList;
	return (
		<Window title="Lazy menus test">
			<MenuBar>
				<Menu label="File">
					<MenuItem>New</MenuItem>
				</Menu>
				<Menu
					label="Recent"
					onOpen={() => record('open')}
					onClose={() => record('close')}
					loading={refreshing}
					loadingLabel="Fetching…"
				>
					{recent?.map((file) => <MenuItem key={file} onClick={() => record('click', file)}>{file}</MenuItem>)}
				</Menu>
				<Menu
					label="Folders"
					onOpen={() => {
						record('outerOpen');
						setOuter(true);
					}}
				>
					{outer && (
						<Menu label="Inner" onOpen={() => record('innerOpen')} onClose={() => record('innerClose')}>
							{inner?.map((name) => <MenuItem key={name}>{name}</MenuItem>)}
						</Menu>
					)}
					{outer && <MenuItem>Other</MenuItem>}
				</Menu>
			</MenuBar>
			<Group insets="window">
				<StringView>Lazy menus</StringView>
			</Group>
		</Window>
	);
}

const root = render(<App/>, {signature: 'application/x-vnd.HaikuReact-LazyMenusTest'});
after(() => root.quit());

const dump = () => root.dump();
const recentMenu = () => findNode(dump(), (node) => node.type === 'Menu' && node.label === 'Recent')!;
const menuItems = () => recentMenu().menuItems as string[];
const callCount = (name: string) => calls[name]?.length ?? 0;
const files = (count: number, prefix = 'file') => Array.from({length: count}, (_, i) => `${prefix}${i}.txt`);

async function open(): Promise<void> {
	const before = callCount('open');
	native.input(recentMenu().id, true);
	await waitFor('menu opened', () => callCount('open') > before && recentMenu().open === true);
}

async function close(): Promise<void> {
	const before = callCount('close');
	native.input(recentMenu().id, false);
	await waitFor('menu closed', () => callCount('close') > before && recentMenu().open === false);
	// Leaves the menu bar's keyboard mode.
	native.input(recentMenu().id, false);
	await new Promise((resolve) => setTimeout(resolve, 50));
}

test('window is shown', async () => {
	await waitFor('window shown', () => dump().length === 1 && dump()[0].hidden === false);
});

test('a menu with onOpen and no items shows the placeholder', () => {
	assert.equal(recentMenu().open, false);
	assert.deepEqual(menuItems(), ['Fetching…']);
});

test('opening reports onOpen; items delivered later appear in the open menu', async () => {
	await open();
	assert.deepEqual(menuItems(), ['Fetching…']);

	setRecent(['a.txt', 'b.txt']);
	await waitFor('items in the open menu', () => menuItems().join() === 'a.txt,b.txt');
	assert.equal(recentMenu().open, true);
	assert.deepEqual(recentMenu().children.map((child) => child.label), ['a.txt', 'b.txt']);
});

test('items of the open menu work', async () => {
	native.invoke(findNode(dump(), (node) => node.label === 'b.txt')!.id);
	await waitFor('click', () => callCount('click') === 1);
	assert.deepEqual(calls.click, ['b.txt']);
});

test('quick updates of the open menu are coalesced to the newest list', async () => {
	for (let i = 1; i <= 100; i++) {
		setRecent(files(1 + (i % 7), `v${i}-`));
		await new Promise((resolve) => setImmediate(resolve));
	}
	const expected = files(1 + (100 % 7), 'v100-');
	await waitFor('newest list', () => menuItems().join() === expected.join());
	assert.equal(recentMenu().open, true);
});

test('loading shows the placeholder above the items; removing all items shows it alone', async () => {
	setRecent(['a.txt']);
	setRefreshing(true);
	await waitFor('placeholder and item', () => menuItems().join() === 'Fetching…,a.txt');
	setRefreshing(false);
	setRecent([]);
	await waitFor('placeholder alone', () => menuItems().join() === 'Fetching…');
});

test('closing reports onClose', async () => {
	await close();
	assert.equal(recentMenu().open, false);
});

test('items delivered after the menu closed are there when it opens again', async () => {
	setRecent(null);
	await open();
	await close();
	setRecent(['late.txt']);
	await waitFor('applied while closed', () => menuItems().join() === 'late.txt');
	await open();
	assert.deepEqual(menuItems(), ['late.txt']);
	await close();
});

test('repeated open, update and close does not leak', async () => {
	setRecent(null);
	await waitFor('placeholder', () => menuItems().join() === 'Fetching…');
	const before = native.stats();
	for (let cycle = 0; cycle < 10; cycle++) {
		await open();
		setRecent(files(cycle % 4, `cycle${cycle}-`));
		await waitFor('items', () => menuItems().length === Math.max(cycle % 4, 1));
		await close();
	}
	setRecent(null);
	await waitFor('placeholder', () => menuItems().join() === 'Fetching…');
	// Menu windows are gone once closed.
	await waitFor('menu windows closed', () => native.stats().windows === before.windows);
	assert.deepEqual(native.stats(), before);
});

test('a lazy submenu of a lazy menu loads when it opens', async () => {
	const menu = (label: string) => findNode(dump(), (node) => node.type === 'Menu' && node.label === label)!;
	native.input(menu('Folders').id, true);
	await waitFor('outer items', () => (menu('Folders').menuItems as string[]).join() === 'Inner,Other');

	// Into the submenu from the keyboard.
	native.input(menu('Folders').id, 'down');
	await new Promise((resolve) => setTimeout(resolve, 50));
	native.input(menu('Folders').id, 'right');
	await waitFor('inner opened', () => callCount('innerOpen') === 1 && menu('Inner').open === true);
	assert.deepEqual(menu('Inner').menuItems, ['Loading…']);

	setInner(['x.txt', 'y.txt']);
	await waitFor('inner items', () => (menu('Inner').menuItems as string[]).join() === 'x.txt,y.txt');
	assert.equal(menu('Folders').open, true);

	// Escape closes the submenu, then the menu, then leaves the menu bar.
	native.input(menu('Inner').id, false);
	await waitFor('inner closed', () => callCount('innerClose') === 1);
	native.input(menu('Folders').id, false);
	await waitFor('outer closed', () => menu('Folders').open === false);
	native.input(menu('Folders').id, false);
	await new Promise((resolve) => setTimeout(resolve, 50));
});
