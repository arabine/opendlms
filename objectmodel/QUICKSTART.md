# 🚀 Guide de Démarrage Rapide - ATP Manager v2.0

## ✨ Nouvelles Fonctionnalités

Cette version améliore considérablement l'extraction et la gestion des tests ATP :

### 📊 Extraction Complète des Tableaux
- **Chaque ligne de tableau Word = une entrée dans la treeview**
- Extraction automatique de tous les test cases avec leurs détails complets
- Préservation de la structure des tableaux (colonnes et données)

### ✏️ Édition Complète (CRUD)
- **Ajouter** un nouveau test (chapitre, section, procédure, test case)
- **Modifier** n'importe quel test avec tous ses champs
- **Supprimer** un test avec confirmation
- Formulaire intuitif avec validation

## 🎯 Démarrage en 3 Étapes

### Étape 1 : Démarrer le Service Python

**Dans le terminal 1 :**
```bash
cd objectmodel
./start_parser_service.sh
```

Vous devriez voir :
```
✓ Python 3 trouvé
✓ Dépendances installées avec succès
🚀 Démarrage du serveur...
Service disponible sur: http://localhost:5000
```

⚠️ **Important** : Laissez ce terminal ouvert, le service doit rester actif.

### Étape 2 : Démarrer l'Application Web

**Dans un nouveau terminal (terminal 2) :**
```bash
cd objectmodel
npm install  # Première fois seulement
npm run dev
```

L'application s'ouvre sur `http://localhost:5173`

### Étape 3 : Charger et Gérer vos Tests

1. **Charger un fichier ATP** : Cliquez sur "Charger un fichier ATP" et sélectionnez votre .docx
2. **Explorer** : Naviguez dans l'arbre des tests
3. **Éditer** : Cliquez sur un test puis sur "Éditer"
4. **Ajouter** : Utilisez le bouton "➕ Ajouter un test"
5. **Supprimer** : Sélectionnez un test et cliquez sur "Supprimer"

## 📖 Utilisation Détaillée

### Charger un Fichier Word

```
┌──────────────────────────────────────┐
│  📁 Charger un fichier ATP           │
│  ────────────────────────────────    │
│  Sélectionnez votre fichier .docx    │
└──────────────────────────────────────┘
```

Le système extrait automatiquement :
- ✅ Tous les chapitres (ex: "7 Test Suites")
- ✅ Toutes les sections et procédures
- ✅ **Tous les tableaux ligne par ligne**
- ✅ Tous les détails des test cases

### Ajouter un Test

```
Clic sur "➕ Ajouter un test"
   ↓
Remplir le formulaire :
   • Type * : Chapitre / Section / Procédure / Test case
   • Titre * : Nom du test
   • Pour test case : Use Case, Scenario, Test Body, etc.
   ↓
Clic sur "Créer"
```

### Éditer un Test

```
Sélectionner un test dans l'arbre
   ↓
Clic sur "✏️ Éditer"
   ↓
Modifier les champs
   ↓
Clic sur "Mettre à jour"
```

### Supprimer un Test

```
Sélectionner un test
   ↓
Clic sur "🗑️ Supprimer"
   ↓
Confirmer
```

## 🎨 Interface

### Vue d'Ensemble

```
┌──────────────────────────────────────────────────────────┐
│  ATP Manager                                             │
├──────────────────────────────────────────────────────────┤
│  [📁 Charger fichier]              [➕ Ajouter un test]  │
├────────────────┬─────────────────────────────────────────┤
│  Arbre         │  Détails du Test                        │
│  ──────        │  ─────────────────                      │
│  📚 7          │  🧪 ACESM-UC06-TC01                     │
│  ├─ 📄 7.1     │  ───────────────────                    │
│  ├─ 📄 7.2     │  Remote connect/disconnect...           │
│  └─ 🧪 TC01    │                                          │
│                │  Données du tableau:                    │
│  📋 6          │  ┌──────────────────────────────┐       │
│  ├─ 📋 6.1     │  │ USE CASE: UC06               │       │
│  └─ 📋 6.2     │  │ SCENARIO: S01                │       │
│                │  │ TEST PURPOSE: Verification...│       │
│                │  └──────────────────────────────┘       │
│                │                                          │
│                │  [✏️ Éditer] [🗑️ Supprimer] [💾 Export] │
└────────────────┴─────────────────────────────────────────┘
```

## 🔍 Détails Techniques

### Fichiers Importants

```
objectmodel/
├── word_parser_service.py       ← Service Python extraction
├── start_parser_service.sh       ← Script de démarrage
├── src/
│   ├── services/
│   │   ├── atpParserService.ts    ← Appelle le service Python
│   │   └── atpDatabaseService.ts  ← Gestion CRUD
│   ├── components/
│   │   ├── AtpManager.vue         ← Composant principal
│   │   ├── AtpEditModal.vue       ← Modal d'édition
│   │   └── AtpDetailView.vue      ← Affichage détails
│   └── types/
│       └── index.ts               ← Types AtpTest étendus
└── README_ATP_FULL.md            ← Documentation complète
```

