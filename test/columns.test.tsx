import assert from 'node:assert/strict';
import {after, test} from 'node:test';
import {useState} from 'react';

import {ColumnListView, render, Row, Window, type Column, type SortKey} from '../src/index.js';
import {native} from '../src/native.js';
import {findAll, findNode, waitFor} from './helpers.js';

const folder = {mimeType: 'application/x-vnd.Be-directory'};
const modified = new Date('2026-01-02T03:04:05Z');
const baseColumns: Column[] = [
	{id: 'name', title: 'Name', width: 160},
	{id: 'size', title: 'Size', type: 'size', align: 'right'},
	{id: 'count', title: 'Count', type: 'integer'},
	{id: 'modified', title: 'Modified', type: 'date', width: 140},
	{id: 'kind', title: '', type: 'icon', width: 24},
];

const calls: Record<string, unknown[]> = {};
let setSortState: (sort: SortKey[]) => void = () => {};
let setMusic: (music: string[] | null) => void = () => {};
let setColumns: (columns: Column[]) => void = () => {};

function record(name: string, value?: unknown): void {
	(calls[name] ??= []).push(value);
}

function App() {
	const [sort, setSort] = useState<SortKey[]>([]);
	const [selection, setSelection] = useState<string[]>([]);
	const [music, setMusicList] = useState<string[] | null>(null);
	const [musicExpanded, setMusicExpanded] = useState(false);
	const [columns, setColumnList] = useState(baseColumns);
	setSortState = setSort;
	setMusic = setMusicList;
	setColumns = setColumnList;

	return (
		<Window title="Columns test" width={600} height={300}>
			<ColumnListView
				columns={columns}
				sort={sort}
				onSortChange={(next) => {
					record('sort', next);
					setSort(next);
				}}
				selection={selection}
				onSelectionChange={(ids) => {
					record('selection', ids);
					setSelection(ids);
				}}
				onInvoke={(id) => record('invoke', id)}
			>
				<Row id="docs" cells={{name: 'Documents', size: 1234, count: 3, kind: {mimeType: 'text/plain'}}} defaultExpanded>
					<Row id="docs/a" cells={{name: 'a.txt', size: 10}}/>
					<Row id="docs/b" cells={{name: 'b.txt', size: 20}}/>
				</Row>
				<Row
					id="music"
					cells={{name: {text: 'Music', icon: folder}, modified}}
					hasChildren
					expanded={musicExpanded}
					onExpand={() => {
						record('musicExpand');
						setMusicExpanded(true);
					}}
				>
					{music?.map((name) => <Row key={name} id={`music/${name}`} cells={{name}}/>)}
				</Row>
				<Row id="notes" cells={{name: 'notes', size: 500}}/>
			</ColumnListView>
		</Window>
	);
}

const root = render(<App/>, {signature: 'application/x-vnd.HaikuReact-ColumnsTest'});
after(() => root.quit());

interface RowDump {
	id: string | null;
	level: number;
	placeholder: boolean;
	cells: Record<string, unknown>;
}

const dump = () => root.dump();
const list = () => findAll(dump(), (node) => node.type === 'ColumnListView')[0];
const rows = () => list().rows as RowDump[];
const names = () => rows().map((row) => {
	const name = row.cells.name;
	const text = typeof name === 'object' && name !== null ? (name as {text: string}).text : name;
	return `${'  '.repeat(row.level)}${text as string}`;
});
const row = (id: string) => findNode(dump(), (node) => node.type === 'Row' && node.itemId === id)!;
const callCount = (name: string) => calls[name]?.length ?? 0;

async function userInput(id: number, value: boolean | string | readonly string[], handler: string) {
	const before = callCount(handler);
	native.input(id, value);
	await waitFor(`${handler} handler`, () => callCount(handler) > before);
}

