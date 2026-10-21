import {useState} from 'react';

import {
	Glue,
	Group,
	Menu,
	MenuBar,
	MenuItem,
	quit,
	render,
	Scroll,
	Separator,
	StringView,
	TextView,
	Window,
} from '../src/index.js';

const sizes = ['Small', 'Medium', 'Large'];

function App() {
	const [text, setText] = useState('Edit me, then try the menus and their shortcuts.\n');
	const [saved, setSaved] = useState(text);
	const [wrap, setWrap] = useState(true);
	const [size, setSize] = useState('Medium');
	const [recent, setRecent] = useState<string[]>([]);
	const [message, setMessage] = useState('Ready');

	const save = () => {
		setSaved(text);
		setRecent([`Snapshot ${recent.length + 1}`, ...recent].slice(0, 5));
		setMessage('Saved');
	};

	return (
		<Window title="HaikuReact menus" width={480} height={320} onCloseRequest={quit}>
			<MenuBar>
				<Menu label="File">
					<MenuItem shortcut="N" onClick={() => {
						setText('');
						setMessage('New document');
					}}>New</MenuItem>
					<MenuItem shortcut="S" enabled={text !== saved} onClick={save}>Save</MenuItem>
					<Menu label="Open recent" enabled={recent.length > 0}>
						{recent.map((name) => (
							<MenuItem key={name} onClick={() => setMessage(`Opened ${name}`)}>{name}</MenuItem>
						))}
					</Menu>
					<Separator/>
					<MenuItem shortcut="Q" onClick={quit}>Quit</MenuItem>
				</Menu>
				<Menu label="Edit">
					<MenuItem shortcut="K" modifiers={['shift']} onClick={() => {
						setText(text.toUpperCase());
						setMessage('Converted to upper case');
					}}>Upper case</MenuItem>
					<MenuItem onClick={() => setText(text.toLowerCase())}>Lower case</MenuItem>
				</Menu>
				<Menu label="View">
					<MenuItem marked={wrap} shortcut="W" modifiers={['shift']} onClick={() => setWrap(!wrap)}>
						Word wrap
					</MenuItem>
					<Separator/>
					<Menu label="Text size" radioMode>
						{sizes.map((item) => (
							<MenuItem key={item} marked={size === item} onClick={() => setSize(item)}>{item}</MenuItem>
						))}
					</Menu>
				</Menu>
			</MenuBar>
			<Group insets="window">
				<Scroll horizontal={!wrap}>
					<TextView text={text} wordWrap={wrap} onChange={setText}/>
				</Scroll>
				<Group orientation="horizontal">
					<StringView>{message}</StringView>
					<Glue/>
					<StringView>{`${text.length} characters · ${size}${text !== saved ? ' · modified' : ''}`}</StringView>
				</Group>
			</Group>
		</Window>
	);
}

render(<App/>, {signature: 'application/x-vnd.HaikuReact-Menus'});
