# ATP Manager - Version Complète avec Extraction de Tableaux et Édition

Version améliorée avec extraction complète des tableaux Word et fonctionnalités d'édition CRUD complètes.

## 🎯 Nouvelles Fonctionnalités

### ✨ Extraction Complète des Tableaux
- **Extraction automatique** de tous les tableaux Word avec leur structure
- **Une ligne de tableau = une entrée** dans la treeview
- **Préservation des données** : colonnes, lignes, et contenu
- **Support des formats** : tableaux 2 colonnes (test cases) et multi-colonnes

### ✏️ Édition Complète (CRUD)
- **Ajouter** un nouveau test (chapitre, section, procédure, test case)
- **Modifier** un test existant avec tous ses champs
- **Supprimer** un test avec confirmation
- **Formulaire complet** avec validation des champs obligatoires

### 📊 Affichage Amélioré
- **Données de tableaux** affichées dans le panneau de détails
- **Champs détaillés** pour les test cases (Use Case, Scenario, Test Purpose, etc.)
- **Interface intuitive** avec notifications de succès/erreur

## 🚀 Installation et Démarrage

### 1. Installation des dépendances

```bash
cd objectmodel
npm install
```

### 2. Démarrage du service d'extraction Python

Le service Python est nécessaire pour extraire les tableaux des fichiers Word.

**Option A : Script automatique (recommandé)**
```bash
chmod +x start_parser_service.sh
./start_parser_service.sh
```

**Option B : Manuel**
```bash
# Installer les dépendances
pip install flask flask-cors python-docx --break-system-packages

# Démarrer le service
python3 word_parser_service.py
```

Le service sera disponible sur `http://localhost:5000`

### 3. Démarrage de l'application Web

Dans un nouveau terminal :

```bash
npm run dev
```

L'application sera disponible sur `http://localhost:5173`

## 📖 Utilisation

### Charger un Fichier ATP

1. Cliquez sur le bouton "Charger un fichier ATP"
2. Sélectionnez votre fichier Word (.docx)
3. Le fichier est automatiquement parsé et les tests sont affichés

**Ce qui est extrait :**
- ✅ Chapitres (ex: "7 Test Suites")
- ✅ Sections et sous-sections
- ✅ Procédures (sections commençant par "6.")
- ✅ Test cases (tableaux ACESM-XX-TCxx)
- ✅ **NOUVEAU** : Contenu complet des tableaux ligne par ligne

### Naviguer dans l'Arbre

- **Cliquer** sur un nœud pour afficher ses détails
- **Icônes** :
  - 📚 Chapitre
  - 📄 Section
  - 📋 Procédure
  - 🧪 Test case

### Ajouter un Test

1. Cliquez sur le bouton "➕ Ajouter un test"
2. Remplissez le formulaire :
   - **Type*** : Chapitre, Section, Procédure, ou Test case (obligatoire)
   - **Titre*** : Nom du test (obligatoire)
   - **Numéro** : Pour chapitres/sections/procédures (ex: "7.1")
   - **Test ID** : Pour test cases (ex: "ACESM-UC06-TC01")
   - **Chapitre** et **Parent** : Pour la hiérarchie
3. Pour les test cases, remplissez les champs détaillés :
   - Use Case, Scenario, Test Purpose
   - Test Strategy, Prerequisites
   - Preamble, Test Body, Postamble, Comment
4. Cliquez sur "Créer"

### Éditer un Test

1. Sélectionnez un test dans l'arbre
2. Cliquez sur le bouton "✏️ Éditer" dans le panneau de détails
3. Modifiez les champs souhaités
4. Cliquez sur "Mettre à jour"

### Supprimer un Test

1. Sélectionnez un test dans l'arbre
2. Cliquez sur le bouton "🗑️ Supprimer" dans le panneau de détails
3. Confirmez la suppression

## 🔧 Architecture Technique

### Services

#### `word_parser_service.py`
Service Flask Python pour l'extraction avancée des tableaux Word.

**Endpoints :**
- `GET /health` : Vérification du service
- `POST /parse` : Parser un fichier Word et extraire les tableaux

**Fonctionnalités :**
- Extraction des chapitres depuis les paragraphes
- Extraction complète des tableaux avec python-docx
- Détection automatique des test cases (format ACESM-XX-TCxx)
- Support des tableaux 2 colonnes (test cases détaillés)
- Support des tableaux multi-colonnes (listes de tests)

