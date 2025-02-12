import { html } from 'htm/preact';
import SevenSegment from './SevenSegment.js';  // Assure-toi que SevenSegment est correctement importé

function LcdTopValue({ x = 0, y = 0, w = 30 }) {

    const segments = [
        { id: 'z1' },
        { id: 'z2' },
        { id: 'z3' },
        { id: 'z4' },
        { id: 'z5' },
        { id: 'z6' },
        { id: 'z7' },
        { id: 'z8' },
    ]

    // On crée un tableau de 8 éléments pour afficher 8 fois SevenSegment
    return html`
        <div style="display: flex; justify-content: space-between; gap: 10px;">
            ${segments.map((item, index) => html`
                <${SevenSegment} id=${item.id} key=${index} x=${index * (w + 2) + x} y=${y} w=${w}/>
            `)}
        </div>
    `;
}

export default LcdTopValue;