test('window is shown', async () => {
	await waitFor('window shown', () => dump().length === 1 && dump()[0].hidden === false);
	assert.deepEqual((list().columns as {id: string}[]).map((column) => column.id), baseColumns.map((column) => column.id));
});

test('cells show values per column type', () => {
	const byId = Object.fromEntries(rows().map((item) => [item.id, item.cells]));
	assert.deepEqual(byId.docs, {name: 'Documents', size: 1234, count: 3, modified: null, kind: true});
	assert.deepEqual(byId.music, {
		name: {text: 'Music', hasIcon: true},
		size: null,
		count: null,
		modified: modified.getTime(),
		kind: null,
	});
});

test('nested rows show under expanded parents in child order', () => {
	assert.deepEqual(names(), ['Documents', '  a.txt', '  b.txt', 'Music', 'notes']);
	assert.equal(row('music').placeholder, 'Loading…');
	assert.equal(row('music').expanded, false);
});

test('expanding a lazy row shows the placeholder, then the loaded rows', async () => {
	await userInput(row('music').id, true, 'musicExpand');
	await waitFor('placeholder shown', () => names().includes('  Loading…'));
	assert.equal(rows().find((item) => item.placeholder)?.level, 1);

	setMusic(['Albums', 'Singles']);
	await waitFor('rows loaded', () => names().includes('  Singles'));
	assert.deepEqual(names(), ['Documents', '  a.txt', '  b.txt', 'Music', '  Albums', '  Singles', 'notes']);
	assert.equal(row('music').placeholder, null);
});

test('the sort prop orders rows, at every level', async () => {
	setSortState([{column: 'size', direction: 'descending'}]);
	await waitFor('sorted', () => names()[0] === 'Documents' && names().at(-1) !== 'notes');
	assert.deepEqual(list().sort, [{column: 'size', direction: 'descending'}]);
	assert.deepEqual(names().filter((name) => !name.startsWith(' ')), ['Documents', 'notes', 'Music']);
	assert.deepEqual(names().slice(1, 3), ['  b.txt', '  a.txt']);
});

test('sorting by the user is reported', async () => {
	await userInput(list().id, 'name', 'sort');
	assert.deepEqual(calls.sort.at(-1), [{column: 'name', direction: 'ascending'}]);
	await waitFor('sorted by name', () => names()[0] === 'Documents' && names().includes('Music'));
	assert.deepEqual(names().filter((name) => !name.startsWith(' ')), ['Documents', 'Music', 'notes']);

	await userInput(list().id, 'name', 'sort');
	assert.deepEqual(calls.sort.at(-1), [{column: 'name', direction: 'descending'}]);
	await waitFor('reversed', () => names()[0] === 'notes');
});

test('clearing the sort restores child order', async () => {
	setSortState([]);
	await waitFor('child order', () => names()[0] === 'Documents' && names().at(-1) === 'notes');
	assert.deepEqual(list().sort, []);
	assert.deepEqual(names(), ['Documents', '  a.txt', '  b.txt', 'Music', '  Albums', '  Singles', 'notes']);
});

test('selection and invocation are by id', async () => {
	await userInput(list().id, ['notes'], 'selection');
	assert.deepEqual(calls.selection.at(-1), ['notes']);
	assert.deepEqual(list().selection, ['notes']);
	native.invoke(list().id);
	await waitFor('invoke', () => callCount('invoke') === 1);
	assert.deepEqual(calls.invoke, ['notes']);
});

test('changing the columns rebuilds the rows', async () => {
	setColumns([{id: 'size', title: 'Size', type: 'size'}, {id: 'name', title: 'Name'}]);
	await waitFor('new columns', () => (list().columns as unknown[]).length === 2);
	assert.deepEqual(rows()[0].cells, {size: 1234, name: 'Documents'});
	assert.deepEqual(names(), ['Documents', '  a.txt', '  b.txt', 'Music', '  Albums', '  Singles', 'notes']);
	assert.deepEqual(list().selection, ['notes']);
});
