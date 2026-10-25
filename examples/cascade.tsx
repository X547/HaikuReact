// File system in cascaded menus, like Tracker's: every folder is a submenu
// that reads its contents when it opens, showing "Loading…" the first time.
// Choosing a file (or "Open" in a folder's menu) shows it below.

import {homedir} from 'node:os';
import {basename, extname} from 'node:path';
import {useState} from 'react';

import {
	Glue,
	Group,
	Icon,
	Image,
	Menu,
	MenuBar,
	MenuItem,
	quit,
	render,
	Separator,
	StringView,
	Window,
} from '../src/index.js';
import {imageExtensions, useFolders, type Entry, type Folders} from './lib/folders.js';

// Long menus get slow and taller than the screen.
const maxItems = 60;

interface FolderMenuProps {
	path: string;
	label: string;
	folders: Folders;
	load: (path: string, refresh?: boolean) => void;
	onChoose: (entry: Entry | string) => void;
}

function FolderMenu({path, label, folders, load, onChoose}: FolderMenuProps) {
	const entries = folders.get(path);
	return (
		// Read again on every open; until the first read finishes the menu has
		// no items and shows "Loading…".
		<Menu label={label} icon={{file: path}} onOpen={() => load(path, true)}>
			{entries != null && (
				<MenuItem icon={{file: path}} onClick={() => onChoose(path)}>{`Open “${label}”`}</MenuItem>
			)}
			{entries != null && <Separator/>}
			{entries?.length === 0 && <MenuItem enabled={false}>Empty folder</MenuItem>}
			{entries?.slice(0, maxItems).map((entry) => entry.directory
				? <FolderMenu key={entry.path} path={entry.path} label={entry.name} folders={folders} load={load} onChoose={onChoose}/>
				: (
					<MenuItem key={entry.path} icon={{file: entry.path}} onClick={() => onChoose(entry)}>
						{entry.name}
					</MenuItem>
				))}
			{entries != null && entries.length > maxItems && (
				<MenuItem enabled={false}>{`… ${entries.length - maxItems} more`}</MenuItem>
			)}
		</Menu>
	);
}

function formatSize(bytes: number): string {
	const units = ['bytes', 'KiB', 'MiB', 'GiB'];
	let size = bytes;
	let unit = 0;
	while (size >= 1024 && unit < units.length - 1) {
		size /= 1024;
		unit++;
	}
	return unit === 0 ? `${bytes} bytes` : `${size.toFixed(1)} ${units[unit]}`;
}

function App() {
	const [folders, load] = useFolders();
	const [chosen, setChosen] = useState<Entry | string | null>(null);
	const roots = [
		{path: homedir(), label: 'Home'},
		{path: '/boot/system', label: 'System'},
		{path: '/boot', label: 'Disk'},
	];

	const path = typeof chosen === 'string' ? chosen : chosen?.path;
	const preview = chosen !== null && typeof chosen !== 'string' && imageExtensions.has(extname(chosen.path).toLowerCase())
		? chosen.path
		: null;

	return (
		<Window title="HaikuReact cascade" width={520} height={360} onCloseRequest={quit}>
			<MenuBar>
				{roots.map((root) => (
					<FolderMenu key={root.path} {...root} folders={folders} load={load} onChoose={setChosen}/>
				))}
				<MenuItem onClick={quit}>Quit</MenuItem>
			</MenuBar>
			<Group insets="window">
				{path === undefined ? (
					<StringView>Browse the menus above; folders open as submenus.</StringView>
				) : (
					<Group orientation="horizontal" verticalAlignment="top">
						<Icon source={{file: path}} size={64}/>
						<Group spacing="half">
							<StringView>{basename(path) || path}</StringView>
							<StringView>{path}</StringView>
							{typeof chosen !== 'string' && chosen !== null && (
								<StringView>{`${formatSize(chosen.size)} · modified ${chosen.modified.toLocaleString()}`}</StringView>
							)}
						</Group>
						<Glue/>
					</Group>
				)}
				{preview !== null && (
					<Image key={preview} src={{path: preview}} maxHeight={220} minWidth={0} minHeight={0}/>
				)}
				<Glue/>
			</Group>
		</Window>
	);
}

render(<App/>, {signature: 'application/x-vnd.HaikuReact-Cascade'});
