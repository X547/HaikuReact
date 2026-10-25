import assert from 'node:assert/strict';
import {after, test} from 'node:test';
import {useState} from 'react';

import {Group, Menu, MenuBar, MenuField, MenuItem, render, Window, type NativeDump} from '../src/index.js';
import {findAll, findNode, waitFor} from './helpers.js';

const textIcon = {mimeType: 'text/plain'};
let setBarIconSize: (size: number) => void = () => {};

function App() {
	const [barIconSize, setSize] = useState(16);
	setBarIconSize = setSize;
	return (
		<Window title="Menu icons test">
			<MenuBar iconSize={barIconSize}>
				<Menu icon={{app: 'application/x-vnd.Haiku-StyledEdit'}}>
					<MenuItem icon={textIcon} shortcut="N">New</MenuItem>
					<MenuItem>Open</MenuItem>
					<MenuItem icon={textIcon} enabled={false}>Disabled</MenuItem>
				</Menu>
				<Menu label="Edit" icon={{mimeType: 'application/x-vnd.Be-directory'}}>
					<MenuItem>Cut</MenuItem>
				</Menu>
				<Menu label="Edit">
					<MenuItem>Copy</MenuItem>
				</Menu>
			</MenuBar>
			<Group insets="window">
				<MenuField
					label="Type"
					items={['Plain', {label: 'Text', icon: textIcon}, {label: 'Folder', icon: {mimeType: 'application/x-vnd.Be-directory'}}]}
					defaultValue={1}
				/>
			</Group>
		</Window>
	);
}

const root = render(<App/>, {signature: 'application/x-vnd.HaikuReact-MenuIconsTest'});
after(() => root.quit());

interface Size {
	width: number;
	height: number;
}

const dump = () => root.dump();
const menuBar = () => findAll(dump(), (node) => node.type === 'MenuBar')[0];
const bar = (index: number) => menuBar().children[index];
const byLabel = (label: string) => findNode(dump(), (node) => node.label === label)!;
const contentSize = (node: NativeDump) => node.contentSize as Size;

test('window is shown', async () => {
	await waitFor('window shown', () => dump().length === 1 && dump()[0].hidden === false);
});

test('an icon-only menu bar item is as wide as its icon', async () => {
	const iconOnly = bar(0);
	assert.equal(iconOnly.label, '');
	assert.equal(iconOnly.hasIcon, true);
	await waitFor('menu bar laid out', () => (bar(0).itemFrame as Size | undefined)?.width !== undefined);
	assert.equal(contentSize(iconOnly).width, iconOnly.iconSize);
	assert.ok(contentSize(iconOnly).height >= (iconOnly.iconSize as number));
});

test('an icon adds its size and a spacing to the label', () => {
	const withIcon = contentSize(bar(1));
	const plain = contentSize(bar(2));
	assert.equal(bar(1).hasIcon, true);
	assert.equal(bar(2).hasIcon, false);
	assert.ok(withIcon.width > plain.width + (bar(1).iconSize as number), `${withIcon.width} vs ${plain.width}`);
	assert.ok(withIcon.height >= plain.height);
	assert.ok((bar(1).itemFrame as Size).width > (bar(2).itemFrame as Size).width);
});

test('menu items show icons; items without one keep the plain size', () => {
	assert.equal(byLabel('New').hasIcon, true);
	assert.equal(byLabel('Open').hasIcon, false);
	assert.equal(byLabel('Disabled').hasIcon, true);
	assert.equal(byLabel('Disabled').enabled, false);
	assert.ok(contentSize(byLabel('New')).height >= (byLabel('New').iconSize as number));
});

test('iconSize on the menu bar resizes its items\' icons', async () => {
	const before = bar(0).iconSize as number;
	setBarIconSize(32);
	await waitFor('bigger icons', () => (bar(0).iconSize as number) >= 2 * before);
	assert.equal(bar(1).iconSize, bar(0).iconSize);
	await waitFor('relayout', () => contentSize(bar(0)).width === bar(0).iconSize);
	// Items inside the menus keep the menu's own size.
	assert.equal(byLabel('New').iconSize, before);
});

test('menu field items take icons', () => {
	const items = byLabel('Type').items as {label: string; hasIcon: boolean}[];
	assert.deepEqual(items.map((item) => [item.label, item.hasIcon]), [['Plain', false], ['Text', true], ['Folder', true]]);
	assert.equal(byLabel('Type').value, 1);
});
