import http from 'node:http';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import {spawn} from 'node:child_process';

const args = process.argv.slice(2);
const htmlPath = args[0];
const filterArg = args.find((arg) => arg.startsWith('--filter='));
const filter = filterArg ? filterArg.substring('--filter='.length) : 'WebGPUBrowser.*';

if (!htmlPath) {
	console.error('Usage: node run_webgpu_browser_test.mjs <LLGI_Test.html> [--filter=WebGPUBrowser.*]');
	process.exit(2);
}

const resolvedHtmlPath = path.resolve(htmlPath);
if (!fs.existsSync(resolvedHtmlPath)) {
	console.error(`Not found: ${resolvedHtmlPath}`);
	process.exit(2);
}

const root = path.dirname(resolvedHtmlPath);
const htmlFile = path.basename(resolvedHtmlPath);
const executablePath = findBrowserExecutable();

function findBrowserExecutable() {
	const candidates = [
		process.env.CHROME_PATH,
		process.env.EDGE_PATH,
		'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',
		'/Applications/Microsoft Edge.app/Contents/MacOS/Microsoft Edge',
		'/usr/bin/google-chrome',
		'/usr/bin/google-chrome-stable',
		'/usr/bin/chromium',
		'/usr/bin/chromium-browser',
		'C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe',
		'C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe',
		'C:\\Program Files\\Microsoft\\Edge\\Application\\msedge.exe',
		'C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe',
	].filter(Boolean);

	for (const candidate of candidates) {
		if (fs.existsSync(candidate)) {
			return candidate;
		}
	}

	console.error('A WebGPU-capable Chrome or Edge executable is required.');
	console.error('Set CHROME_PATH, for example on Windows:');
	console.error('$env:CHROME_PATH = "C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe"');
	console.error('or on macOS:');
	console.error('CHROME_PATH="/Applications/Google Chrome.app/Contents/MacOS/Google Chrome"');
	process.exit(2);
}

function chromeGpuArgs() {
	if (process.platform === 'win32') {
		return ['--use-angle=d3d11'];
	}
	if (process.platform === 'darwin') {
		return ['--use-angle=metal'];
	}
	return [];
}

function contentType(filePath) {
	if (filePath.endsWith('.html')) return 'text/html';
	if (filePath.endsWith('.js')) return 'application/javascript';
	if (filePath.endsWith('.wasm')) return 'application/wasm';
	if (filePath.endsWith('.data')) return 'application/octet-stream';
	return 'application/octet-stream';
}

const server = http.createServer((request, response) => {
	const requestUrl = new URL(request.url, 'http://127.0.0.1');
	if (requestUrl.pathname === '/favicon.ico') {
		response.writeHead(204);
		response.end();
		return;
	}

	const relativePath = decodeURIComponent(requestUrl.pathname === '/' ? `/${htmlFile}` : requestUrl.pathname);
	const filePath = path.resolve(root, `.${relativePath}`);

	if (!filePath.startsWith(root) || !fs.existsSync(filePath) || !fs.statSync(filePath).isFile()) {
		response.writeHead(404);
		response.end('Not found');
		return;
	}

	response.writeHead(200, {
		'Content-Type': contentType(filePath),
		'Cache-Control': 'no-store, max-age=0',
		'Pragma': 'no-cache',
		'Expires': '0',
	});
	fs.createReadStream(filePath).pipe(response);
});

await new Promise((resolve) => server.listen(0, '127.0.0.1', resolve));
const {port} = server.address();
const url = `http://127.0.0.1:${port}/${htmlFile}?filter=${encodeURIComponent(filter)}`;

function delay(ms) {
	return new Promise((resolve) => setTimeout(resolve, ms));
}

async function getFreePort() {
	const probe = http.createServer();
	await new Promise((resolve) => probe.listen(0, '127.0.0.1', resolve));
	const freePort = probe.address().port;
	await new Promise((resolve) => probe.close(resolve));
	return freePort;
}

async function waitForDevToolsHttp(devtoolsPort, chrome, getStderr) {
	const start = Date.now();
	while (Date.now() - start < 30000) {
		if (chrome.exitCode !== null) {
			throw new Error(`Chrome exited before DevTools was ready.${getStderr() ? `\n${getStderr()}` : ''}`);
		}
		try {
			const response = await fetch(`http://127.0.0.1:${devtoolsPort}/json/version`);
			if (response.ok) {
				return;
			}
		} catch {
		}
		await delay(100);
	}
	throw new Error('Timed out waiting for Chrome DevTools HTTP endpoint.');
}

async function createPage(devtoolsPort, pageUrl) {
	const response = await fetch(`http://127.0.0.1:${devtoolsPort}/json/new?${encodeURIComponent(pageUrl)}`, {method: 'PUT'});
	if (!response.ok) {
		throw new Error(`Failed to create Chrome page: ${response.status} ${response.statusText}`);
	}
	return await response.json();
}

