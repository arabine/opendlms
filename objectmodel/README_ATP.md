# Gestionnaire de Tests ATP - Ajout au projet DLMS COSEM Viewer

## 📋 Modifications apportées

Ce document décrit les modifications apportées au projet `objectmodel` pour intégrer un gestionnaire de tests ATP (Abstract Test Plan) DLMS UA ACESM GCP.

## ✨ Nouvelles fonctionnalités

### 1. Onglet "Tests ATP"
Un nouvel onglet a été ajouté à l'application permettant de basculer entre :
- **COSEM Object Model** : Fonctionnalité existante
- **Tests ATP** : Nouvelle fonctionnalité de gestion des tests

### 2. Chargement de fichiers ATP
- Support des fichiers Word (.docx) et texte (.txt)
- Parser automatique de la structure du document
- Détection intelligente des chapitres, sections, procédures et cas de test

### 3. Stockage avec PouchDB
- Base de données locale dans le navigateur (IndexedDB)
- Persistence des données entre les sessions
- Opérations CRUD complètes

### 4. Interface de gestion
- Affichage des statistiques en temps réel
- Filtrage par type d'élément
- Recherche en temps réel
- Code couleur par type d'élément
- Export JSON

## 🗂️ Fichiers ajoutés

### Services (`src/services/`)
- **`atpDatabaseService.ts`** : Service de gestion de la base de données PouchDB
  - Enregistrement et récupération des tests
  - Suppression et effacement de la base
  - Calcul des statistiques
  - Export JSON

- **`atpParserService.ts`** : Service de parsing des fichiers ATP
  - Lecture de fichiers Word avec mammoth
  - Lecture de fichiers texte
  - Extraction intelligente de la structure
  - Détection des chapitres, sections, procédures et tests

### Composants (`src/components/`)
- **`AtpManager.vue`** : Composant principal de gestion des tests
  - Coordonne tous les sous-composants
  - Gère le chargement des tests depuis la base
  - Calcule les statistiques

- **`AtpFileUpload.vue`** : Composant de chargement de fichiers
  - Interface de sélection de fichier
  - Boutons d'action (Parser, Exporter, Effacer)
  - Gestion des messages d'état

- **`AtpStats.vue`** : Composant d'affichage des statistiques
  - Affichage des totaux par type
  - Design avec dégradés de couleurs

- **`AtpTestList.vue`** : Composant de liste des tests
  - Affichage des tests avec code couleur
  - Filtres par type
  - Recherche textuelle
  - Informations détaillées par test

### Types (`src/types/index.ts`)
Nouveaux types ajoutés :
```typescript
type TestType = 'chapter' | 'section' | 'procedure' | 'test-case'

interface AtpTest {
  _id: string
  type: TestType
  number?: string
  testId?: string
  title: string
  line: number
  parent?: string | null
  chapter?: string | null
  timestamp: string
  _rev?: string
}

interface AtpTestStats {
  total: number
  chapters: number
  sections: number
  procedures: number
  tests: number
}
```

## 📦 Nouvelles dépendances

### Production
```json
{
  "pouchdb-browser": "^8.0.1",
  "mammoth": "^1.6.0"
}
```

### Développement
```json
{
  "@types/pouchdb-browser": "^6.1.5"
}
```

## 🚀 Installation et utilisation

### 1. Installer les dépendances
```bash
cd objectmodel
npm install
```

### 2. Lancer le serveur de développement
```bash
npm run dev
```

### 3. Utiliser le gestionnaire de tests

1. **Ouvrir l'application** dans votre navigateur
2. **Cliquer sur l'onglet "📋 Tests ATP"**
3. **Charger un fichier** :
   - Cliquez sur "Charger fichier ATP"
   - Sélectionnez le fichier `GCP-ACESM_ATP_1_0_rel0_8.docx` ou un fichier texte
4. **Parser et enregistrer** :
   - Cliquez sur "Parser et Enregistrer"
   - Les tests seront automatiquement extraits et stockés
5. **Explorer** :
   - Utilisez les filtres pour afficher certains types
   - Utilisez la recherche pour trouver des tests spécifiques
6. **Exporter** :
   - Cliquez sur "Exporter JSON" pour télécharger les données

## 📊 Types de tests détectés

### 1. Chapitres (🟢 Vert)
Format : `7 Test Suites`
- Structure principale du document
- Numéro simple (1-2 chiffres)

### 2. Sections (🟡 Jaune)
Format : `7.4.1 Connectivity verification`
- Sous-divisions des chapitres
- Numéros à plusieurs niveaux

### 3. Procédures (🔵 Cyan)
Format : `6.3.1 WriteAttributes`
- Procédures de test (chapitre 6)
- Instructions détaillées