#### `atpParserService.ts`
Service TypeScript qui appelle le service Python.

**Méthode principale :**
```typescript
async parseFile(file: File): Promise<AtpTest[]>
```

**Fonctionnement :**
1. Envoie le fichier au service Python
2. Reçoit les tests parsés avec leurs données de tableaux
3. Ajoute les timestamps
4. Fallback sur l'ancien parsing texte si le service Python n'est pas disponible

#### `atpDatabaseService.ts`
Service de gestion de la base de données PouchDB.

**Nouvelles méthodes :**
```typescript
async updateTest(test: AtpTest): Promise<void>  // Mise à jour
async saveTest(test: AtpTest): Promise<void>    // Création
async deleteTest(id: string): Promise<void>     // Suppression
```

### Composants Vue

#### `AtpEditModal.vue`
Modal d'édition/création de tests.

**Features :**
- Formulaire complet avec tous les champs
- Validation des champs obligatoires
- Mode création/édition automatique
- Support de tous les types de tests

#### `AtpDetailView.vue`
Panneau de détails amélioré.

**Nouvelles features :**
- Affichage des données de tableaux (tableData, tableRows)
- Affichage des champs détaillés des test cases
- Boutons Éditer et Supprimer
- Émission d'événements pour l'édition/suppression

#### `AtpManager.vue`
Composant principal de gestion.

**Gestion complète :**
- Chargement des tests
- Affichage de l'arbre et des détails
- Gestion du modal d'édition
- Notifications de succès/erreur
- CRUD complet

### Types

#### `AtpTest`
Type TypeScript étendu pour supporter les données de tableaux.

**Nouveaux champs :**
```typescript
interface AtpTest {
  // ... champs existants
  
  // Données de tableau
  tableColumns?: string[]                    // En-têtes
  tableData?: Record<string, string>         // Clé-valeur (2 colonnes)
  tableRows?: string[][]                     // Brut (multi-colonnes)
  
  // Champs détaillés test case
  useCase?: string
  scenario?: string
  testPurpose?: string
  testStrategy?: string
  aaFilter?: string
  prerequisites?: string
  preamble?: string
  testBody?: string
  postamble?: string
  comment?: string
}
```

## 📊 Exemple de Flux de Données

```
1. Utilisateur charge un fichier Word
   ↓
2. Frontend envoie le fichier au service Python
   ↓
3. Python extrait :
   - Chapitres depuis les paragraphes
   - Tableaux avec python-docx
   - Test cases (ACESM-XX-TCxx)
   - Toutes les données de chaque ligne
   ↓
4. Retour JSON avec structure complète :
   {
     _id: "test_ACESM-UC06-TC01_123",
     type: "test-case",
     testId: "ACESM-UC06-TC01",
     title: "Remote connect/disconnect...",
     tableData: {
       "Use Case": "UC06",
       "Scenario": "S01",
       "Test purpose": "Verification of...",
       ...
     }
   }
   ↓
5. Sauvegarde dans PouchDB
   ↓
6. Affichage dans la treeview
```

## 🎨 Interface Utilisateur

### Panneau de Détails

```
╔═══════════════════════════════════════════════════════════════╗
║  🧪 ACESM-UC06-TC01                              [CAS DE TEST] ║ ← En-tête coloré
║  ───────────────────────────────────────────────────────       ║
║  Remote connect/disconnect of breaker without arbitrator      ║
║                                                                ║
║  ┌─────────────────────┬─────────────────────────────────┐    ║
║  │  Type               │  Ligne                          │    ║
║  │  Cas de test        │  -1                             │    ║
║  └─────────────────────┴─────────────────────────────────┘    ║
║                                                                ║
║  Données du tableau                                            ║
║  ────────────────                                              ║
║  ┌────────────────────────────────────────────────────────┐   ║
║  │  USE CASE                                              │   ║
║  │  UC06                                                  │   ║
║  └────────────────────────────────────────────────────────┘   ║
║  ┌────────────────────────────────────────────────────────┐   ║
║  │  SCENARIO                                              │   ║
║  │  S01                                                   │   ║
║  └────────────────────────────────────────────────────────┘   ║
║  ...                                                           ║
║                                                                ║
║  Actions                                                       ║
║  ────────                                                      ║
║  [✏️ Éditer]  [🗑️ Supprimer]  [💾 Exporter en JSON]           ║
║                                                                ║
╚════════════════════════════════════════════════════════════════╝
```

