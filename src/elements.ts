// Host element types and their props. Each export is the element's type
// string, typed as a component so JSX checks its props.

import type {ComponentType, ReactNode} from 'react';

export type SpacingName = 'default' | 'item' | 'half' | 'window' | 'small' | 'corner' | 'big' | 'border';

/** Pixels, or a named B_USE_*_SPACING value of the current look. */
export type Spacing = number | SpacingName;

export type Insets =
	| Spacing
	| readonly [horizontal: Spacing, vertical: Spacing]
	| readonly [left: Spacing, top: Spacing, right: Spacing, bottom: Spacing];

export type Orientation = 'horizontal' | 'vertical';
export type HorizontalAlignment = 'left' | 'center' | 'right' | 'fill';
export type VerticalAlignment = 'top' | 'middle' | 'bottom' | 'fill';
export type BorderStyle = 'fancy' | 'plain' | 'none';

/** Pixels; 'unlimited' lifts a maximum. */
export type LayoutSize = number | 'unlimited';

/**
 * Where an icon or image comes from.
 *
 * - `system`: an icon built into Haiku: 'dialog-information', 'dialog-idea',
 *   'dialog-warning', 'dialog-error', or a flag like 'flag-de'
 * - `mimeType`: the icon of a file type, e.g. 'text/plain' or
 *   'application/x-vnd.Be-directory' (folder); unknown subtypes show their
 *   supertype's icon
 * - `file`: the icon Tracker shows for a file or folder
 * - `app`: an application's icon, by signature
 * - `path`: an HVIF, PNG, JPEG, … file
 * - `data`: encoded HVIF or image bytes
 */
export type IconSource =
	| {readonly system: string}
	| {readonly mimeType: string}
	| {readonly file: string}
	| {readonly app: string}
	| {readonly path: string}
	| {readonly data: Uint8Array | ArrayBuffer};

/** Children that are joined into a label or text property. */
export type TextChildren = string | number | ReadonlyArray<string | number>;

/** Read by the parent container; each container uses the ones that apply to it. */
export interface LayoutItemProps {
	/** Share of the extra space in a Group or Split. Default 1. */
	weight?: number;
	/** Cell in a Grid. Default 0. */
	column?: number;
	row?: number;
	/** Cells covered in a Grid. Default 1. */
	columnSpan?: number;
	rowSpan?: number;
	/** Whether a Split may collapse this child by dragging. Default true. */
	collapsible?: boolean;
}

export interface ViewProps extends LayoutItemProps {
	hidden?: boolean;
	/**
	 * Explicit layout limits; unset ones come from the view's content. A
	 * minimum above the view's own maximum raises the maximum.
	 *
	 * Containers (Group, Grid, Split, Cards, Box, Tabs, Scroll) default to
	 * maxWidth="unlimited" and fill the available width; controls keep their
	 * natural maximum and are aligned within the space they get.
	 */
	minWidth?: LayoutSize;
	minHeight?: LayoutSize;
	maxWidth?: LayoutSize;
	maxHeight?: LayoutSize;
	preferredWidth?: LayoutSize;
	preferredHeight?: LayoutSize;
	/** Placement in a cell that is larger than the view's maximum size. */
	horizontalAlignment?: HorizontalAlignment;
	verticalAlignment?: VerticalAlignment;
}

export interface GroupLayoutProps {
	orientation?: Orientation;
	spacing?: Spacing;
	insets?: Insets;
}

export type WindowLook = 'titled' | 'document' | 'modal' | 'floating' | 'bordered' | 'none';

/** 'modal-app' blocks the application's other windows; 'floating-app' stays above them. */
export type WindowFeel = 'normal' | 'modal-app' | 'modal-all' | 'floating-app' | 'floating-all';

export interface WindowProps extends GroupLayoutProps {
	title?: string;
	/** Default true. */
	visible?: boolean;
	/**
	 * Screen position; the window is centered when neither is set. Applied
	 * initially and whenever the prop changes; the user may move the window
	 * in between (see onMove).
	 */
	left?: number;
	top?: number;
	/** Size in pixels, applied like left/top; the preferred layout size is used when neither is set. */
	width?: number;
	height?: number;
	/** Frame and title bar style. Default 'titled'. */
	look?: WindowLook;
	/** Stacking behavior. Default 'normal'. */
	feel?: WindowFeel;
	/** Default true. */
	resizable?: boolean;
	zoomable?: boolean;
	closable?: boolean;
	minimizable?: boolean;
	movable?: boolean;
	/** Escape requests closing the window. Default false. */
	closeOnEscape?: boolean;
	/** The user asked to close the window. Without a handler, the application quits. */
	onCloseRequest?: () => void;
	onActivate?: (active: boolean) => void;
	onMove?: (position: {x: number; y: number}) => void;
	/** Size in pixels. */
	onResize?: (size: {width: number; height: number}) => void;
	children?: ReactNode;
}

