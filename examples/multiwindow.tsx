import {useState} from 'react';

import {
	Button,
	Glue,
	Group,
	ListView,
	Menu,
	MenuBar,
	MenuItem,
	quit,
	render,
	Scroll,
	showAlert,
	StringView,
	TextView,
	Window,
} from '../src/index.js';

interface Document {
	id: number;
	name: string;
	text: string;
	saved: string;
}

let nextId = 1;
// Quit with confirmation; set by App, used for quit requests from outside (Command+Q, Deskbar).
let requestQuit: () => void = quit;

function DocumentWindow({document, offset, onChange, onSave, onClose}: {
	document: Document;
	offset: number;
	onChange: (text: string) => void;
	onSave: () => void;
	onClose: () => void;
}) {
	const modified = document.text !== document.saved;

	const requestClose = async () => {
		if (!modified) {
			onClose();
			return;
		}
		const choice = await showAlert({
			title: 'Close document',
			text: `Save changes to "${document.name}" before closing?`,
			buttons: ['Cancel', "Don't save", 'Save'],
			type: 'warning',
		});
		if (choice === 2) {
			onSave();
		}
		if (choice > 0) {
			onClose();
		}
	};

	return (
		<Window
			title={`${document.name}${modified ? ' *' : ''}`}
			left={220 + offset * 30}
			top={120 + offset * 30}
			width={360}
			height={240}
			onCloseRequest={requestClose}
		>
			<MenuBar>
				<Menu label="Document">
					<MenuItem shortcut="S" enabled={modified} onClick={onSave}>Save</MenuItem>
					<MenuItem shortcut="W" onClick={requestClose}>Close</MenuItem>
				</Menu>
			</MenuBar>
			<Group insets="window">
				<Scroll>
					<TextView text={document.text} onChange={onChange}/>
				</Scroll>
			</Group>
		</Window>
	);
}

function App() {
	const [documents, setDocuments] = useState<Document[]>([]);
	const [selection, setSelection] = useState<number[]>([]);
	const [paletteVisible, setPaletteVisible] = useState(true);
	const [lastAction, setLastAction] = useState('Open a document to start.');

	const open = () => {
		const id = nextId++;
		setDocuments((current) => [...current, {id, name: `Untitled ${id}`, text: '', saved: ''}]);
		setLastAction(`Opened Untitled ${id}`);
	};
	const update = (id: number, change: Partial<Document>) =>
		setDocuments((current) => current.map((item) => (item.id === id ? {...item, ...change} : item)));
	const close = (id: number) => setDocuments((current) => current.filter((item) => item.id !== id));

	const quitApp = async () => {
		const unsaved = documents.filter((item) => item.text !== item.saved).length;
		if (unsaved === 0) {
			quit();
			return;
		}
		const choice = await showAlert({
			title: 'Quit',
			text: `${unsaved} document(s) have unsaved changes. Quit anyway?`,
			buttons: ['Cancel', 'Quit'],
			type: 'stop',
		});
		if (choice === 1) {
			quit();
		}
	};
	requestQuit = () => void quitApp();

	return (
		<>
			<Window title="Documents" left={40} top={120} width={180} height={260} onCloseRequest={quitApp}>
				<MenuBar>
					<Menu label="App">
						<MenuItem shortcut="N" onClick={open}>New document</MenuItem>
						<MenuItem marked={paletteVisible} onClick={() => setPaletteVisible(!paletteVisible)}>Show palette</MenuItem>
						<MenuItem onClick={() => showAlert({title: 'About', text: 'HaikuReact multi-window example'})}>
							About…
						</MenuItem>
					</Menu>
				</MenuBar>
				<Group insets="window">
					<Scroll>
						<ListView
							items={documents.map((item) => item.name + (item.text !== item.saved ? ' *' : ''))}
							selection={selection}
							onSelectionChange={setSelection}
						/>
					</Scroll>
					<Group orientation="horizontal">
						<Button onClick={open}>New</Button>
						<Glue/>
						<Button onClick={quitApp}>Quit</Button>
					</Group>
				</Group>
			</Window>
			{documents.map((document, index) => (
				<DocumentWindow
					key={document.id}
					document={document}
					offset={index}
					onChange={(text) => update(document.id, {text})}
					onSave={() => {
						update(document.id, {saved: document.text});
						setLastAction(`Saved ${document.name}`);
					}}
					onClose={() => {
						close(document.id);
						setLastAction(`Closed ${document.name}`);
					}}
				/>
			))}
			<Window
				title="Status"
				look="floating"
				feel="floating-app"
				resizable={false}
				closeOnEscape
				visible={paletteVisible}
				left={40}
				top={420}
				onCloseRequest={() => setPaletteVisible(false)}
			>
				<Group insets="small">
					<StringView>{`${documents.length} open`}</StringView>
					<StringView>{lastAction}</StringView>
				</Group>
			</Window>
		</>
	);
}

render(<App/>, {
	signature: 'application/x-vnd.HaikuReact-MultiWindow',
	onQuitRequest: () => {
		requestQuit();
		return false;
	},
});
