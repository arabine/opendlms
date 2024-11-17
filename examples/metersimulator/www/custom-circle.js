class CustomCircle extends HTMLElement {
  constructor() {
    super();

    // Créez un Shadow DOM pour encapsuler le style et la structure du composant
    this.attachShadow({ mode: 'open' });

    // Création du SVG avec un cercle
    this.shadowRoot.innerHTML = `
      <style>
        svg {
          width: 100px;
          height: 100px;
        }
      </style>
      <svg width="100" height="100">
        <circle id="cercle" cx="50" cy="50" r="40" fill="blue"></circle>
      </svg>
      <button id="changeColorButton">Changer Couleur</button>
    `;
  }

  connectedCallback() {
    // Récupérer le bouton et ajouter un écouteur pour changer la couleur
    this.shadowRoot.getElementById('changeColorButton').addEventListener('click', () => {
      this.changerCouleur('red');  // Exemple pour changer en rouge
    });
  }

  // Méthode pour changer la couleur du cercle
  changerCouleur(couleur) {
    const cercle = this.shadowRoot.getElementById('cercle');
    cercle.setAttribute('fill', couleur);
  }
}

// Déclarer le Web Component sous le nom "custom-circle"
customElements.define('custom-circle', CustomCircle);