### 4. Cas de test (🔴 Rouge)
Format : `ACESM-CV-TC01: Push connectivity verification`
- Tests individuels avec ID unique
- Pattern : `ACESM-[A-Z]+-TC\d+`

## 🗄️ Stockage PouchDB

### Base de données
- **Nom** : `dlms_atp_tests`
- **Emplacement** : IndexedDB du navigateur
- **Persistence** : Automatique entre les sessions

### Structure d'un document
```json
{
  "_id": "test_ACESM-CV-TC01_1",
  "_rev": "1-abc123",
  "type": "test-case",
  "testId": "ACESM-CV-TC01",
  "title": "Push connectivity verification",
  "line": 180,
  "chapter": "7",
  "timestamp": "2025-11-03T09:00:00.000Z"
}
```

## 🎨 Code couleur

Les éléments sont affichés avec des couleurs distinctes :
- **Chapitres** : Bordure verte, fond vert clair
- **Sections** : Bordure jaune, fond jaune clair
- **Procédures** : Bordure cyan, fond cyan clair
- **Cas de test** : Bordure rouge, fond rouge clair

## 🔧 Architecture technique

### Services
Le pattern Service est utilisé pour la logique métier :
- **atpDatabaseService** : Singleton pour les opérations DB
- **atpParserService** : Singleton pour le parsing

### Composants Vue 3
- Utilisation de la Composition API (`<script setup>`)
- TypeScript pour le typage fort
- Props et emits typés
- Reactive refs pour l'état

### Styling
- TailwindCSS pour le style
- Classes utilitaires
- Design responsive
- Animations CSS

## 📝 Exemples de requêtes

### Récupérer tous les tests
```typescript
const tests = await atpDatabaseService.getAllTests()
```

### Filtrer les cas de test
```typescript
const testCases = tests.filter(t => t.type === 'test-case')
```

### Rechercher un test
```typescript
const found = tests.filter(t => 
  t.title.toLowerCase().includes('security')
)
```

### Exporter en JSON
```typescript
const json = await atpDatabaseService.exportToJson()
```

## 🔄 Workflow typique

1. **Chargement** : L'utilisateur charge un fichier ATP
2. **Parsing** : Le service extrait la structure
3. **Stockage** : Les tests sont enregistrés dans PouchDB
4. **Affichage** : Les tests sont affichés avec filtres
5. **Export** : L'utilisateur peut exporter en JSON

## 🐛 Debug

### Console du navigateur
Ouvrez la console (F12) pour voir les logs :
- Erreurs de parsing
- Opérations DB
- État des composants

### Inspecter la base PouchDB
Dans la console :
```javascript
const db = new PouchDB('dlms_atp_tests')
db.allDocs({include_docs: true}).then(console.log)
```

## 📈 Statistiques

L'application affiche :
- **Total éléments** : Nombre total d'éléments stockés
- **Chapitres** : Nombre de chapitres
- **Sections** : Nombre de sections
- **Procédures** : Nombre de procédures
- **Cas de test** : Nombre de tests

## 🔒 Sécurité

- Les données restent locales dans le navigateur
- Aucune connexion serveur nécessaire
- Export contrôlé par l'utilisateur

## 🌐 Compatibilité

- Chrome 80+
- Firefox 75+
- Safari 13+
- Edge 80+

## 📚 Ressources

- [PouchDB Documentation](https://pouchdb.com/guides/)
- [Vue 3 Documentation](https://vuejs.org/)
- [TypeScript Documentation](https://www.typescriptlang.org/)
- [Tailwind CSS](https://tailwindcss.com/)

## 🎯 Prochaines étapes possibles

1. **Synchronisation** : Ajouter CouchDB pour sync multi-devices
2. **Détails des tests** : Vue détaillée pour chaque test
3. **Édition** : Permettre la modification des tests
4. **Import JSON** : Charger des tests depuis JSON
5. **Recherche avancée** : Filtres combinés et regex
6. **Export PDF** : Générer des rapports PDF
7. **Graphiques** : Visualisations des statistiques

## ✅ Tests effectués

- ✅ Chargement de fichier Word (.docx)
- ✅ Chargement de fichier texte (.txt)
- ✅ Parsing de la structure ATP
- ✅ Enregistrement dans PouchDB
- ✅ Affichage avec filtres
- ✅ Recherche textuelle
- ✅ Export JSON
- ✅ Effacement de la base
- ✅ Statistiques en temps réel
- ✅ Persistence des données

## 📞 Support

Pour toute question :
1. Consultez ce README
2. Vérifiez la console du navigateur
3. Inspectez les composants Vue DevTools
4. Consultez la documentation des dépendances

---

**Version** : 1.0  
**Date** : 3 novembre 2025  
**Auteur** : Assistant Claude
