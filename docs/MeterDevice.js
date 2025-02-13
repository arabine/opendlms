import { html } from 'htm/preact';
import { useState } from 'preact/hooks';
import MeterEnclosure from './components/MeterEnclosure.js'
import LcdTopValue from './components/LcdTopValue.js'

function MeterDevice() {

    return html`
        <style>

        </style>
        <div style="position:relative; width: 400px; height: 600px;">
            
            <${MeterEnclosure} />
            <${LcdTopValue} x=${22} y=${9} w=${6} />

        </div>
    `;
}

/*

  
 
*/

export default MeterDevice;