/** Default: vertical, 'default' spacing, no insets. */
export interface GroupProps extends ViewProps, GroupLayoutProps {
	children?: ReactNode;
}

export interface GridProps extends ViewProps {
	/** Default 'default'. */
	horizontalSpacing?: Spacing;
	verticalSpacing?: Spacing;
	/** Default 0. */
	insets?: Insets;
	/** Weights of the first columns/rows; the others weigh 1. */
	columnWeights?: readonly number[];
	rowWeights?: readonly number[];
	children?: ReactNode;
}

export interface SplitProps extends ViewProps {
	/** Default 'horizontal'. */
	orientation?: Orientation;
	spacing?: Spacing;
	insets?: Insets;
	children?: ReactNode;
}

export interface CardsProps extends ViewProps {
	/** Index of the visible child. Default 0. */
	activeIndex?: number;
	children?: ReactNode;
}

/** Content defaults: vertical, 'default' spacing and insets. */
export interface BoxProps extends ViewProps, GroupLayoutProps {
	label?: string;
	/** Default 'fancy'. */
	border?: BorderStyle;
	children?: ReactNode;
}

export interface ScrollProps extends ViewProps {
	/** Scroll bars; read when the element is created. Default: vertical only. */
	horizontal?: boolean;
	vertical?: boolean;
	/** Default 'fancy'. */
	border?: BorderStyle;
	/** A single element. */
	children?: ReactNode;
}

export interface TabsProps extends ViewProps {
	/** Index of the selected tab. When set, the selection is controlled. */
	selected?: number;
	onChange?: (index: number) => void;
	/** <Tab> elements. */
	children?: ReactNode;
}

/** Content defaults: vertical, 'default' spacing and insets. */
export interface TabProps extends GroupLayoutProps {
	label?: string;
	children?: ReactNode;
}

export type GlueProps = LayoutItemProps;

export interface StrutProps extends LayoutItemProps {
	/** Length along the parent group's orientation. */
	size?: Spacing;
}

export interface ButtonProps extends ViewProps {
	/** Alternative to text children. */
	label?: string;
	/** Default true. */
	enabled?: boolean;
	onClick?: () => void;
	children?: TextChildren;
}

export interface StringViewProps extends ViewProps {
	/** Alternative to text children. */
	text?: string;
	children?: TextChildren;
}

/** An icon, loaded when the element is rendered. A source without an icon shows nothing. */
export interface IconProps extends ViewProps {
	source?: IconSource;
	/** Nominal size in pixels, scaled with the system font like other icons. Default 16. */
	size?: number;
}

export type ImageFit = 'contain' | 'cover' | 'fill' | 'none';

/**
 * A raster image, decoded in the background. The preferred size is the image
 * size, scaled down to maxWidth/maxHeight if set.
 */
export interface ImageProps extends ViewProps {
	/** A file or encoded bytes (PNG, JPEG, … anything a translator reads). */
	src?: {readonly path: string} | {readonly data: Uint8Array | ArrayBuffer};
	/**
	 * How the image fills the view: 'contain' (whole image, default), 'cover'
	 * (fills the view, cropped), 'fill' (stretched) or 'none' (natural size).
	 */
	fit?: ImageFit;
	/** The image is shown; its size in pixels. */
	onLoad?: (size: {width: number; height: number}) => void;
	onError?: (message: string) => void;
}

// Inputs follow React's controlled/uncontrolled model: with the value prop
// set (checked, value, text, selection), the control shows that value and
// returns to it after an event unless the handler changes it; defaultX only
// sets the initial value.

export interface CheckBoxProps extends ViewProps {
	label?: string;
	checked?: boolean;
	defaultChecked?: boolean;
	enabled?: boolean;
	onChange?: (checked: boolean) => void;
	children?: TextChildren;
}

