import assert from 'node:assert/strict';
import {after, test} from 'node:test';
import {Component, useState, type ReactNode} from 'react';

import {Button, Group, render, showAlert, StringView, Window} from '../src/index.js';
import {native} from '../src/native.js';
import {findNode, frameOf, waitFor} from './helpers.js';

type Broken = 'none' | 'prop' | 'nesting';

interface Controls {
	open(name: string): void;
	setMainFrame(frame: {width?: number; left?: number}): void;
	setBroken(broken: Broken): void;
}

let controls: Controls;
const clicks: string[] = [];
const windowEvents: [string, string, unknown][] = [];
let quitRequests = 0;

class ErrorBoundary extends Component<{children: ReactNode}, {error: Error | null}> {
	state = {error: null as Error | null};

	static getDerivedStateFromError(error: Error) {
		return {error};
	}

	render() {
		if (this.state.error !== null) {
			return <StringView>{`Failed: ${this.state.error.message}`}</StringView>;
		}
		return this.props.children;
	}
}

function BrokenContent({broken}: {broken: Broken}) {
	switch (broken) {
		case 'prop':
			return <Button enabled={'yes' as unknown as boolean}>Bad prop</Button>;
		case 'nesting':
			return <Group><Window title="Nested"/></Group>;
		default:
			return <StringView>Content OK</StringView>;
	}
}

function App() {
	const [documents, setDocuments] = useState(['One']);
	const [mainFrame, setMainFrame] = useState({width: 400, left: 100});
	const [broken, setBroken] = useState<Broken>('none');
	controls = {
		open: (name) => setDocuments((current) => [...current, name]),
		setMainFrame: (frame) => setMainFrame((current) => ({...current, ...frame})),
		setBroken,
	};
	const log = (window: string, event: string) => (value: unknown) => windowEvents.push([window, event, value]);

	return (
		<>
			<Window
				title="Main"
				width={mainFrame.width}
				left={mainFrame.left}
				top={100}
				onMove={log('Main', 'move')}
				onResize={log('Main', 'resize')}
			>
				<Group insets="window">
					<StringView>{`${documents.length} documents`}</StringView>
					<ErrorBoundary key={broken}>
						<BrokenContent broken={broken}/>
					</ErrorBoundary>
				</Group>
			</Window>
			{documents.map((name) => (
				<Window
					key={name}
					title={name}
					onActivate={log(name, 'activate')}
					onCloseRequest={() => setDocuments((current) => current.filter((item) => item !== name))}
				>
					<Group insets="window">
						<Button onClick={() => clicks.push(name)}>{`Click ${name}`}</Button>
					</Group>
				</Window>
			))}
			<Window title="Palette" look="floating" feel="floating-app" resizable={false} closeOnEscape onCloseRequest={() => {}}>
				<Group insets="small"><StringView>Tools</StringView></Group>
			</Window>
		</>
	);
}

let allowQuit = false;
const caughtErrors: unknown[] = [];
const root = render(<App/>, {
	signature: 'application/x-vnd.HaikuReact-WindowsTest',
	strictMode: true,
	onCaughtError: (error) => caughtErrors.push(error),
	onQuitRequest: () => {
		quitRequests++;
		return allowQuit;
	},
});
after(() => root.quit());

const dump = () => root.dump();
const windowTitled = (title: string) => dump().find((window) => window.title === title);
const titles = () => dump().map((window) => window.title).sort();
const byText = (text: string) => findNode(dump(), (node) => node.text === text);

test('several windows are shown at once', async () => {
	await waitFor('windows shown', () => dump().length === 3 && dump().every((window) => window.hidden === false));
	assert.deepEqual(titles(), ['Main', 'One', 'Palette']);
});

test('events go to the window they come from', async () => {
	native.invoke(findNode(dump(), (node) => node.label === 'Click One')!.id);
	await waitFor('click', () => clicks.length === 1);
	assert.deepEqual(clicks, ['One']);
});

test('windows open and close independently', async () => {
	controls.open('Two');
	await waitFor('Two shown', () => windowTitled('Two')?.hidden === false);
	await waitFor('Two activated', () => windowEvents.some(([window, event, value]) => window === 'Two' && event === 'activate' && value === true));

	native.invoke(windowTitled('Two')!.id);
	await waitFor('Two closed', () => windowTitled('Two') === undefined);
	assert.deepEqual(titles(), ['Main', 'One', 'Palette']);
	assert.ok(byText('1 documents'));
});

test('look, feel and behavior flags', () => {
	const palette = windowTitled('Palette')!;
	assert.equal(palette.look, 'floating');
	assert.equal(palette.feel, 'floating-app');
	assert.equal(palette.resizable, false);
	assert.equal(palette.closeOnEscape, true);
	const main = windowTitled('Main')!;
	assert.equal(main.look, 'titled');
	assert.equal(main.resizable, true);
});

test('frame props move and resize the window, which reports it', async () => {
	controls.setMainFrame({width: 500});
	await waitFor('resize event', () => windowEvents.some(([window, event, value]) =>
		window === 'Main' && event === 'resize' && (value as {width: number}).width === 500));
	assert.equal(frameOf(windowTitled('Main')).width, 500);

	controls.setMainFrame({left: 150});
	await waitFor('move event', () => windowEvents.some(([window, event, value]) =>
		window === 'Main' && event === 'move' && (value as {x: number}).x === 150));
	assert.equal(frameOf(windowTitled('Main')).x, 150);
});

test('a quit request from outside can be refused', async () => {
	native.postQuitRequest();
	await waitFor('quit request', () => quitRequests === 1);
	assert.equal(dump().length, 3);
});

test('showAlert resolves with the chosen button', async () => {
	const answer = showAlert({title: 'Question', text: 'Proceed?', buttons: ['Cancel', 'Proceed'], type: 'warning'});
	const [alertId] = await waitFor('alert open', () => native.stats().alerts.length === 1 && native.stats().alerts);
	native.pressAlertButton(alertId, 1);
	assert.equal(await answer, 1);
	await waitFor('alert closed', () => native.stats().alerts.length === 0);
});

test('error boundaries catch invalid props reported by native code', async () => {
	assert.ok(byText('Content OK'));
	controls.setBroken('prop');
	const fallback = await waitFor('fallback', () => findNode(dump(), (node) => String(node.text ?? '').startsWith('Failed:')));
	assert.match(String(fallback.text), /enabled.*boolean/);
});

test('error boundaries catch invalid nesting', async () => {
	controls.setBroken('nesting');
	const fallback = await waitFor('fallback', () => findNode(dump(), (node) => String(node.text ?? '').includes('<Window>')));
	assert.match(String(fallback.text), /<Window> cannot be placed in a layout/);
	assert.equal(caughtErrors.length, 2, 'both errors reported to onCaughtError');

	controls.setBroken('none');
	await waitFor('content restored', () => byText('Content OK'));
});
