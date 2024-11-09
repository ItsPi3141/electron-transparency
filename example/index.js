const { app, ipcMain, WebContentsView, BaseWindow } = require("electron");
const path = require("node:path");
const {
	applyMica,
	restoreControls,
	maximize,
	removeFrame,
	redrawWindow,
	Materials,
	Theme,
} = require("../src");

app.on("ready", () => {
	const win = new BaseWindow({
		transparent: true,
		show: true,
	});

	const titlebarControls = new WebContentsView({
		webPreferences: {
			preload: path.join(__dirname, "titlebar.js"),
		},
	});
	win.contentView.addChildView(titlebarControls);
	titlebarControls.webContents.loadFile(path.join(__dirname, "titlebar.html"));
	titlebarControls.setBackgroundColor("#0000");

	const onResize_Titlebar = () => {
		const b = win.getBounds();
		titlebarControls.setBounds({
			x: b.width - 158,
			y: 0,
			width: 138,
			height: 30,
		});
	};
	onResize_Titlebar();
	win.on("resize", onResize_Titlebar);

	const mainView = new WebContentsView({});
	win.contentView.addChildView(mainView);
	mainView.webContents.loadFile(path.join(__dirname, "index.html"));
	mainView.setBackgroundColor("#0000");

	const onResize_MainWin = () => {
		const b = win.getBounds();
		mainView.setBounds({
			x: 0,
			y: 30,
			width: b.width,
			height: b.height - 30,
		});
	};
	onResize_MainWin();
	win.on("resize", onResize_MainWin);

	win.setMenu(null);
	titlebarControls.webContents.on("ready-to-show", () => {
		applyMica(win, Materials.Mica, Theme.Auto);
		removeFrame(win);
		redrawWindow(win);
	});
	setInterval(() => {
		try {
			restoreControls(win);
		} catch {}
	}, 60);

	ipcMain.on("window:maximize", () => {
		maximize(win);
	});

	ipcMain.on("window:close", () => {
		win.close();
	});
});
