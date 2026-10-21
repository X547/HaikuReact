import assert from 'node:assert/strict';
import {after, test} from 'node:test';
import {useState} from 'react';

import {
	Box,
	Button,
	CheckBox,
	Grid,
	Group,
	ListView,
	Menu,
	MenuBar,
	MenuField,
	MenuItem,
	render,
	Scroll,
	Slider,
	Split,
	StringView,
	Tab,
	Tabs,
	TextControl,
	TextView,
	Window,
} from '../src/index.js';
import {native} from '../src/native.js';
import {waitFor} from './helpers.js';

let setWindows: (count: number) => void = () => {};
let setRows: (count: number) => void = () => {};

// A window that uses every kind of node, so that teardown of all of them runs.
function Everything({index, rows}: {index: number; rows: number}) {
	return (
		<Window title={`Window ${index}`} left={60 + index * 30} top={60 + index * 30}>
			<MenuBar>
				<Menu label="File">
					<MenuItem shortcut="N">New</MenuItem>
					<Menu label="More"><MenuItem>Item</MenuItem></Menu>
				</Menu>
			</MenuBar>
			<Group insets="window">
				<Tabs>
					<Tab label="Controls">
						<Grid>
							<TextControl column={0} row={0} label="Text" defaultValue="value"/>
							<CheckBox column={0} row={1}>Check</CheckBox>
							<Slider column={0} row={2} defaultValue={40}/>
							<MenuField column={0} row={3} label="Pick" items={['A', 'B']} defaultValue={0}/>
						</Grid>
					</Tab>
					<Tab label="Lists">
						<Split>
							<Scroll><ListView items={['One', 'Two', 'Three']}/></Scroll>
							<Scroll><TextView defaultText="Text"/></Scroll>
						</Split>
					</Tab>
				</Tabs>
				<Box label="Rows">
					{Array.from({length: rows}, (_, row) => (
						<Group key={row} orientation="horizontal">
							<StringView>{`Row ${row}`}</StringView>
							<Button>Remove</Button>
						</Group>
					))}
				</Box>
			</Group>
		</Window>
	);
}

function App() {
	const [windows, setWindowCount] = useState(0);
	const [rows, setRowCount] = useState(0);
	setWindows = setWindowCount;
	setRows = setRowCount;
	return Array.from({length: windows}, (_, index) => <Everything key={index} index={index} rows={rows}/>);
}

const root = render(<App/>, {signature: 'application/x-vnd.HaikuReact-LifecycleTest'});
after(() => root.quit());

let baseline = native.stats();

test('baseline', async () => {
	await new Promise((resolve) => setTimeout(resolve, 100));
	baseline = native.stats();
	assert.equal(baseline.nodes, 0);
});

test('mounting and unmounting windows releases all nodes and windows', async () => {
	for (let cycle = 0; cycle < 10; cycle++) {
		setWindows(3);
		await waitFor(`cycle ${cycle} mounted`, () => root.dump().length === 3 && root.dump().every((window) => !window.hidden));
		const mounted = native.stats();
		assert.ok(mounted.nodes > 50, `nodes while mounted: ${mounted.nodes}`);
		assert.equal(mounted.windows, baseline.windows + 3);

		setWindows(0);
		await waitFor(`cycle ${cycle} unmounted`, () => root.dump().length === 0);
		assert.deepEqual(native.stats(), baseline);
	}
});

test('adding and removing children inside a window keeps the node count stable', async () => {
	setWindows(1);
	await waitFor('mounted', () => root.dump().length === 1);
	const before = native.stats().nodes;

	for (let cycle = 0; cycle < 10; cycle++) {
		setRows(20);
		await waitFor('rows added', () => native.stats().nodes === before + 20 * 3);
		setRows(0);
		await waitFor('rows removed', () => native.stats().nodes === before);
	}

	setWindows(0);
	await waitFor('unmounted', () => root.dump().length === 0);
	assert.deepEqual(native.stats(), baseline);
});
