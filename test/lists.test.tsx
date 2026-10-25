import assert from 'node:assert/strict';
import {after, test} from 'node:test';
import {useState} from 'react';

import {ListItem, ListView, render, Scroll, Window} from '../src/index.js';
import {native} from '../src/native.js';
import {findAll, findNode, waitFor} from './helpers.js';

const folder = {mimeType: 'application/x-vnd.Be-directory'};
const calls: Record<string, unknown[]> = {};
let setDocs: (docs: string[]) => void = () => {};
let setMusic: (music: string[] | null) => void = () => {};
let setRefreshing: (refreshing: boolean) => void = () => {};
let setEmptyLoaded: (loaded: boolean) => void = () => {};

function record(name: string, value?: unknown): void {
	(calls[name] ??= []).push(value);
}

function App() {
	const [docs, setDocList] = useState(['a.txt', 'b.txt']);
	const [music, setMusicList] = useState<string[] | null>(null);
	const [musicExpanded, setMusicExpanded] = useState(false);
	const [refreshing, setRefreshingState] = useState(false);
	const [selection, setSelection] = useState<string[]>([]);
	setDocs = setDocList;
	setMusic = setMusicList;
	const [emptyLoaded, setEmptyLoadedState] = useState(false);
	setRefreshing = setRefreshingState;
	setEmptyLoaded = setEmptyLoadedState;

	return (
		<Window title="Lists test">
			<Scroll minHeight={200}>
				<ListView
					multiple
					selection={selection}
					onSelectionChange={(ids) => {
						record('selection', ids);
						setSelection(ids);
					}}
					onInvoke={(id) => record('invoke', id)}
				>
					<ListItem id="docs" label="Documents" icon={folder} defaultExpanded loading={refreshing}>
						{docs.map((name) => <ListItem key={name} id={`docs/${name}`} label={name}/>)}
					</ListItem>
					<ListItem
						id="music"
						label="Music"
						hasChildren
						expanded={musicExpanded}
						loadingLabel="Fetching…"
						onExpand={() => {
							record('musicExpand');
							setMusicExpanded(true);
						}}
						onCollapse={() => {
							record('musicCollapse');
							setMusicExpanded(false);
						}}
					>
						{music?.map((name) => (
							<ListItem key={name} id={`music/${name}`} label={name} hasChildren={name === 'Albums'}/>
						))}
					</ListItem>
					<ListItem
						id="pictures"
						label="Pictures"
						hasChildren
						expanded={false}
						onExpand={() => record('picturesExpand')}
					/>
				</ListView>
			</Scroll>
			<Scroll>
				<ListView items={['One', {label: 'Two', icon: folder}]} defaultSelection={[1]}/>
			</Scroll>
			<Scroll>
				<ListView>
					<ListItem id="empty" label="Empty" hasChildren={!emptyLoaded} defaultExpanded/>
				</ListView>
			</Scroll>
		</Window>
	);
}

const root = render(<App/>, {signature: 'application/x-vnd.HaikuReact-ListsTest'});
after(() => root.quit());

interface Row {
	id: string | null;
	label: string;
	level: number;
	placeholder: boolean;
}

const dump = () => root.dump();
const lists = () => findAll(dump(), (node) => node.type === 'ListView');
const tree = () => lists()[0];
const rows = () => (tree().rows as Row[]).map((row) => `${'  '.repeat(row.level)}${row.label}`);
const item = (id: string) => findNode(dump(), (node) => node.type === 'ListItem' && node.itemId === id)!;
const callCount = (name: string) => calls[name]?.length ?? 0;

async function userInput(id: number, value: boolean | readonly string[], handler: string) {
	const before = callCount(handler);
	native.input(id, value);
	await waitFor(`${handler} handler`, () => callCount(handler) > before);
}

test('window is shown', async () => {
	await waitFor('window shown', () => dump().length === 1 && dump()[0].hidden === false);
});

test('items prop lists entries by index, with icons', () => {
	const flat = lists()[1];
	assert.deepEqual((flat.rows as Row[]).map((row) => [row.id, row.label]), [['0', 'One'], ['1', 'Two']]);
	assert.deepEqual(flat.selection, [1]);
});

