import {useState} from 'react';

import {
	Box,
	Button,
	Cards,
	Glue,
	Grid,
	Group,
	quit,
	render,
	Scroll,
	Split,
	StringView,
	Strut,
	Tab,
	Tabs,
	Window,
} from '../src/index.js';

function GridPage() {
	return (
		<Grid columnWeights={[0, 1, 1]}>
			<StringView column={0} row={0}>Row 1:</StringView>
			<Button column={1} row={0} maxWidth="unlimited">One</Button>
			<Button column={2} row={0} maxWidth="unlimited">Two</Button>
			<StringView column={0} row={1}>Row 2:</StringView>
			<Button column={1} row={1} columnSpan={2} maxWidth="unlimited">Spanning two columns</Button>
			<Glue column={0} row={2}/>
		</Grid>
	);
}

function SplitPage() {
	return (
		<Split>
			<Box label="Sidebar">
				<StringView>Drag the divider.</StringView>
				<Glue/>
			</Box>
			<Box label="Content" weight={3}>
				<StringView>This pane takes three times the extra space.</StringView>
				<Glue/>
			</Box>
		</Split>
	);
}

function CardsPage() {
	const [card, setCard] = useState(0);
	const cards = ['First', 'Second', 'Third'];
	return (
		<Group>
			<Group orientation="horizontal">
				<Button enabled={card > 0} onClick={() => setCard(card - 1)}>Previous</Button>
				<Glue/>
				<StringView>{`${card + 1} / ${cards.length}`}</StringView>
				<Glue/>
				<Button enabled={card < cards.length - 1} onClick={() => setCard(card + 1)}>Next</Button>
			</Group>
			<Cards activeIndex={card}>
				{cards.map((name) => (
					<Box key={name} label={name}>
						<StringView>{`This is the ${name.toLowerCase()} card.`}</StringView>
						<Glue/>
					</Box>
				))}
			</Cards>
		</Group>
	);
}

function ScrollPage() {
	const [lines, setLines] = useState(40);
	return (
		<Group>
			<Group orientation="horizontal">
				<Button onClick={() => setLines(lines + 10)}>Add 10 lines</Button>
				<Button enabled={lines > 10} onClick={() => setLines(lines - 10)}>Remove 10</Button>
				<Glue/>
			</Group>
			<Scroll>
				<Group insets="small">
					{Array.from({length: lines}, (_, i) => <StringView key={i}>{`Line ${i + 1}`}</StringView>)}
				</Group>
			</Scroll>
		</Group>
	);
}

function App() {
	const [tab, setTab] = useState(0);
	return (
		<Window title="HaikuReact layout" width={520} height={400}>
			<Group insets="window">
				<Tabs selected={tab} onChange={setTab}>
					<Tab label="Grid"><GridPage/></Tab>
					<Tab label="Split"><SplitPage/></Tab>
					<Tab label="Cards"><CardsPage/></Tab>
					<Tab label="Scroll"><ScrollPage/></Tab>
				</Tabs>
				<Group orientation="horizontal">
					<StringView>{`Tab ${tab + 1} of 4`}</StringView>
					<Glue/>
					<Button onClick={() => setTab((tab + 1) % 4)}>Next tab</Button>
					<Strut size="half"/>
					<Button onClick={quit}>Quit</Button>
				</Group>
			</Group>
		</Window>
	);
}

render(<App/>, {signature: 'application/x-vnd.HaikuReact-Layout'});
