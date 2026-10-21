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

/** List of text items; wrap in <Scroll> to scroll. */
export interface ListViewProps extends ViewProps {
	items: readonly string[];
	/** Indices of the selected items. */
	selection?: readonly number[];
	defaultSelection?: readonly number[];
	/** Allow selecting several items. Default false. */
	multiple?: boolean;
	onSelectionChange?: (selection: number[]) => void;
	/** Double-click or Enter on an item. */
	onInvoke?: (index: number) => void;
}

/** Labeled pop-up menu for choosing one of several items. */
export interface MenuFieldProps extends ViewProps {
	label?: string;
	items: readonly string[];
	/** Index of the chosen item; -1 for none. */
	value?: number;
	defaultValue?: number;
	enabled?: boolean;
	onChange?: (index: number) => void;
}

/** Menu bar; usually the first child of a <Window>, whose root has no insets. */
export interface MenuBarProps extends ViewProps {
	/** <Menu>, <MenuItem> and <Separator> elements. */
	children?: ReactNode;
}

export interface MenuProps {
	label?: string;
	/** Default true. */
	enabled?: boolean;
	/** Choosing an item marks it and unmarks the others. */
	radioMode?: boolean;
	/** <Menu>, <MenuItem> and <Separator> elements. */
	children?: ReactNode;
}

export type KeyModifier = 'shift' | 'option' | 'control';

export interface MenuItemProps {
	/** Alternative to text children. */
	label?: string;
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
export const CheckBox = hostElement<CheckBoxProps>('CheckBox');
export const RadioButton = hostElement<RadioButtonProps>('RadioButton');
export const TextControl = hostElement<TextControlProps>('TextControl');
export const TextView = hostElement<TextViewProps>('TextView');
export const Slider = hostElement<SliderProps>('Slider');
export const StatusBar = hostElement<StatusBarProps>('StatusBar');
export const ListView = hostElement<ListViewProps>('ListView');
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
};
