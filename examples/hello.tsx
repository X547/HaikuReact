import {useState} from 'react';

import {Button, Group, render, StringView, Window} from '../src/index.js';

function App() {
	const [count, setCount] = useState(0);

	return (
		<Window title="Hello HaikuReact">
			<Group insets="window">
				<StringView>Clicked {count} times</StringView>
				<Group orientation="horizontal">
					<Button onClick={() => setCount(count + 1)}>Increment</Button>
					<Button enabled={count > 0} onClick={() => setCount(0)}>Reset</Button>
				</Group>
			</Group>
		</Window>
	);
}

render(<App/>, {signature: 'application/x-vnd.HaikuReact-Hello'});
