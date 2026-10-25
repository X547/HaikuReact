// File browser: a folder tree that loads on demand, a column list of the
// selected folder with file-type icons, an image preview, a "recent" menu
// that loads when it opens and an icon-only menu in the menu bar.

import {homedir} from 'node:os';
import {basename, extname} from 'node:path';
import {useEffect, useState} from 'react';

import {
	ColumnListView,
	Group,
	Image,
	ListItem,
	ListView,
	Menu,
	MenuBar,
	MenuItem,
	quit,
	render,
	Row,
	Scroll,
	Separator,
	showAlert,
	Split,
	StringView,
	Window,
} from '../src/index.js';
import {imageExtensions, useFolders, type Entry, type Folders} from './lib/folders.js';

function FolderItem({entry, folders, load}: {
	entry: Entry;
	folders: Folders;
	load: (path: string) => void;
}) {
	const children = folders.get(entry.path)?.filter((child) => child.directory);
	return (
		<ListItem
			id={entry.path}
			label={entry.name}
			icon={{file: entry.path}}
			// Until the folder is read it may have subfolders; then only if it does.
			hasChildren={children === undefined || children.length > 0}
			onExpand={() => load(entry.path)}
		>
			{children?.map((child) => <FolderItem key={child.path} entry={child} folders={folders} load={load}/>)}
		</ListItem>
	);
}

function FileRow({entry, folders, load}: {
	entry: Entry;
	folders: Folders;
	load: (path: string) => void;
}) {
	const children = folders.get(entry.path);
	return (
		<Row
			id={entry.path}
			cells={{
				name: {text: entry.name, icon: {file: entry.path}},
				size: entry.directory ? '' : entry.size,
				modified: entry.modified,
			}}
			hasChildren={entry.directory && (children == null || children.length > 0)}
			onExpand={() => load(entry.path)}
		>
			{entry.directory && children?.map((child) => (
				<FileRow key={child.path} entry={child} folders={folders} load={load}/>
			))}
		</Row>
	);
}

function App() {
	const home = homedir();
	const [folders, load] = useFolders();
	const [folder, setFolder] = useState(home);
	const [file, setFile] = useState<string | null>(null);
	const [recent, setRecent] = useState<string[]>([]);
	// What the "Open recent" menu shows; null while it "loads".
	const [recentShown, setRecentShown] = useState<string[] | null>(null);
	const [status, setStatus] = useState('Expand folders on the left; double-click files on the right.');

	useEffect(() => load(home), [home, load]);
	useEffect(() => load(folder), [folder, load]);

	const open = (path: string) => {
		setRecent((current) => [path, ...current.filter((item) => item !== path)].slice(0, 8));
		setStatus(`Opened ${path}`);
	};
	const homeEntries = folders.get(home)?.filter((entry) => entry.directory);
	const entries = folders.get(folder);
	const preview = file !== null && imageExtensions.has(extname(file).toLowerCase()) ? file : null;

	return (
		<Window title="HaikuReact files" width={820} height={520} onCloseRequest={quit}>
			<MenuBar>
				<Menu icon={{app: 'application/x-vnd.Be-TRAK'}}>
					<MenuItem onClick={() => void showAlert({title: 'About', text: 'A HaikuReact example.'})}>
						About…
					</MenuItem>
					<Separator/>
					<MenuItem shortcut="Q" onClick={quit}>Quit</MenuItem>
				</Menu>
				<Menu label="File">
					<MenuItem
						shortcut="O"
						icon={{mimeType: 'text/plain'}}
						enabled={file !== null}
						onClick={() => file !== null && open(file)}
					>
						Open
					</MenuItem>
					<Menu
						label="Open recent"
						icon={{mimeType: 'application/x-vnd.Be-directory'}}
						onOpen={() => {
							// Pretend that the list takes a moment to load.
							setRecentShown(null);
							setTimeout(() => setRecentShown(recent), 400);
						}}
					>
						{recentShown?.length === 0 && <MenuItem enabled={false}>No recent files</MenuItem>}
						{recentShown?.map((path) => (
							<MenuItem key={path} icon={{file: path}} onClick={() => open(path)}>{basename(path)}</MenuItem>
						))}
					</Menu>
				</Menu>
				<Menu label="Go">
					<MenuItem icon={{file: home}} onClick={() => setFolder(home)}>Home</MenuItem>
					<MenuItem icon={{file: '/boot/system'}} onClick={() => setFolder('/boot/system')}>System</MenuItem>
				</Menu>
			</MenuBar>
			<Group insets="window">
				<Split weight={1}>
					<Scroll weight={1} minWidth={180}>
						<ListView
							selection={[folder]}
							onSelectionChange={([path]) => {
								if (path !== undefined) {
									setFolder(path);
									setFile(null);
								}
							}}
						>
							<ListItem id={home} label="Home" icon={{file: home}} defaultExpanded hasChildren>
								{homeEntries?.map((entry) => (
									<FolderItem key={entry.path} entry={entry} folders={folders} load={load}/>
								))}
							</ListItem>
						</ListView>
					</Scroll>
					<Group weight={3}>
						<ColumnListView
							columns={[
								{id: 'name', title: 'Name', width: 260},
								{id: 'size', title: 'Size', type: 'size', width: 90, align: 'right'},
								{id: 'modified', title: 'Modified', type: 'date', width: 170},
							]}
							defaultSort={[{column: 'name'}]}
							selection={file !== null ? [file] : []}
							onSelectionChange={([path]) => setFile(path ?? null)}
							onInvoke={open}
						>
							{entries === null && (
								<Row id="loading" cells={{name: 'Loading…'}}/>
							)}
							{entries?.map((entry) => <FileRow key={entry.path} entry={entry} folders={folders} load={load}/>)}
						</ColumnListView>
						{preview !== null && (
							<Image
								key={preview}
								src={{path: preview}}
								maxHeight={160}
								minWidth={0}
								minHeight={0}
								onError={(message) => setStatus(`Cannot show ${basename(preview)}: ${message}`)}
							/>
						)}
					</Group>
				</Split>
				<StringView>{status}</StringView>
			</Group>
		</Window>
	);
}

render(<App/>, {signature: 'application/x-vnd.HaikuReact-Files'});
