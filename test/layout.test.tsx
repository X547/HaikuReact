import assert from 'node:assert/strict';
import {after, test} from 'node:test';

import {render} from '../src/index.js';
import {native} from '../src/native.js';
import {findAll, findNode, frameOf, right, waitFor} from './helpers.js';
import {LayoutApp, tabChanges, update} from './layoutApp.js';

const root = render(<LayoutApp/>, {signature: 'application/x-vnd.HaikuReact-LayoutTest'});
after(() => root.quit());

const dump = () => root.dump();
const byLabel = (label: string) => findNode(dump(), (node) => node.label === label);
const byText = (text: string) => findNode(dump(), (node) => node.text === text);
const byType = (type: string, index = 0) => findAll(dump(), (node) => node.type === type)[index];
const tabLabels = (tabs: ReturnType<typeof byType>) => tabs.children.map((tab) => tab.label);

test('window uses the requested width', async () => {
	await waitFor('window shown', () => dump().length === 1 && dump()[0].hidden === false);
	assert.equal(frameOf(dump()[0]).width, 600);
});

test('grid places children by column, row and span', () => {
	const g00 = frameOf(byLabel('G00'));
	const g10 = frameOf(byLabel('G10'));
	const span = frameOf(byLabel('Span'));
	assert.equal(g10.y, g00.y);
	assert.equal(g10.x, right(g00));
	assert.equal(span.x, g00.x);
	assert.equal(span.y, g00.y + g00.height);
	assert.equal(span.width, g00.width + g10.width);
});

test('grid moves a child when its cell changes', async () => {
	update({gridSwap: true});
	await waitFor('G10 moved', () => frameOf(byLabel('G10')).y > frameOf(byLabel('Span')).y && true);
	assert.equal(frameOf(byLabel('G10')).x, frameOf(byLabel('G00')).x);
});

test('strut adds fixed space and glue takes the rest', () => {
	const group = findNode(dump(), (node) => node.type === 'Group' && node.children.some((child) => child.type === 'Strut'));
	const a = frameOf(byLabel('A'));
	const b = frameOf(byLabel('B'));
	const end = frameOf(byLabel('End'));
	assert.equal(b.x - right(a), 30);
	assert.equal(right(end), frameOf(group).width);
});

test('split puts children side by side', () => {
	const left = frameOf(byLabel('Left'));
	const rightBox = frameOf(byLabel('Right'));
	assert.equal(left.y, rightBox.y);
	assert.ok(right(left) < rightBox.x);
});

test('split divider can be dragged', async () => {
	const before = {left: frameOf(byLabel('Left')), right: frameOf(byLabel('Right'))};
	const splitterX = (right(before.left) + before.right.x) / 2;
	const y = before.left.y + before.left.height / 2;

	native.drag(byType('Split').id, splitterX, y, splitterX + 60, y);

	const after = await waitFor('divider moved', () => {
		const left = frameOf(byLabel('Left'));
		return left.width !== before.left.width && {left, right: frameOf(byLabel('Right'))};
	});
	assert.equal(after.left.width, before.left.width + 60);
	assert.equal(after.right.width, before.right.width - 60);
});

test('widening the window widens the containers', async () => {
	const gridBefore = frameOf(byType('Grid'));
	const splitBefore = frameOf(byType('Split'));
	update({windowWidth: 800});
	await waitFor('window widened', () => frameOf(dump()[0]).width === 800);
	await waitFor('grid widened', () => frameOf(byType('Grid')).width === gridBefore.width + 200);
	assert.equal(frameOf(byType('Split')).width, splitBefore.width + 200);
	assert.equal(frameOf(byType('Scroll')).width, gridBefore.width + 200);
});

test('cards show only the active child', async () => {
	assert.deepEqual(['card0', 'card1', 'card2'].map((text) => byText(text)!.hidden), [false, true, true]);
	update({activeCard: 2});
	await waitFor('card2 shown', () => byText('card2')!.hidden === false);
	assert.deepEqual(['card0', 'card1', 'card2'].map((text) => byText(text)!.hidden), [true, true, false]);
	assert.equal(byType('Cards').activeIndex, 2);
});

test('uncontrolled tabs keep the selected tab when tabs are inserted and removed', async () => {
	const tabs = () => byType('Tabs', 0);
	assert.equal(tabs().selected, 0);

	native.invoke(tabs().id);
	await waitFor('tab Two selected', () => tabs().selected === 1);

	update({extraTab: true});
	await waitFor('tab inserted', () => tabs().children.length === 3);
	assert.deepEqual(tabLabels(tabs()), ['New', 'One', 'Two']);
	assert.equal(tabs().selected, 2, 'selection stays on "Two"');

	update({extraTab: false});
	await waitFor('tab removed', () => tabs().children.length === 2);
	assert.deepEqual(tabLabels(tabs()), ['One', 'Two']);
	assert.equal(tabs().selected, 1, 'selection stays on "Two"');
});

test('controlled tabs follow the selected prop and report user changes', async () => {
	const tabs = () => byType('Tabs', 1);
	assert.equal(tabs().selected, 1);

	native.invoke(tabs().id);
	await waitFor('change event', () => tabChanges.length === 1);
	assert.deepEqual(tabChanges, [2]);

	update({controlledTab: 0});
	await waitFor('tab C0 selected', () => tabs().selected === 0);
});

test('scroll view scrolls content taller than its maximum height', () => {
	const scroll = byType('Scroll');
	const viewport = scroll.viewport as {width: number; height: number};
	const content = frameOf(scroll.children[0]);
	assert.ok(frameOf(scroll).height <= 120);
	assert.ok(content.height > viewport.height, 'content is taller than the viewport');
	assert.equal(content.width, viewport.width, 'content fills the viewport width');
	assert.ok((scroll.verticalRange as number) > 0);
	assert.equal(scroll.verticalRange, content.height - viewport.height);
});

test('explicit size and alignment props', () => {
	const wide = frameOf(byLabel('Wide'));
	assert.ok(wide.width >= 250);
	assert.ok(wide.x > 10, 'centered rather than at the left inset');
});
