// Prints a directory the native build needs: `headers` (node_api.h of the
// running node) or `addon-api` (node-addon-api, wherever npm installed it).
// Usage: node-build-paths.cjs headers|addon-api <project root>
// Directories inside the project root are printed relative to it, since
// Meson rejects absolute include paths into the source tree.

const fs = require('node:fs');
const path = require('node:path');

function nodeHeaders() {
	// Haiku packages install headers to develop/headers/node, other systems to include/node.
	const prefix = path.dirname(path.dirname(process.execPath));
	const candidates = ['develop/headers/node', 'include/node'].map((dir) => path.join(prefix, dir));
	const found = candidates.find((dir) => fs.existsSync(path.join(dir, 'node_api.h')));
	if (found === undefined) {
		throw new Error(`node_api.h not found in: ${candidates.join(', ')}`);
	}
	return found;
}

function addonApi() {
	return path.dirname(require.resolve('node-addon-api/package.json'));
}

const [which, root] = process.argv.slice(2);
const resolvers = {headers: nodeHeaders, 'addon-api': addonApi};
if (!(which in resolvers) || root === undefined) {
	console.error('usage: node-build-paths.cjs headers|addon-api <project root>');
	process.exit(2);
}
try {
	const dir = resolvers[which]();
	const relative = path.relative(root, dir);
	const inside = relative !== '' && !relative.startsWith('..') && !path.isAbsolute(relative);
	console.log(inside ? relative : dir);
} catch (error) {
	console.error(error.message);
	process.exit(1);
}
