import { html } from 'htm/preact';
import { useState } from 'preact/hooks';
import MeterEnclosure from './components/MeterEnclosure.js'
import LcdTopValue from './components/LcdTopValue.js'

function MeterDevice() {

    return html`
        <style>

        </style>
        <div>
            
                <${MeterEnclosure} />
                <${LcdTopValue} x=${130} y=${100} w=${25}/>
        </div>
    `;
}
export default MeterDevice;