test('nested items show under expanded parents; lazy ones show an arrow but no children', () => {
	assert.deepEqual(rows(), ['Documents', '  a.txt', '  b.txt', 'Music', 'Pictures']);
	assert.equal(item('docs').expanded, true);
	assert.equal(item('docs').hasIcon, true);
	assert.equal(item('music').expanded, false);
	// The placeholder exists (it makes the arrow appear) but is hidden.
	assert.equal(item('music').placeholder, 'Fetching…');
});

test('children are inserted, moved and removed under their parent', async () => {
	setDocs(['a.txt', 'c.txt', 'b.txt']);
	await waitFor('inserted', () => rows().includes('  c.txt'));
	assert.deepEqual(rows(), ['Documents', '  a.txt', '  c.txt', '  b.txt', 'Music', 'Pictures']);

	setDocs(['b.txt', 'a.txt', 'c.txt']);
	await waitFor('moved', () => rows()[1] === '  b.txt');
	assert.deepEqual(rows(), ['Documents', '  b.txt', '  a.txt', '  c.txt', 'Music', 'Pictures']);

	setDocs(['a.txt']);
	await waitFor('removed', () => !rows().includes('  b.txt'));
	assert.deepEqual(rows(), ['Documents', '  a.txt', 'Music', 'Pictures']);
});

test('selection is by id and survives moves', async () => {
	setDocs(['a.txt', 'b.txt']);
	await waitFor('docs', () => rows().includes('  b.txt'));
	await userInput(tree().id, ['docs/b.txt'], 'selection');
	assert.deepEqual(calls.selection.at(-1), ['docs/b.txt']);
	assert.deepEqual(tree().selection, ['docs/b.txt']);

	setDocs(['b.txt', 'a.txt']);
	await waitFor('moved', () => rows()[1] === '  b.txt');
	assert.deepEqual(tree().selection, ['docs/b.txt']);

	native.invoke(tree().id);
	await waitFor('invoke', () => callCount('invoke') === 1);
	assert.deepEqual(calls.invoke, ['docs/b.txt']);
});

test('expanding a lazy item shows the placeholder, then the loaded children', async () => {
	await userInput(item('music').id, true, 'musicExpand');
	await waitFor('placeholder shown', () => rows().includes('  Fetching…'));
	assert.equal(item('music').expanded, true);
	assert.equal((tree().rows as Row[]).find((row) => row.label === 'Fetching…')?.placeholder, true);

	setMusic(['Albums', 'Singles']);
	await waitFor('children loaded', () => rows().includes('  Singles'));
	assert.deepEqual(rows().slice(-4), ['Music', '  Albums', '  Singles', 'Pictures']);
	assert.equal(item('music').placeholder, null);
	// A lazy grandchild has its own (hidden) placeholder.
	assert.equal(item('music/Albums').placeholder, 'Loading…');
});

test('collapsing hides the children', async () => {
	await userInput(item('music').id, false, 'musicCollapse');
	await waitFor('collapsed', () => !rows().includes('  Albums'));
	assert.equal(item('music').expanded, false);
	assert.equal(item('music/Albums').visible, false);
});

test('a rejected expand snaps back', async () => {
	await userInput(item('pictures').id, true, 'picturesExpand');
	await waitFor('snapped back', () => item('pictures').expanded === false);
	assert.ok(!rows().includes('  Loading…'));
});

test('loading forces the placeholder next to existing children', async () => {
	setRefreshing(true);
	await waitFor('placeholder', () => rows()[1] === '  Loading…');
	assert.deepEqual(rows().slice(0, 4), ['Documents', '  Loading…', '  b.txt', '  a.txt']);
	setRefreshing(false);
	await waitFor('placeholder removed', () => rows()[1] === '  b.txt');
});

test('clearing hasChildren of an expanded item removes the placeholder', async () => {
	const emptyRows = () => (lists()[2].rows as Row[]).map((row) => row.label);
	assert.deepEqual(emptyRows(), ['Empty', 'Loading…']);
	setEmptyLoaded(true);
	await waitFor('placeholder removed', () => emptyRows().length === 1);
	assert.equal(item('empty').placeholder, null);
});