### Modal d'Édition

```
╔═══════════════════════════════════════════════════════════════╗
║  Éditer le test                                          [✕]  ║
║  ─────────────────────────────────────────────────────────    ║
║                                                                ║
║  Type *                                                        ║
║  ┌────────────────────────────────────────────────────────┐   ║
║  │ [v] Cas de test                                        │   ║
║  └────────────────────────────────────────────────────────┘   ║
║                                                                ║
║  Titre *                                                       ║
║  ┌────────────────────────────────────────────────────────┐   ║
║  │ Remote connect/disconnect of breaker...                │   ║
║  └────────────────────────────────────────────────────────┘   ║
║                                                                ║
║  Test ID                          Ligne                       ║
║  ┌──────────────────────┐         ┌──────────────────────┐   ║
║  │ ACESM-UC06-TC01      │         │ -1                   │   ║
║  └──────────────────────┘         └──────────────────────┘   ║
║                                                                ║
║  Détails du test case                                          ║
║  ─────────────────                                             ║
║  Use Case                         Scenario                    ║
║  ┌──────────────────────┐         ┌──────────────────────┐   ║
║  │ UC06                 │         │ S01                  │   ║
║  └──────────────────────┘         └──────────────────────┘   ║
║                                                                ║
║  Test Purpose                                                  ║
║  ┌────────────────────────────────────────────────────────┐   ║
║  │ Verification of the Disconnect Control object...       │   ║
║  │                                                         │   ║
║  └────────────────────────────────────────────────────────┘   ║
║  ...                                                           ║
║                                                                ║
║  ──────────────────────────────────────────────────────────   ║
║                                    [Annuler]  [✓ Mettre à jour] ║
╚═══════════════════════════════════════════════════════════════╝
```

## 🐛 Dépannage

### Le service Python ne démarre pas

**Erreur** : `command not found: python3`
**Solution** : Installer Python 3

**Erreur** : `ModuleNotFoundError: No module named 'flask'`
**Solution** :
```bash
pip install flask flask-cors python-docx --break-system-packages
```

### Les tableaux ne sont pas extraits

**Vérification** :
1. Le service Python est-il en cours d'exécution ?
   ```bash
   curl http://localhost:5000/health
   ```
2. Consulter les logs du service Python
3. Vérifier que le fichier Word contient bien des tableaux

**Fallback** : Si le service Python n'est pas disponible, l'application utilisera l'extraction texte basique (sans tableaux).

### Erreur lors de la sauvegarde

**Erreur** : `Error updating test`
**Cause** : Conflit de révision dans PouchDB
**Solution** : Recharger la page et réessayer

## 📝 Notes de Version

### Version 2.0 - Extraction et Édition Complètes

**Nouvelles fonctionnalités :**
- ✨ Extraction complète des tableaux Word ligne par ligne
- ✏️ Édition CRUD complète (Créer, Lire, Modifier, Supprimer)
- 📊 Affichage amélioré des données de tableaux
- 🔧 Service Python Flask pour l'extraction avancée
- 🎨 Modal d'édition avec formulaire complet
- 💾 Méthode updateTest dans atpDatabaseService
- 🔔 Notifications de succès/erreur

**Améliorations :**
- Interface utilisateur plus intuitive
- Gestion complète des types de tests
- Support des champs détaillés pour test cases
- Fallback automatique si service Python indisponible

### Version 1.0 - Version Initiale

- Extraction basique des tests depuis texte
- Affichage en arborescence
- Détails des tests
- Export JSON

## 🤝 Contribuer

Pour contribuer au projet :

1. Assurez-vous que le service Python est bien testé
2. Testez l'extraction sur différents fichiers Word ATP
3. Vérifiez que les opérations CRUD fonctionnent correctement
4. Documentez toute nouvelle fonctionnalité

## 📄 Licence

Ce projet fait partie de l'écosystème DLMS/COSEM et suit les mêmes licences.

---

**Version** : 2.0  
**Date** : 3 novembre 2025  
**Auteur** : Équipe DLMS/COSEM
