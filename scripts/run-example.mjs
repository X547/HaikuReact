// Usage: npm run example <name>   (runs dist/examples/<name>.js)

import {readdirSync} from 'node:fs';
import {dirname, join} from 'node:path';
import {fileURLToPath, pathToFileURL} from 'node:url';

const examplesDir = join(dirname(fileURLToPath(import.meta.url)), '..', 'dist', 'examples');
const available = readdirSync(examplesDir)
	.filter((file) => file.endsWith('.js'))
	.map((file) => file.slice(0, -'.js'.length));

const name = process.argv[2];
if (name === undefined || !available.includes(name)) {
	console.error(`usage: npm run example <name>\navailable: ${available.join(', ')}`);
	process.exit(name === undefined ? 0 : 1);
}
await import(pathToFileURL(join(examplesDir, `${name}.js`)).href);
