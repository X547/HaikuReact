import {useState} from 'react';

import {
	Box,
	Button,
	CheckBox,
	Glue,
	Group,
	ListView,
	MenuField,
	quit,
	RadioButton,
	render,
	Scroll,
	Slider,
	Split,
	StatusBar,
	StringView,
	TextControl,
	TextView,
	Window,
} from '../src/index.js';

const fruits = ['Apple', 'Banana', 'Cherry', 'Durian', 'Elderberry', 'Fig', 'Grape', 'Honeydew'];
const sizes = ['Small', 'Medium', 'Large'];
const colors = ['Red', 'Green', 'Blue'];

function App() {
	const [name, setName] = useState('');
	const [age, setAge] = useState('');
	const [size, setSize] = useState('Medium');
	const [color, setColor] = useState(0);
	const [newsletter, setNewsletter] = useState(false);
	const [volume, setVolume] = useState(30);
	const [picked, setPicked] = useState<number[]>([]);
	const [notes, setNotes] = useState('');

	const summary = [
		name === '' ? 'Anonymous' : name,
		age === '' ? null : `${age} years`,
		size,
		colors[color],
		newsletter ? 'newsletter' : null,
		picked.length > 0 ? picked.map((index) => fruits[index]).join(', ') : null,
	].filter((part) => part !== null).join(' · ');

	return (
		<Window title="HaikuReact form" width={560}>
			<Group insets="window">
				<Split>
					<Group>
						<Box label="Person">
							<TextControl label="Name:" value={name} onChange={setName}/>
							<TextControl
								label="Age (digits only):"
								value={age}
								maxLength={3}
								onChange={(value) => setAge(value.replace(/\D/g, ''))}
							/>
							<MenuField label="Favorite color:" items={colors} value={color} onChange={setColor}/>
						</Box>
						<Box label="Options">
							<Group orientation="horizontal">
								{sizes.map((item) => (
									<RadioButton key={item} checked={size === item} onChange={() => setSize(item)}>
										{item}
									</RadioButton>
								))}
								<Glue/>
							</Group>
							<CheckBox checked={newsletter} onChange={setNewsletter}>Subscribe to the newsletter</CheckBox>
							<Slider
								label="Volume (stops at 80)"
								value={volume}
								hashMarks="bottom"
								hashMarkCount={11}
								minLabel="Quiet"
								maxLabel="Loud"
								onChange={(value) => setVolume(Math.min(value, 80))}
							/>
							<StatusBar label="Volume" trailingLabel={`${volume}%`} value={volume}/>
						</Box>
					</Group>
					<Box label="Fruits">
						<Scroll>
							<ListView items={fruits} multiple selection={picked} onSelectionChange={setPicked}/>
						</Scroll>
					</Box>
				</Split>
				<Box label="Notes">
					<Scroll maxHeight={80}>
						<TextView text={notes} onChange={setNotes}/>
					</Scroll>
				</Box>
				<Group orientation="horizontal">
					<StringView>{summary}</StringView>
					<Glue/>
					<Button onClick={quit}>Quit</Button>
				</Group>
			</Group>
		</Window>
	);
}

render(<App/>, {signature: 'application/x-vnd.HaikuReact-Form'});
