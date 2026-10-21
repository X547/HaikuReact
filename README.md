# haiku-react

A React renderer for native [Haiku](https://www.haiku-os.org/) applications. Components such as `<Window>`, `<Button>` and `<MenuBar>` are real Interface Kit objects (`BWindow`, `BButton`, `BMenuBar`, …), laid out by the Haiku layout engine and driven by React 19.

```tsx
import {useState} from 'react';
import {Button, Group, render, StringView, Window} from 'haiku-react';

function App() {
	const [count, setCount] = useState(0);
	return (
		<Window title="Hello">
			<Group insets="window">
				<StringView>Clicked {count} times</StringView>
				<Button onClick={() => setCount(count + 1)}>Increment</Button>
			</Group>
		</Window>
	);
}

render(<App/>, {signature: 'application/x-vnd.Example-Hello'});
```

## Requirements

- Haiku (developed and tested on a nightly, hrev60205, x86_64)
- Node.js 22 or newer with its headers, npm, Meson, Ninja and GCC:

  ```sh
  pkgman install nodejs24 nodejs24_devel npm meson ninja
  ```

The native add-on is built from source with Meson when the package is installed. On other systems installing succeeds without building it, which is enough for editing and type checking, but nothing can run there.

## Installing and building

In an application:

```sh
npm install haiku-react react
```

From a checkout of this repository:

```sh
npm install          # dependencies, native add-on (build/) and TypeScript (dist/)
npm test             # end-to-end tests; they open real windows
npm run example      # lists the examples
npm run example form # runs one
```

`npm run build` rebuilds both parts; `npm run build:native` and `npm run build:ts` rebuild one. The tests need a running Haiku desktop session. `npm run typecheck` checks the TypeScript without building; it also works on other systems after `npm install` there, and the repository's VS Code settings make the editor use the same TypeScript version.

Use TypeScript with `"jsx": "react-jsx"` or any JSX setup that targets React 19. TypeScript 7 (the native Go compiler) has no Haiku build; use TypeScript 6 to compile on Haiku.

### Developing from another machine

`scripts/remote.sh` uploads the sources to a Haiku machine over ssh and builds, tests or runs them there:

```sh
cat > local/remote.env <<'EOF'
HAIKU_HOST=user@haiku-vm
HAIKU_DIR=remote/HaikuReact
EOF
sh scripts/remote.sh install    # first time
sh scripts/remote.sh test
sh scripts/remote.sh run dist/examples/hello.js
```

## Concepts

### The application

`render(element, options)` starts the `BApplication` and renders `element`, which must consist of `<Window>` elements (a fragment or an array for several windows). It can be called once per process; `root.render(next)` replaces the tree.

| Option | Description |
|---|---|
| `signature` | MIME application signature, e.g. `'application/x-vnd.Vendor-App'` |
| `onQuitRequest()` | Command+Q, Deskbar or shutdown asked the application to quit. Return `false` to refuse; by default it quits |
| `strictMode` | React Strict Mode for the whole tree |
| `onCaughtError(error, info)` | An error boundary caught an error. Default: logged |
| `onUncaughtError(error, info)` | No boundary caught it. Default: logged, exit code 1, quit |

`quit()` (or `root.quit()`) unmounts everything and ends the application; Node exits once nothing else keeps it busy.

Closing a window calls its `onCloseRequest`. The window only goes away when React stops rendering it; without a handler, closing a window quits the application.

### Layout

Every container is a view with a Haiku layout, and sizes come from the content: windows open at their preferred size unless `width`/`height` are given.

- `<Group>` arranges its children in a row or column. Extra space is shared by `weight`.
- `<Grid>` places children by their `column`, `row`, `columnSpan` and `rowSpan` props.
- `<Glue/>` takes up the remaining space; `<Strut size={…}/>` adds fixed space.
- Spacing and insets are pixels or the names of the system's spacing values: `'default'`, `'item'`, `'half'`, `'window'`, `'small'`, `'corner'`, `'big'`, `'border'`. `insets` also takes `[horizontal, vertical]` or `[left, top, right, bottom]`.
- Containers fill the available width; controls keep their natural size (buttons and labels do not grow) and are aligned within their cell by `horizontalAlignment`/`verticalAlignment`. Any view can set `minWidth`, `maxWidth`, `preferredWidth` (and the heights) in pixels; `'unlimited'` lifts a maximum.
- A `<Window>`'s root group has no insets or spacing, so a `<MenuBar>` can span the window; wrap the content in `<Group insets="window">`.

### Inputs and events

Inputs follow React's controlled/uncontrolled model. With the value prop set (`checked`, `value`, `text`, `selection`, `selected`, `marked`), the control shows that value: after each event, React applies the handler's updates and the control returns to the prop, so changes the handler does not accept are undone and filtered values (e.g. digits only) are shown as filtered. `defaultChecked`, `defaultValue`, `defaultText` and `defaultSelection` only set the initial value.

Event handlers receive the new value directly (`onChange(checked)`, `onChange(text)`, `onSelectionChange(indices)`), not an event object.

### Alerts

```ts
const choice = await showAlert({
	title: 'Close document',
	text: 'Save changes before closing?',
	buttons: ['Cancel', "Don't save", 'Save'],
	type: 'warning',
});
```

Resolves with the index of the chosen button, or -1 if the application quits first. Escape chooses `escapeButton` (default 0). Types: `'empty'`, `'info'`, `'idea'`, `'warning'`, `'stop'`.

### Errors

Wrong prop types and impossible nesting are reported by the native side as JavaScript errors, so React error boundaries catch them, e.g. `prop "enabled": expected a boolean` or `<Window> cannot be placed in a layout`.

## Elements

Sizes and positions are pixels. Props marked *creation* are read when the element is created; give it a new `key` to change them.

**All views** (everything except `Window`, `Tab`, menu entries, `Glue` and `Strut`): `hidden`, `minWidth`, `minHeight`, `maxWidth`, `maxHeight`, `preferredWidth`, `preferredHeight`, `horizontalAlignment` (`'left'`, `'center'`, `'right'`, `'fill'`), `verticalAlignment` (`'top'`, `'middle'`, `'bottom'`, `'fill'`).

**Children of containers**: `weight` (Group, Split; default 1), `column`, `row`, `columnSpan`, `rowSpan` (Grid), `collapsible` (Split; default true).

### Windows and containers

| Element | Haiku class | Props | Events |
|---|---|---|---|
| `Window` | `BWindow` | `title`, `visible`, `left`, `top`, `width`, `height`, `look` (`'titled'`, `'document'`, `'modal'`, `'floating'`, `'bordered'`, `'none'`), `feel` (`'normal'`, `'modal-app'`, `'modal-all'`, `'floating-app'`, `'floating-all'`), `resizable`, `zoomable`, `closable`, `minimizable`, `movable`, `closeOnEscape`, `orientation`, `spacing`, `insets` | `onCloseRequest()`, `onActivate(active)`, `onMove({x, y})`, `onResize({width, height})` |
| `Group` | `BGroupView` | `orientation` (default vertical), `spacing` (default `'default'`), `insets` (default 0) | |
| `Grid` | `BGridView` | `horizontalSpacing`, `verticalSpacing`, `insets`, `columnWeights`, `rowWeights` | |
| `Split` | `BSplitView` | `orientation` (default horizontal), `spacing`, `insets` | |
| `Cards` | `BCardView` | `activeIndex` | |
| `Box` | `BBox` | `label`, `border` (`'fancy'`, `'plain'`, `'none'`), `orientation`, `spacing`, `insets` | |
| `Tabs` | `BTabView` | `selected`; children are `Tab` | `onChange(index)` |
| `Tab` | `BTab` | `label`, `orientation`, `spacing`, `insets` | |
| `Scroll` | `BScrollView` | `horizontal`, `vertical` (creation; default vertical only), `border`; one child | |
| `Glue` | `BSpaceLayoutItem` | | |
| `Strut` | `BSpaceLayoutItem` | `size` | |

`Window`'s `left`, `top`, `width` and `height` are applied initially and whenever they change; the user can move and resize the window in between.

### Controls

| Element | Haiku class | Props | Events |
|---|---|---|---|
| `StringView` | `BStringView` | `text` or text children | |
| `Button` | `BButton` | `label` or text children, `enabled` | `onClick()` |
| `CheckBox` | `BCheckBox` | `label` or text children, `checked`, `defaultChecked`, `enabled` | `onChange(checked)` |
| `RadioButton` | `BRadioButton` | like `CheckBox`; buttons with the same parent form a group | `onChange(true)` |
| `TextControl` | `BTextControl` | `label`, `value`, `defaultValue`, `enabled`, `password`, `maxLength` | `onChange(text)`, `onSubmit(text)` on Enter or leaving after an edit |
| `TextView` | `BTextView` | `text`, `defaultText`, `editable`, `selectable`, `wordWrap` | `onChange(text)` |
| `Slider` | `BSlider` | `label`, `min`, `max`, `value`, `defaultValue`, `enabled`, `orientation`, `hashMarks` (`'none'`, `'top'`, `'bottom'`, `'both'`), `hashMarkCount`, `minLabel`, `maxLabel` | `onChange(value)` while dragging, `onChangeEnd(value)` |
| `StatusBar` | `BStatusBar` | `label`, `trailingLabel`, `text`, `trailingText`, `value`, `maxValue` | |
| `ListView` | `BListView` | `items` (strings), `selection`, `defaultSelection`, `multiple` | `onSelectionChange(indices)`, `onInvoke(index)` |
| `MenuField` | `BMenuField` | `label`, `items` (strings), `value`, `defaultValue`, `enabled` | `onChange(index)` |

Put `TextView` and `ListView` in a `Scroll` to scroll them.

### Menus

| Element | Haiku class | Props | Events |
|---|---|---|---|
| `MenuBar` | `BMenuBar` | children: `Menu`, `MenuItem`, `Separator` | |
| `Menu` | `BMenu` | `label`, `enabled`, `radioMode` | |
| `MenuItem` | `BMenuItem` | `label` or text children, `enabled`, `marked`, `shortcut` (with Command), `modifiers` (`'shift'`, `'option'`, `'control'`) | `onClick()` |
| `Separator` | `BSeparatorItem` | | |

Command+Q is handled by Haiku as a quit request for the application (see `onQuitRequest`), not by a menu item with that shortcut.

## How it works

```
Node main thread                          Haiku threads
────────────────                          ─────────────
JS, React, react-reconciler               application thread: BApplication::Run()
host config ─► native add-on ─► lock ─►   window threads: one per BWindow
                                          │
event handlers ◄── thread-safe function ◄─┘ (never waits for JS)
```

- The add-on (C++, node-addon-api, `native/src`) starts the `BApplication` on its own thread, so the Node main thread keeps running JavaScript. Each React host instance is a native *node* (`native/src/nodes`) addressed by an integer id.
- React changes the native views from the JS thread while holding the window's lock, the usual way to modify another thread's window in Haiku. All windows touched by one React commit stay locked until the commit is complete, so a window never draws a half-applied update.
- Window threads send events to JavaScript through a non-blocking thread-safe function and never wait for it, which rules out deadlocks between the two sides.
- Every container is backed by a view (`BGroupView`, `BGridView`, …). A Haiku layout without a view of its own deletes its items when it is attached elsewhere, which React's bottom-up tree building would trigger.

The C++ part follows the `CODE_STYLE.md` of the surrounding development tree: tabs, `f`/`s`/`k` prefixes, braces on all bodies, exceptions for errors.

## Known limitations

- Labeled `TextControl`s in a column do not align their text fields yet.
- Changing a `ListView`'s `items` clears an uncontrolled selection.
- No context (pop-up) menus, file panels, images or custom drawing yet.