/** Radio buttons with the same parent element form a group. */
export interface RadioButtonProps extends ViewProps {
	label?: string;
	checked?: boolean;
	defaultChecked?: boolean;
	enabled?: boolean;
	/** Called with true when the user selects this button. */
	onChange?: (checked: boolean) => void;
	children?: TextChildren;
}

export interface TextControlProps extends ViewProps {
	label?: string;
	value?: string;
	defaultValue?: string;
	enabled?: boolean;
	/** Hides the typed characters. */
	password?: boolean;
	/** Maximum length in bytes. */
	maxLength?: number;
	/** Every edit. */
	onChange?: (value: string) => void;
	/** Enter, or leaving the field after an edit. */
	onSubmit?: (value: string) => void;
}

/** Multi-line text; wrap in <Scroll> to scroll. */
export interface TextViewProps extends ViewProps {
	text?: string;
	defaultText?: string;
	/** Default true. */
	editable?: boolean;
	/** Default true. */
	selectable?: boolean;
	/** Default true. */
	wordWrap?: boolean;
	onChange?: (text: string) => void;
}

export type HashMarks = 'none' | 'top' | 'bottom' | 'both';

export interface SliderProps extends ViewProps {
	label?: string;
	/** Default 0. */
	min?: number;
	/** Default 100. */
	max?: number;
	value?: number;
	defaultValue?: number;
	enabled?: boolean;
	/** Default 'horizontal'. */
	orientation?: Orientation;
	hashMarks?: HashMarks;
	hashMarkCount?: number;
	minLabel?: string;
	maxLabel?: string;
	/** While the thumb moves. */
	onChange?: (value: number) => void;
	/** When the thumb is released. */
	onChangeEnd?: (value: number) => void;
}

export interface StatusBarProps extends ViewProps {
	/** Shown above the bar on the left. */
	label?: string;
	/** Shown above the bar on the right. */
	trailingLabel?: string;
	/** Shown after the label. */
	text?: string;
	/** Shown before the trailing label. */
	trailingText?: string;
	/** Default 0. */
	value?: number;
	/** Default 100. */
	maxValue?: number;
}

interface ListViewBaseProps extends ViewProps {
	/** Allow selecting several items. Default false. */
	multiple?: boolean;
}

/** Flat list given by the items prop; items are addressed by index. */
export interface ListViewItemsProps extends ListViewBaseProps {
	items: readonly LabeledItem[];
	/** Indices of the selected items. */
	selection?: readonly number[];
	defaultSelection?: readonly number[];
	onSelectionChange?: (selection: number[]) => void;
	/** Double-click or Enter on an item. */
	onInvoke?: (index: number) => void;
	children?: undefined;
}

/** List of <ListItem> elements, which may be nested; items are addressed by id. */
export interface ListViewElementProps extends ListViewBaseProps {
	items?: undefined;
	/**
	 * Ids of the selected items. Only shown items can be selected: items under
	 * a collapsed parent are not.
	 */
	selection?: readonly string[];
	defaultSelection?: readonly string[];
	onSelectionChange?: (selection: string[]) => void;
	/** Double-click or Enter on an item. */
	onInvoke?: (id: string) => void;
	/** <ListItem> elements. */
	children?: ReactNode;
}

/** List view; wrap in <Scroll> to scroll. */
export type ListViewProps = ListViewItemsProps | ListViewElementProps;

/**
 * Item of a <ListView>, or of a <ListItem> whose children it shows when
 * expanded. To load children on demand, set hasChildren and add them in
 * onExpand; until they exist the item shows a "Loading…" child.
 */
export interface ListItemProps {
	/** Identifies the item in selection and events; unique within the list. */
	id: string;
	label?: string;
	icon?: IconSource;
	/** Shows the expand arrow even before there are children. */
	hasChildren?: boolean;
	/** Whether the children are shown. When set, it is controlled. */
	expanded?: boolean;
	/** Default false. */
	defaultExpanded?: boolean;
	/** Shows the "Loading…" child even when there are children (e.g. while refreshing). */
	loading?: boolean;
	/** Text of the placeholder child. Default 'Loading…'. */
	loadingLabel?: string;
	/** The user expanded the item. */
	onExpand?: () => void;
	/** The user collapsed the item. */
	onCollapse?: () => void;
	/** <ListItem> elements. */
	children?: ReactNode;
}