### Flux de Données

```
Fichier Word (.docx)
   ↓
Service Python (port 5000)
   • Extraction des tableaux avec python-docx
   • Une ligne de tableau = un test
   ↓
JSON structuré
   {
     testId: "ACESM-UC06-TC01",
     tableData: {...},  ← Toutes les colonnes
     useCase: "UC06",
     testBody: "..."
   }
   ↓
PouchDB (stockage local)
   ↓
Interface Vue.js
   • Arbre hiérarchique
   • Détails complets
   • Édition CRUD
```

## ❓ Résolution de Problèmes

### Le service Python ne démarre pas

**Erreur** : "command not found: python3"
```bash
# Installer Python 3
sudo apt install python3 python3-pip  # Linux
brew install python3                  # macOS
```

**Erreur** : "ModuleNotFoundError"
```bash
pip install flask flask-cors python-docx --break-system-packages
```

### Les tableaux ne sont pas extraits

1. **Vérifier que le service Python tourne** :
   ```bash
   curl http://localhost:5000/health
   # Doit retourner: {"status":"ok"}
   ```

2. **Voir les logs du service** : Les erreurs s'affichent dans le terminal 1

3. **Fallback** : Si le service Python n'est pas disponible, l'extraction texte basique fonctionne (sans tableaux détaillés)

### Erreur lors de la sauvegarde

**Message** : "Error updating test"
**Solution** : Recharger la page (F5) et réessayer

## 📚 Exemples d'Utilisation

### Exemple 1 : Ajouter un Nouveau Test Case

```
1. Clic sur "➕ Ajouter un test"
2. Remplir :
   Type: Test case
   Titre: "Test de connexion push"
   Test ID: "ACESM-CV-TC10"
   Use Case: "UC10"
   Scenario: "S01"
   Test Purpose: "Vérifier la connexion push..."
3. Clic sur "Créer"
```

### Exemple 2 : Modifier un Test Existant

```
1. Sélectionner "ACESM-UC06-TC01" dans l'arbre
2. Clic sur "✏️ Éditer"
3. Modifier "Test Body" pour ajouter des étapes
4. Clic sur "Mettre à jour"
```

### Exemple 3 : Supprimer un Chapitre

```
1. Sélectionner le chapitre "7" dans l'arbre
2. Clic sur "🗑️ Supprimer"
3. Confirmer dans la popup
4. Le chapitre et ses sous-éléments sont supprimés
```

## 🎓 Conseils d'Utilisation

### ✅ Bonnes Pratiques

1. **Toujours démarrer le service Python en premier**
2. **Remplir les champs obligatoires (*)** : Type, Titre
3. **Utiliser la hiérarchie** : Parent et Chapitre pour organiser
4. **Sauvegarder régulièrement** : Export JSON pour backup
5. **Nommer clairement** : Test ID et titres descriptifs

### ⚠️ À Éviter

- ❌ Ne pas arrêter le service Python pendant l'utilisation
- ❌ Ne pas oublier de remplir Type et Titre
- ❌ Ne pas créer de doublons de Test ID

## 📊 Statistiques et Export

### Voir les Statistiques

Le panneau de stats affiche :
- Total de tests
- Nombre de chapitres
- Nombre de sections
- Nombre de procédures
- Nombre de test cases

### Exporter en JSON

**Un seul test** :
```
1. Sélectionner le test
2. Clic sur "💾 Exporter en JSON"
3. Le fichier est téléchargé
```

**Tous les tests** :
```javascript
// Dans la console développeur (F12)
const tests = await atpDatabaseService.getAllTests()
console.log(JSON.stringify(tests, null, 2))
```

## 🎉 Résumé

### Ce Que Vous Pouvez Faire

✨ **Extraction automatique** de tous les tableaux Word  
✏️ **Créer** de nouveaux tests avec formulaire complet  
📝 **Modifier** n'importe quel test existant  
🗑️ **Supprimer** des tests avec confirmation  
📊 **Visualiser** les données de tableaux complètes  
💾 **Exporter** en JSON pour archivage  
🔍 **Rechercher** dans l'arbre des tests  

### Prochaines Étapes

1. ✅ Charger votre premier fichier ATP
2. ✅ Explorer l'extraction complète des tableaux
3. ✅ Tester l'ajout d'un nouveau test
4. ✅ Modifier un test existant
5. ✅ Organiser votre hiérarchie de tests

---

**🚀 Bon développement avec ATP Manager v2.0 !**

**Support** : Consultez README_ATP_FULL.md pour plus de détails  
**Version** : 2.0  
**Date** : 3 novembre 2025
