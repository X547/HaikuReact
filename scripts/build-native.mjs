// Builds the native add-on with Meson into build/. Runs on `npm install`.
// Elsewhere than on Haiku there is nothing to build; installing still works
// there, e.g. for editing and type checking.

import {spawnSync} from 'node:child_process';
import {existsSync} from 'node:fs';
import {dirname, join} from 'node:path';
import {fileURLToPath} from 'node:url';

const root = join(dirname(fileURLToPath(import.meta.url)), '..');
const buildDir = join(root, 'build');

function run(command, args) {
	const result = spawnSync(command, args, {cwd: root, stdio: 'inherit'});
	if (result.error !== undefined) {
		console.error(`haiku-react: cannot run ${command}: ${result.error.message}`);
		console.error('haiku-react: building needs meson, ninja and a C++ compiler (pkgman install meson ninja)');
		process.exit(1);
	}
	if (result.status !== 0) {
		process.exit(result.status ?? 1);
	}
}

if (process.platform !== 'haiku') {
	console.warn(`haiku-react: the native add-on only builds on Haiku; skipped on ${process.platform}`);
	process.exit(0);
}

const configured = existsSync(join(buildDir, 'build.ninja'));
run('meson', ['setup', buildDir, root, ...(configured ? ['--reconfigure'] : [])]);
run('meson', ['compile', '-C', buildDir]);