/**
 * Kind of values in a column: 'text' (strings, numbers or {text, icon}),
 * 'integer', 'size' (bytes, shown as KiB, MiB, …), 'date' (Date or
 * milliseconds) or 'icon' (an IconSource).
 */
export type ColumnType = 'text' | 'integer' | 'size' | 'date' | 'icon';

export interface Column {
	/** Names the column in cells and sort. */
	id: string;
	title?: string;
	/** Default 'text'. */
	type?: ColumnType;
	/** Default 100. */
	width?: number;
	/** Default 20. */
	minWidth?: number;
	maxWidth?: number;
	/** Default 'left'. */
	align?: 'left' | 'center' | 'right';
}

export type SortDirection = 'ascending' | 'descending';

export interface SortKey {
	column: string;
	direction?: SortDirection;
}

/** A cell value; which ones a column shows depends on its type. */
export type CellValue = string | number | Date | {readonly text: string; readonly icon?: IconSource} | IconSource;

/**
 * List with columns, which the user can resize, reorder and sort by clicking
 * their titles; it scrolls itself. While a sort is active, rows show in sort
 * order rather than in the order of the <Row> children. Uses Haiku's private
 * BColumnListView, so details may change with Haiku versions.
 */
export interface ColumnListViewProps extends ViewProps {
	/** Changing the columns rebuilds the list. */
	columns: readonly Column[];
	/** When set, sorting is controlled; [] shows rows in child order. */
	sort?: readonly SortKey[];
	defaultSort?: readonly SortKey[];
	/** The user sorted by clicking a column title. */
	onSortChange?: (sort: {column: string; direction: SortDirection}[]) => void;
	/** Ids of the selected rows; rows under a collapsed parent cannot be selected. */
	selection?: readonly string[];
	defaultSelection?: readonly string[];
	/** Allow selecting several rows. Default false. */
	multiple?: boolean;
	onSelectionChange?: (selection: string[]) => void;
	/** Double-click or Enter on a row. */
	onInvoke?: (id: string) => void;
	/** <Row> elements. */
	children?: ReactNode;
}

/** Row of a <ColumnListView>, or of a <Row>; nesting, expanding and lazy children work as for <ListItem>. */
export interface RowProps {
	/** Identifies the row in selection and events; unique within the list. */
	id: string;
	/** Values by column id; missing cells are blank. */
	cells?: Readonly<Record<string, CellValue>>;
	hasChildren?: boolean;
	expanded?: boolean;
	defaultExpanded?: boolean;
	loading?: boolean;
	/** Shown in the first text column of the placeholder row. Default 'Loading…'. */
	loadingLabel?: string;
	onExpand?: () => void;
	onCollapse?: () => void;
	/** <Row> elements. */
	children?: ReactNode;
}

/** Labeled pop-up menu for choosing one of several items. */
/** A list or menu entry: its label, optionally with an icon. */
export type LabeledItem = string | {readonly label: string; readonly icon?: IconSource};

export interface MenuFieldProps extends ViewProps {
	label?: string;
	/** Icons show in the pop-up menu, not in the field. */
	items: readonly LabeledItem[];
	/** Index of the chosen item; -1 for none. */
	value?: number;
	defaultValue?: number;
	enabled?: boolean;
	onChange?: (index: number) => void;
}

/** Menu bar; usually the first child of a <Window>, whose root has no insets. */
export interface MenuBarProps extends ViewProps {
	/** Nominal size of the icons of the bar's items, scaled with the system font. Default 16. */
	iconSize?: number;
	/** <Menu>, <MenuItem> and <Separator> elements. */
	children?: ReactNode;
}

export interface MenuProps {
	/** Without a label, a menu in a menu bar shows only its icon. */
	label?: string;
	/** Shown before the label. */
	icon?: IconSource;
	/** Nominal size of the icons of the menu's items. Default 16. */
	iconSize?: number;
	/** Default true. */
	enabled?: boolean;
	/** Choosing an item marks it and unmarks the others. */
	radioMode?: boolean;
	/**
	 * The menu opened (by mouse or keyboard). With this handler and no
	 * children the menu shows a disabled "Loading…" item, so items can be
	 * loaded here; the open menu updates when they arrive.
	 */
	onOpen?: () => void;
	onClose?: () => void;
	/** Shows the "Loading…" item even with children (e.g. while refreshing). */
	loading?: boolean;
	/** Text of the placeholder item. Default 'Loading…'. */
	loadingLabel?: string;
	/** <Menu>, <MenuItem> and <Separator> elements. */
	children?: ReactNode;
}

