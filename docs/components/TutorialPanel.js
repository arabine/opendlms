
import { html } from 'htm/preact';
import { useState } from 'preact/hooks';

function TutorialPanel()
{
    return html`

    <div class="join join-vertical w-full">
  <div class="collapse collapse-arrow join-item border-base-300 border">
    <input type="radio" name="my-accordion-4" checked="checked" />
    <div class="collapse-title text-xl font-medium">Getting started with OpenDLMS</div>
    <div class="collapse-content">
      <p>OpenDLMS is an open source, MIT licensed DLMS/Cosem stack in clean C. This is a web-based server-less Meter Simulator that contains some basic objects.</p>
      <p>Everything in this webpage is available in the source code repository (scripts, tools, stack...). </p>
      <p>  <a href="https://www.github.com/arabine/opendlms" target="_blank" class="text-blue-500">Get the source code on Github</a>
      </p>
    </div>
  </div>
  <div class="collapse collapse-arrow join-item border-base-300 border">
    <input type="radio" name="my-accordion-4" />
    <div class="collapse-title text-xl font-medium">Starting the simulator</div>
    <div class="collapse-content">
      <p>The DLMS stack, server side, is running in the browser (through WASM compilation) and communicate using WebSocket. You need to start a gateway server to use a standard DLMS/Cosem client tool.</p>
      <img src="assets/gateway_diagram.png" />
      <p>The gateway needs <b>bun.js</b> (go to the website to install it) to run, just start the script located in docs/gateway.js by typing <b>bun gaetway.js</b>. Start in the following order:</p>
      <p class="m-4">
        <ol class="list-decimal">
            <li>Start the gateway</li>
            <li>Connect the Meter simulator using the button below</li>
            <li>Connect the DLMS/Client</li>
                
        </ol>
      </p>
    </div>
  </div>
  <div class="collapse collapse-arrow join-item border-base-300 border">
    <input type="radio" name="my-accordion-4" />
    <div class="collapse-title text-xl font-medium">Connection parameters</div>
    <div class="collapse-content">
      <p>The following Association is available:</p>
      <p>Public (SAP 16) and Management (SAP 1) with LLS password 00000001 (8 ASCII characters) </p>
    </div>
  </div>
</div>

    `
}

export default TutorialPanel;
