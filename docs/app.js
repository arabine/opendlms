// Preact
import { useContext, useEffect } from 'preact/hooks';
import { render } from 'preact';
import { html } from 'htm/preact';

// Components
import MeterDevice  from './MeterDevice.js'
import TopNavBar  from './components/TopNavBar.js'
import TutorialPanel from './components/TutorialPanel.js';

export function App() {


    // The useEffect hook with an empty dependency array simulates the componentDidMount lifecycle method. 
    // It runs the provided function after the component is first rendered. 
    // This is a good place to perform data fetching or initial setup.
    useEffect(() => {
        console.log("App ready");
        return () => {
            console.log("App unmount");
        }
    }, []);

    return html`

        <${TopNavBar} />

        <div class="flex">
        <${MeterDevice} />
        <${TutorialPanel} />
        </div>
    `;
}

render(html`<${App} />`, document.getElementById('app'));
