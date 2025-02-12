import { html } from 'htm/preact';
import { useState } from 'preact/hooks';


function SevenSegment({ x = 0, y = 0, w = 400 })
{
    const colorLightGreen = '#8baf7c';

    // États pour contrôler la visibilité et la couleur de chaque segment
    const [segments, setSegments] = useState({
        a: { visible: true, color: colorLightGreen },
        b: { visible: true, color: colorLightGreen },
        c: { visible: true, color: colorLightGreen },
        d: { visible: true, color: colorLightGreen },
        e: { visible: true, color: colorLightGreen },
        f: { visible: true, color: colorLightGreen },
        g: { visible: true, color: colorLightGreen },
        dpm: { visible: false, color: colorLightGreen }, // middle point
        dpb: { visible: false, color: colorLightGreen }  // bottom point
    });

    // Fonction pour changer l'état d'un segment
    const toggleSegment = (key) => {
        setSegments(prev => ({
            ...prev,
            [key]: {
                ...prev[key],
                visible: !prev[key].visible
            }
        }));
    };

    // Fonction pour changer la couleur
    const changeColor = (key, color) => {
        setSegments(prev => ({
            ...prev,
            [key]: {
                ...prev[key],
                color: color
            }
        }));
    };

    return html`
    <svg width="400"  viewBox="0 0 219.41 305.22" xml:space="preserve"
    style=${{ 
        position: 'absolute', 
        top: `${y}px`, 
        left: `${x}px`,
        width: `${w}px`
    }}
    xmlns="http://www.w3.org/2000/svg">
    <g id="layer1" transform="translate(2509.1 508.13)" stroke="#000" stroke-linecap="round" stroke-linejoin="round" stroke-width="4">
        <path id="a" fill=${segments.a.color}  stroke=${segments.a.color} d="m-2478.8-506.13h142.8l-34.83 33.464h-77.792z"/>
        <path id="b" fill=${segments.b.color}  stroke=${segments.b.color} d="m-2368-469.5 35.446-33.853-11.032 142.96-32.373-15.844z"/>
        <path id="c" fill=${segments.c.color}  stroke=${segments.c.color} d="m-2356.7-208.65-33.088-33.761 9.8829-94.155 35.099-15.814z"/>
        <path id="d" fill=${segments.d.color}  stroke=${segments.d.color} d="m-2502.7-205.36 143.09-.17961-34.147-34.666-75.712-1.2675z"/>
        <path id="e" fill=${segments.e.color}  stroke=${segments.e.color} d="m-2507.1-207.17 34.173-37.145 10.311-91.289-33.658-16.63z"/>
        <path id="f" fill=${segments.f.color}  stroke=${segments.f.color} d="m-2481.6-502.96 30.485 33.823-8.0402 92.741-34.966 17.106z"/>
        <path id="g" fill=${segments.g.color}  stroke=${segments.g.color} d="m-2492.5-355.23 37.434-18.311h74.597l34.346 16.996-36.83 16.524-76.16.9314z"/>
        <path id="dpm" fill=${segments.dpm.color}  stroke=${segments.dpm.color} d="m-2325.4-351.27h33.74l-3.7946 33.712h-32.88z"/>
        <path id="dpb" fill=${segments.dpb.color}  stroke=${segments.dpb.color}  d="m-2333.5-238.63h33.74l-3.7946 33.712h-32.88z"/>
    </g>
</svg>
    
    `
}

export default SevenSegment;
