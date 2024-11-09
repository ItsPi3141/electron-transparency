<h1>
    <p align="center">Electron Transparency</p>

</h1>
<p align="center">
    <i>Mica background for Electron apps</i>
</p>
<hr>

## About

Electron has a built-in feature to add transparency effects to the window. However, this feature has some limitations:

- The transparency effect is only applied to the window frame, not to the window content.
- In order to apply the transparency effect to the whole window, `transparent` must be set to `true` in the `BrowserWindow` options. This creates a number of bugs which prevents the window from being resized or maximized. (see [this issue](https://github.com/electron/electron/pull/28207))

Electron Transparency aims to solve these issues by adding a native module which allows the transparency effect to be applied to the window content.

## Usage

See the example app [here](https://github.com/ItsPi3141/electron-transparency/tree/main/example)

### Installation

Install the module and electron-rebuild

```bash
npm install electron-transparency @electron/rebuild
```

Compile the native module

```bash
npx electron-rebuild
```