class CdpClient {
	constructor(webSocketUrl) {
		this.nextId = 1;
		this.pending = new Map();
		this.handlers = new Map();
		this.socket = new WebSocket(webSocketUrl);
		this.socket.addEventListener('message', (event) => this.onMessage(event));
	}

	async open() {
		if (this.socket.readyState === WebSocket.OPEN) {
			return;
		}
		await new Promise((resolve, reject) => {
			this.socket.addEventListener('open', resolve, {once: true});
			this.socket.addEventListener('error', reject, {once: true});
		});
	}

	on(eventName, handler) {
		const handlers = this.handlers.get(eventName) || [];
		handlers.push(handler);
		this.handlers.set(eventName, handlers);
	}

	onMessage(event) {
		const message = JSON.parse(event.data);
		if (message.id && this.pending.has(message.id)) {
			const {resolve, reject} = this.pending.get(message.id);
			this.pending.delete(message.id);
			if (message.error) {
				reject(new Error(message.error.message));
			} else {
				resolve(message.result);
			}
			return;
		}

		for (const handler of this.handlers.get(message.method) || []) {
			handler(message.params || {});
		}
	}

	send(method, params = {}) {
		const id = this.nextId++;
		this.socket.send(JSON.stringify({id, method, params}));
		return new Promise((resolve, reject) => {
			this.pending.set(id, {resolve, reject});
		});
	}

	close() {
		this.socket.close();
	}
}

function consoleArgumentToString(argument) {
	if ('value' in argument) {
		return String(argument.value);
	}
	return argument.description || argument.unserializableValue || '';
}

async function evaluateJson(client, expression) {
	const result = await client.send('Runtime.evaluate', {
		expression: `JSON.stringify((${expression}))`,
		awaitPromise: true,
		returnByValue: true,
	});
	return result.result && result.result.value ? JSON.parse(result.result.value) : null;
}

async function waitForTestResult(client, timeoutMs) {
	const start = Date.now();
	while (Date.now() - start < timeoutMs) {
		const value = await evaluateJson(client, 'globalThis.Module && globalThis.Module.llgiTestResult || null');
		if (value) {
			return value;
		}
		await delay(100);
	}
	throw new Error('Timed out waiting for LLGI browser WebGPU test result.');
}

let chrome;
let client;
let userDataDir;
let chromeExit;
let chromeStderr = '';
try {
	userDataDir = fs.mkdtempSync(path.join(os.tmpdir(), 'llgi-webgpu-chrome-'));
	const devtoolsPort = await getFreePort();
	const chromeArgs = [
		'--headless=new',
		`--remote-debugging-port=${devtoolsPort}`,
		'--remote-debugging-address=127.0.0.1',
		`--user-data-dir=${userDataDir}`,
		'--no-first-run',
		'--no-default-browser-check',
		'--enable-unsafe-webgpu',
		'--ignore-gpu-blocklist',
		...chromeGpuArgs(),
		'about:blank',
	];
	chrome = spawn(executablePath, chromeArgs, {stdio: ['ignore', 'ignore', 'pipe']});
	chrome.stderr.on('data', (chunk) => {
		chromeStderr += chunk.toString();
	});
	chromeExit = new Promise((resolve) => chrome.once('exit', resolve));

	await waitForDevToolsHttp(devtoolsPort, chrome, () => chromeStderr.trim());
	const page = await createPage(devtoolsPort, url);
	client = new CdpClient(page.webSocketDebuggerUrl);
	await client.open();
	client.on('Runtime.consoleAPICalled', (params) => {
		const text = (params.args || []).map(consoleArgumentToString).join(' ');
		console.log(`[browser:${params.type}] ${text}`);
	});
	client.on('Runtime.exceptionThrown', (params) => {
		const details = params.exceptionDetails || {};
		const exception = details.exception || {};
		console.error(`[browser:pageerror] ${exception.description || details.text || 'Unhandled exception'}`);
	});

	await client.send('Runtime.enable');
	await client.send('Page.enable');
	const value = await waitForTestResult(client, 60000);
	if (!value || value.status !== 'passed') {
		console.error(value && value.message ? value.message : 'LLGI browser WebGPU test failed.');
		process.exitCode = 1;
	} else {
		await delay(500);
		const lateError = await evaluateJson(client, 'globalThis.Module && globalThis.Module.llgiLastWebGPUError || null');
		if (lateError) {
			console.error(lateError);
			process.exitCode = 1;
		}
	}
} finally {
	if (client) {
		await client.send('Browser.close').catch(() => {});
		client.close();
	}
	if (chrome) {
		await Promise.race([chromeExit, delay(2000)]);
		if (chrome.exitCode === null) {
			chrome.kill();
			await Promise.race([chromeExit, delay(2000)]);
		}
	}
	if (userDataDir) {
		fs.rmSync(userDataDir, {recursive: true, force: true, maxRetries: 10, retryDelay: 100});
	}
	await new Promise((resolve) => server.close(resolve));
}
