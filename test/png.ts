import {crc32, deflateSync} from 'node:zlib';

function chunk(type: string, data: Buffer): Buffer {
	const length = Buffer.alloc(4);
	length.writeUInt32BE(data.length);
	const typeAndData = Buffer.concat([Buffer.from(type, 'ascii'), data]);
	const crc = Buffer.alloc(4);
	crc.writeUInt32BE(crc32(typeAndData));
	return Buffer.concat([length, typeAndData, crc]);
}

/** Encodes a PNG of one RGBA color. */
export function makePng(width: number, height: number, rgba: readonly [number, number, number, number]): Uint8Array {
	const header = Buffer.alloc(13);
	header.writeUInt32BE(width, 0);
	header.writeUInt32BE(height, 4);
	header[8] = 8;  // bit depth
	header[9] = 6;  // RGBA
	const row = Buffer.alloc(1 + width * 4);
	for (let x = 0; x < width; x++) {
		row.set(rgba, 1 + x * 4);
	}
	const pixels = Buffer.concat(Array.from({length: height}, () => row));
	return new Uint8Array(Buffer.concat([
		Buffer.from([0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a]),
		chunk('IHDR', header),
		chunk('IDAT', deflateSync(pixels)),
		chunk('IEND', Buffer.alloc(0)),
	]));
}
