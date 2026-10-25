import assert from 'node:assert/strict';
import {mkdtempSync, writeFileSync} from 'node:fs';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {after, test} from 'node:test';
import {useState} from 'react';

import {Group, Icon, Image, render, Window, type IconSource, type ImageFit} from '../src/index.js';
import {native} from '../src/native.js';
import {findAll, findNode, frameOf, waitFor} from './helpers.js';
import {makePng} from './png.js';

const directory = mkdtempSync(join(tmpdir(), 'haiku-react-images-'));
const pngPath = join(directory, 'red.png');
writeFileSync(pngPath, makePng(24, 12, [255, 0, 0, 255]));
const pngData = makePng(40, 20, [0, 0, 255, 255]);

const icons: Record<string, IconSource> = {
	system: {system: 'dialog-warning'},
	mimeType: {mimeType: 'text/plain'},
	folder: {mimeType: 'application/x-vnd.Be-directory'},
	unknownSubtype: {mimeType: 'text/x-haiku-react-made-up'},
	file: {file: '/boot/home'},
	app: {app: 'application/x-vnd.Haiku-StyledEdit'},
	path: {path: pngPath},
	data: {data: pngData},
	unknownSystem: {system: 'no-such-icon'},
	missingFile: {file: '/boot/no/such/file'},
};

const fits: ImageFit[] = ['contain', 'cover', 'fill', 'none'];
const events: {load: unknown[]; error: unknown[]} = {load: [], error: []};
let showIcons: (show: boolean) => void = () => {};
let setBigIcon: (size: number) => void = () => {};

function App() {
	const [iconsShown, setIconsShown] = useState(true);
	const [bigSize, setBigSize] = useState(32);
	showIcons = setIconsShown;
	setBigIcon = setBigSize;
	return (
		<Window title="Images test">
			<Group insets="window" orientation="horizontal">
				{iconsShown && (
					<Group>
						{Object.entries(icons).map(([name, source]) => <Icon key={name} source={source}/>)}
						<Icon key="big" source={icons.system} size={bigSize}/>
					</Group>
				)}
				<Group>
					<Image src={{data: pngData}} onLoad={(size) => events.load.push(size)}/>
					<Image src={{path: '/boot/no/such/image.png'}} onError={(message) => events.error.push(message)}/>
					{fits.map((fit) => (
						<Image
							key={fit}
							src={{data: pngData}}
							fit={fit}
							minWidth={100}
							maxWidth={100}
							minHeight={100}
							maxHeight={100}
						/>
					))}
				</Group>
			</Group>
		</Window>
	);
}

const root = render(<App/>, {signature: 'application/x-vnd.HaikuReact-ImagesTest'});
after(() => root.quit());

const dump = () => root.dump();
const iconNodes = () => findAll(dump(), (node) => node.type === 'Icon');
const imageNodes = () => findAll(dump(), (node) => node.type === 'Image');

interface BitmapDump {
	width: number;
	height: number;
	opaquePixels: number;
}

test('window is shown', async () => {
	await waitFor('window shown', () => dump().length === 1 && dump()[0].hidden === false);
});

test('icons load from every source type at the requested size', () => {
	const nodes = iconNodes();
	const names = Object.keys(icons);
	for (const name of ['system', 'mimeType', 'folder', 'unknownSubtype', 'file', 'app', 'path']) {
		const node = nodes[names.indexOf(name)];
		const bitmap = node.bitmap as BitmapDump | null;
		assert.ok(bitmap !== null, `${name}: ${node.error as string}`);
		assert.ok(bitmap.opaquePixels > 0, `${name} has visible pixels`);
		assert.equal(frameOf(node).width, node.size, `${name} view width`);
		assert.equal(frameOf(node).height, node.size, `${name} view height`);
		if (name !== 'path') {
			assert.equal(bitmap.width, node.size, `${name} bitmap width`);
		}
	}
	// A raster file keeps its own size and is scaled when drawn.
	assert.deepEqual(nodes[names.indexOf('path')].bitmap, {width: 24, height: 12, opaquePixels: 24 * 12});
	assert.deepEqual(nodes[names.indexOf('data')].bitmap, {width: 40, height: 20, opaquePixels: 40 * 20});
});

test('sources without an icon show nothing', () => {
	const names = Object.keys(icons);
	for (const name of ['unknownSystem', 'missingFile']) {
		const node = iconNodes()[names.indexOf(name)];
		assert.equal(node.bitmap, null);
		assert.notEqual(node.error, '');
	}
});

test('size prop sets the icon size and identical icons share a bitmap', async () => {
	const big = () => iconNodes().at(-1)!;
	assert.ok((big().size as number) >= 32);
	assert.equal(frameOf(big()).width, big().size);
	const before = native.stats().bitmaps;
	setBigIcon(48);
	await waitFor('icon resized', () => frameOf(big()).width === big().size && (big().size as number) >= 48);
	assert.equal((big().bitmap as BitmapDump).width, big().size);
	// The 32 pixel warning icon is gone, the 48 pixel one is new.
	assert.equal(native.stats().bitmaps, before);
});

test('images report onLoad with their size', async () => {
	await waitFor('load', () => events.load.length === 1);
	assert.deepEqual(events.load, [{width: 40, height: 20}]);
	// The window grows to the image's natural size, its minimum; the column
	// is wider, and 'contain' centers the image in the extra width.
	await waitFor('image laid out', () => frameOf(imageNodes()[0]).height === 20);
	const image = imageNodes()[0];
	assert.equal(image.loading, false);
	assert.deepEqual(image.bitmap, {width: 40, height: 20, opaquePixels: 800});
	assert.deepEqual(image.drawRect, {x: (frameOf(image).width - 40) / 2, y: 0, width: 40, height: 20});
});

test('images report onError', async () => {
	await waitFor('error', () => events.error.length === 1);
	assert.match(events.error[0] as string, /no\/such\/image\.png/);
	assert.equal(imageNodes()[1].bitmap, null);
});

test('fit modes place the image in the view', async () => {
	await waitFor('fit images loaded', () => imageNodes().slice(2).every((node) => node.bitmap !== null));
	const rects = Object.fromEntries(imageNodes().slice(2).map((node, i) => {
		assert.equal(frameOf(node).width, 100);
		assert.equal(frameOf(node).height, 100);
		return [fits[i], node.drawRect];
	}));
	assert.deepEqual(rects, {
		contain: {x: 0, y: 25, width: 100, height: 50},
		cover: {x: -50, y: 0, width: 200, height: 100},
		fill: {x: 0, y: 0, width: 100, height: 100},
		none: {x: 30, y: 40, width: 40, height: 20},
	});
});

test('unused icons are released', async () => {
	assert.ok(native.stats().bitmaps > 0);
	showIcons(false);
	await waitFor('icons removed', () => iconNodes().length === 0);
	assert.equal(native.stats().bitmaps, 0);
	assert.equal(findNode(dump(), (node) => node.type === 'Icon'), undefined);
});