export type KeyModifier = 'shift' | 'option' | 'control';

export interface MenuItemProps {
	/** Alternative to text children. */
	label?: string;
	/** Shown before the label, dimmed while the item is disabled. */
	icon?: IconSource;
	/** Default true. */
	enabled?: boolean;
	/** Check mark; in a radio-mode menu, the chosen item. */
	marked?: boolean;
	/** Character typed with the Command key, e.g. 'S' for Command+S. */
	shortcut?: string;
	/** Further modifiers required with the shortcut. */
	modifiers?: readonly KeyModifier[];
	onClick?: () => void;
	children?: TextChildren;
}

function hostElement<Props>(type: string): ComponentType<Props> {
	return type as unknown as ComponentType<Props>;
}

export const Window = hostElement<WindowProps>('Window');
export const Group = hostElement<GroupProps>('Group');
export const Grid = hostElement<GridProps>('Grid');
export const Split = hostElement<SplitProps>('Split');
export const Cards = hostElement<CardsProps>('Cards');
export const Box = hostElement<BoxProps>('Box');
export const Scroll = hostElement<ScrollProps>('Scroll');
export const Tabs = hostElement<TabsProps>('Tabs');
export const Tab = hostElement<TabProps>('Tab');
export const Glue = hostElement<GlueProps>('Glue');
export const Strut = hostElement<StrutProps>('Strut');
export const Button = hostElement<ButtonProps>('Button');
export const StringView = hostElement<StringViewProps>('StringView');
export const Icon = hostElement<IconProps>('Icon');
export const Image = hostElement<ImageProps>('Image');
export const CheckBox = hostElement<CheckBoxProps>('CheckBox');
export const RadioButton = hostElement<RadioButtonProps>('RadioButton');
export const TextControl = hostElement<TextControlProps>('TextControl');
export const TextView = hostElement<TextViewProps>('TextView');
export const Slider = hostElement<SliderProps>('Slider');
export const StatusBar = hostElement<StatusBarProps>('StatusBar');
// Overloads rather than a union of props, so that handler parameters are
// typed from the form used.
export const ListView = 'ListView' as unknown as {
	(props: ListViewItemsProps): ReactNode;
	(props: ListViewElementProps): ReactNode;
};
export const ListItem = hostElement<ListItemProps>('ListItem');
export const ColumnListView = hostElement<ColumnListViewProps>('ColumnListView');
export const Row = hostElement<RowProps>('Row');
export const MenuField = hostElement<MenuFieldProps>('MenuField');
export const MenuBar = hostElement<MenuBarProps>('MenuBar');
export const Menu = hostElement<MenuProps>('Menu');
export const MenuItem = hostElement<MenuItemProps>('MenuItem');
export const Separator = hostElement<Record<string, never>>('Separator');

/** Elements whose text children are sent as the named property. */
export const textProperties: Readonly<Record<string, string>> = {
	Button: 'label',
	CheckBox: 'label',
	MenuItem: 'label',
	RadioButton: 'label',
	StringView: 'text',
};

/** Value props that make an element controlled. */
export const controlledProperties: Readonly<Record<string, readonly string[]>> = {
	CheckBox: ['checked'],
	RadioButton: ['checked'],
	TextControl: ['value'],
	TextView: ['text'],
	Slider: ['value'],
	ListView: ['selection'],
	ListItem: ['expanded'],
	ColumnListView: ['selection', 'sort'],
	Row: ['expanded'],
	MenuField: ['value'],
	MenuItem: ['marked'],
	Tabs: ['selected'],
};

/** Elements whose user change also changes their siblings (radio groups). */
export const groupedElements: ReadonlySet<string> = new Set(['RadioButton', 'MenuItem']);

/** Initial-value props and the value prop they initialize. */
export const defaultValueProperties: Readonly<Record<string, string>> = {
	defaultChecked: 'checked',
	defaultValue: 'value',
	defaultText: 'text',
	defaultSelection: 'selection',
	defaultExpanded: 'expanded',
	defaultSort: 'sort',
};
