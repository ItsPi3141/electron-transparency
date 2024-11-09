const native = require("bindings")("electronTransparency");

const Materials = {
	Mica: 0b000010,
	MicaTabbed: 0b000100,
};
const Theme = {
	Light: 0b001000,
	Dark: 0b010000,
	Auto: 0b100000,
};

/**
 * Get hwnd of window
 * @param {import("electron").BrowserWindow} win
 */
const getHwnd = (win) => win.getNativeWindowHandle().readInt32LE();

/**
 * Apply mica
 * @param {import("electron").BrowserWindow} win
 * @param {[keyof typeof Materials]} type
 * @param {[keyof typeof Theme]} theme
 */
const applyMica = (win, type, theme) => {
	native.applyMica(getHwnd(win), type | theme);
};

/**
 * Restore grayed out controls
 * @param {import("electron").BrowserWindow} win
 */
const restoreControls = (win) => {
	native.restoreControls(getHwnd(win), 0);
};

/**
 * Maximize window
 * @param {import("electron").BrowserWindow} win
 */
const maximize = (win) => {
	native.maximize(getHwnd(win), 0);
};

/**
 * Remove window frame
 * @param {import("electron").BrowserWindow} win
 */
const removeFrame = (win) => {
	native.removeFrame(getHwnd(win), 0);
};

/**
 * Redraw window
 * @param {import("electron").BrowserWindow} win
 */
const redrawWindow = (win) => {
	native.redrawWindow(getHwnd(win), 0);
};

module.exports = {
	applyMica,
	restoreControls,
	maximize,
	removeFrame,
	redrawWindow,

	Materials,
	Theme,
};
