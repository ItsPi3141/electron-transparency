const { ipcRenderer } = require("electron");
window.addEventListener("load", () => {
	document.querySelector(".minimize").addEventListener("click", () => {
		ipcRenderer.send("window:minimize");
	});
	document.querySelector(".restore").addEventListener("click", () => {
		ipcRenderer.send("window:restore");
	});
	document.querySelector(".maximize").addEventListener("click", () => {
		ipcRenderer.send("window:maximize");
	});
	document.querySelector(".close").addEventListener("click", () => {
		ipcRenderer.send("window:close");
	});
});
