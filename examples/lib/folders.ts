// Reading folders for the examples.

import {readdir, stat} from 'node:fs/promises';
import {join} from 'node:path';
import {useCallback, useState} from 'react';

export interface Entry {
	path: string;
	name: string;
	directory: boolean;
	size: number;
	modified: Date;
}

export const imageExtensions: ReadonlySet<string> = new Set(['.png', '.jpg', '.jpeg', '.gif', '.bmp', '.webp']);

/** Visible entries of a folder, folders first, then by name. */
export async function listFolder(path: string): Promise<Entry[]> {
	const names = (await readdir(path)).filter((name) => !name.startsWith('.'));
	const entries = await Promise.all(names.map(async (name): Promise<Entry | null> => {
		const entryPath = join(path, name);
		try {
			const info = await stat(entryPath);
			return {path: entryPath, name, directory: info.isDirectory(), size: info.size, modified: info.mtime};
		} catch {
			return null;
		}
	}));
	return entries
		.filter((entry): entry is Entry => entry !== null)
		.sort((a, b) => Number(b.directory) - Number(a.directory) || a.name.localeCompare(b.name));
}

export type Folders = ReadonlyMap<string, Entry[] | null>;

/**
 * Folder contents by path; null while a folder is read for the first time.
 * load() reads a folder once, or again with refresh (keeping the old
 * contents until the new ones are read).
 */
export function useFolders(): readonly [Folders, (path: string, refresh?: boolean) => void] {
	const [folders, setFolders] = useState<Folders>(new Map());
	const load = useCallback((path: string, refresh = false) => {
		setFolders((current) => {
			if (current.has(path) && !refresh) {
				return current;
			}
			listFolder(path).then(
				(entries) => setFolders((latest) => new Map(latest).set(path, entries)),
				() => setFolders((latest) => new Map(latest).set(path, [])),
			);
			return current.has(path) ? current : new Map(current).set(path, null);
		});
	}, []);
	return [folders, load] as const;
}
