#!/bin/bash

# Script de démarrage du service d'extraction ATP
# Ce script installe les dépendances et démarre le serveur Flask

echo "==================================="
echo "  ATP Word Parser Service"
echo "==================================="
echo ""

# Vérifier Python
if ! command -v python3 &> /dev/null; then
    echo "❌ Python 3 n'est pas installé"
    exit 1
fi

echo "✓ Python 3 trouvé: $(python3 --version)"
echo ""

# Installer les dépendances
echo "📦 Installation des dépendances Python..."
pip install flask flask-cors python-docx --break-system-packages -q

if [ $? -eq 0 ]; then
    echo "✓ Dépendances installées avec succès"
else
    echo "❌ Erreur lors de l'installation des dépendances"
    exit 1
fi

echo ""
echo "🚀 Démarrage du serveur..."
echo ""
echo "Service disponible sur: http://localhost:5000"
echo "Endpoints:"
echo "  - GET  /health  : Health check"
echo "  - POST /parse   : Parser un fichier Word"
echo ""
echo "Appuyez sur Ctrl+C pour arrêter le serveur"
echo ""

# Démarrer le serveur
python3 word_parser_service.py
