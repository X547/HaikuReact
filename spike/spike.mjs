// Phase 0 spike driver. Usage: node spike/spike.mjs [--manual]
//   default:  fully automated round trip, exits with code 0 on success
//   --manual: shows the window and waits for a real click / close

import {createRequire} from 'node:module';

const require = createRequire(import.meta.url);
const native = require('./build/spike.node');

const manual = process.argv.includes('--manual');
const log = (...args) => console.log('[spike]', ...args);
const fail = (message) => {
	console.error('[spike] FAIL:', message);
	process.exit(1);
};

let ticks = 0;
const ticker = setInterval(() => ticks++, 10);

const timeout = setTimeout(() => fail('timeout'), manual ? 600_000 : 10_000);
timeout.unref();

process.on('exit', (code) => log(`node exit (code ${code}), libuv ticks: ${ticks}`));

try {
	native.failTest();
	fail('failTest did not throw');
} catch (e) {
	log('native exception surfaced as JS error:', e instanceof Error, e.message);
}

native.start('application/x-vnd.HaikuReact-Spike', (kind, value) => {
	log('event', kind, value);
	switch (kind) {
		case 'click':
			native.setButtonLabel(`Clicked ${value}x`);
			log('layout after relabel', native.dumpLayout());
			if (!manual) {
				native.requestClose();
			}
			break;
		case 'closeRequest':
			native.destroyWindow();
			native.quit();
			break;
		case 'appQuitRequest':
			native.quit();
			break;
		case 'appQuit':
			clearInterval(ticker);
			log('application thread finished; node should exit now');
			break;
	}
});
log('started');

native.createWindow('HaikuReact spike');
native.setButtonLabel('Label set from JS before Show()');
native.show();
log('threads', native.threads());
log('layout', native.dumpLayout());

if (!manual) {
	setTimeout(() => native.clickButton(), 300);
}
