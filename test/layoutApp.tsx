// Application under test for layout.test.tsx; diagnose-layout.tsx renders it too.

import {useState} from 'react';

import {
	Box,
	Button,
	Cards,
	Glue,
	Grid,
	Group,
	Scroll,
	Split,
	StringView,
	Strut,
	Tab,
	Tabs,
	Window,
} from '../src/index.js';

export interface LayoutState {
	gridSwap: boolean;
	activeCard: number;
	extraTab: boolean;
	controlledTab: number;
	windowWidth: number;
}

export let update: (patch: Partial<LayoutState>) => void = () => {};
export const tabChanges: number[] = [];

export function LayoutApp() {
	const [state, setState] = useState<LayoutState>({
		gridSwap: false,
		activeCard: 0,
		extraTab: false,
		controlledTab: 1,
		windowWidth: 600,
	});
	update = (patch) => setState((current) => ({...current, ...patch}));

	return (
		<Window title="Layout test" width={state.windowWidth}>
			<Group insets={10} spacing={10}>
				<Grid horizontalSpacing={0} verticalSpacing={0}>
					<Button column={0} row={0} maxWidth="unlimited">G00</Button>
					<Button column={state.gridSwap ? 0 : 1} row={state.gridSwap ? 2 : 0} maxWidth="unlimited">G10</Button>
					<Button column={0} row={1} columnSpan={2} maxWidth="unlimited">Span</Button>
				</Grid>
				<Group orientation="horizontal" spacing={0}>
					<Button>A</Button>
					<Strut size={30}/>
					<Button>B</Button>
					<Glue/>
					<Button>End</Button>
				</Group>
				<Split>
					<Box label="Left"><StringView>left</StringView></Box>
					<Box label="Right" weight={3}><StringView>right</StringView></Box>
				</Split>
				<Cards activeIndex={state.activeCard}>
					<StringView>card0</StringView>
					<StringView>card1</StringView>
					<StringView>card2</StringView>
				</Cards>
				<Tabs>
					{state.extraTab && <Tab label="New"><StringView>new</StringView></Tab>}
					<Tab label="One"><StringView>one</StringView></Tab>
					<Tab label="Two"><StringView>two</StringView></Tab>
				</Tabs>
				<Tabs selected={state.controlledTab} onChange={(index) => tabChanges.push(index)}>
					<Tab label="C0"/>
					<Tab label="C1"/>
					<Tab label="C2"/>
				</Tabs>
				<Scroll maxHeight={120}>
					<Group>
						{Array.from({length: 30}, (_, i) => <StringView key={i}>Line {i}</StringView>)}
					</Group>
				</Scroll>
				<Button minWidth={250} horizontalAlignment="center">Wide</Button>
			</Group>
		</Window>
	);
}
