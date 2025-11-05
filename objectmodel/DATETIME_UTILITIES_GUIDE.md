# COSEM Date & Time Utilities - Guide d'utilisation

## Vue d'ensemble

L'onglet **Date Time Utilities** fournit une interface complète pour générer des chaînes d'octets (octet-string) COSEM conformes aux spécifications DLMS/COSEM pour les dates, heures et date-time.

## Formats supportés

### 1. Date (5 octets)
Format : `year_high | year_low | month | day_of_month | day_of_week`

### 2. Time (4 octets)
Format : `hour | minute | second | hundredths`

### 3. Date-Time (12 octets)
Format complet combinant date + time + deviation + clock_status

## Fonctionnalités

### Wildcards et valeurs spéciales

#### Pour Date :
- **Year** : `0xFFFF` = non spécifié (any year)
- **Month** :
  - `0xFF` = non spécifié (any month)
  - `0xFE` = début de l'heure d'été (daylight savings begin)
  - `0xFD` = fin de l'heure d'été (daylight savings end)
  - `1-12` = mois spécifique (1=Janvier)

- **Day of Month** :
  - `0xFF` = non spécifié
  - `0xFE` = dernier jour du mois
  - `0xFD` = avant-dernier jour du mois
  - `1-31` = jour spécifique

- **Day of Week** :
  - `0xFF` = non spécifié
  - `1-7` = jour spécifique (1=Lundi, 7=Dimanche)

#### Pour Time :
- **Hour** : `0xFF` = non spécifié, sinon `0-23`
- **Minute** : `0xFF` = non spécifié, sinon `0-59`
- **Second** : `0xFF` = non spécifié, sinon `0-59`
- **Hundredths** : `0xFF` = non spécifié, sinon `0-99`

#### Pour Date-Time :
- **Deviation** :
  - `0x8000` = non spécifié
  - `-720 à +720` = minutes de décalage par rapport à UTC
  - Exemples : UTC+1 = 60, UTC-5 = -300

- **Clock Status** (bits) :
  - Bit 0 : Invalid value
  - Bit 1 : Doubtful value
  - Bit 2 : Different clock base
  - Bit 3 : Invalid clock status
  - Bit 7 : Daylight saving active
  - `0xFF` = non spécifié

## Exemples d'utilisation

### 1. Dernier jour de chaque mois
**Configuration :**
- Year : Not specified (0xFFFF)
- Month : Not specified (0xFF)
- Day of Month : Last day (0xFE)
- Day of Week : Not specified (0xFF)

**Résultat :** `FFFFFFFF FEFF`

**Usage :** Planification mensuelle récurrente

### 2. Dernier dimanche de chaque mois
**Configuration :**
- Year : Not specified (0xFFFF)
- Month : Not specified (0xFF)
- Day of Month : Last day (0xFE)
- Day of Week : Sunday (7)

**Résultat :** `FFFFFFFF FE07`

**Usage :** Changements d'heure d'été/hiver

### 3. Dernier dimanche de mars (changement d'heure)
**Configuration :**
- Year : Not specified (0xFFFF)
- Month : March (3)
- Day of Month : Last day (0xFE)
- Day of Week : Sunday (7)

**Résultat :** `FFFF03 FE07`

**Usage :** Activation automatique de l'heure d'été en Europe

### 4. Tous les jours à minuit
**Configuration :**
- Date : tous wildcards
- Time : 00:00:00, hundredths not specified

**Résultat (date-time) :** `FFFFFFFFFFFF 00000FF 8000FF`

**Usage :** Tâche quotidienne à heure fixe

### 5. Date-time complète avec fuseau horaire
**Configuration :**
- Date complète : 2024-11-05 (mardi)
- Time : 14:30:00.00
- Deviation : UTC+1 (60 minutes)
- Clock Status : Normal (0x00)

**Résultat :** `07E80B0503 0E1E0000 003C00`

**Usage :** Horodatage précis avec fuseau horaire

## Règles d'interprétation

### Combinaisons dayOfMonth et dayOfWeek

1. **Last day + Wildcard dayOfWeek** : Dernier jour calendaire du mois
   - `dayOfMonth=0xFE, dayOfWeek=0xFF`

2. **Last day + dayOfWeek spécifique** : Dernière occurrence du jour dans le mois
   - `dayOfMonth=0xFE, dayOfWeek=7` = dernier dimanche

3. **Year wildcard + dates spécifiques** : Jour de la semaine suivant une date
   - `year=0xFFFF, month=3, dayOfMonth=22, dayOfWeek=5`
   - = premier vendredi à partir du 22 mars (quatrième vendredi)

4. **Year et month spécifiés + incohérence** : ERREUR
   - `year=2024, month=8, dayOfMonth=13 (mercredi), dayOfWeek=2 (mardi)`
   - = Erreur car incompatible

## Cas d'usage typiques

### Planification de tâches

#### Exécutions périodiques simples
- **Quotidien** : Year/Month/Day wildcards + Time fixe
  - Exemple : Tous les jours à 6h00 → `FFFFFFFFFFFF 06000FF`

- **Hebdomadaire** : Year/Month wildcards + DayOfWeek spécifique
  - Exemple : Tous les lundis à 9h00 → `FFFFFFFF FF01 09000FF`

- **Mensuel** : Year wildcard + Month spécifique ou Last day
  - Exemple : Premier jour du mois → `FFFFFF FF0101 00000FF`
  - Exemple : Dernier jour du mois → `FFFFFFFEFE FF 00000FF`

- **Annuel** : Year wildcard + date complète
  - Exemple : 1er janvier à minuit → `FFFF010101 00000FF`

#### Exécutions haute fréquence

**Important** : COSEM ne supporte pas directement les wildcards d'intervalles (ex: "toutes les 10 minutes"). Pour ces cas, utilisez la classe 22 Single Action Schedule avec `type=2` (SIZE_N_SAME_TIME_NO_WILDCARD) ou `type=4` (SIZE_N_DIFFERENT_TIME_NO_WILDCARD).

- **Toutes les heures** : Hour wildcard + Minute=00
  - Configuration : `hourNotSpecified=true, minute=0`
  - Résultat Time : `FF0000FF`
  - Utilisation : Single Action Schedule type 2 avec une seule entrée

- **Toutes les 15 minutes** : Nécessite 4 entrées distinctes
  - Entrée 1 : minute=00 → `FF0000FF`
  - Entrée 2 : minute=15 → `FF0F00FF`
  - Entrée 3 : minute=30 → `FF1E00FF`
  - Entrée 4 : minute=45 → `FF2D00FF`
  - Utilisation : Single Action Schedule type 4 avec array de 4 execution_time

- **Toutes les 10 minutes** : Nécessite 6 entrées distinctes
  - Minutes : 00, 10, 20, 30, 40, 50
  - Résultats : `FF0000FF`, `FF0A00FF`, `FF1400FF`, `FF1E00FF`, `FF2800FF`, `FF3200FF`
  - Utilisation : Single Action Schedule type 4 avec array de 6 execution_time

- **Toutes les 5 minutes** : Nécessite 12 entrées distinctes
  - Minutes : 00, 05, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55

- **Jours ouvrés (Lundi-Vendredi)** : 5 schedules séparés ou 5 entrées
  - Lundi (1), Mardi (2), Mercredi (3), Jeudi (4), Vendredi (5)
  - Chacun avec la même heure

**Exemple complet - Exécution toutes les 10 minutes** :

```json
{
  "executed_script": {
    "script_logical_name": "0-0:10.0.1.0",
    "script_selector": 1
  },
  "type": 4,  // SIZE_N_DIFFERENT_TIME_NO_WILDCARD
  "execution_time": [
    { "time": "FF0000FF", "date": "07FFFFFFFFFF0000FF" },  // :00
    { "time": "FF0A00FF", "date": "07FFFFFFFFFF0000FF" },  // :10
    { "time": "FF1400FF", "date": "07FFFFFFFFFF0000FF" },  // :20
    { "time": "FF1E00FF", "date": "07FFFFFFFFFF0000FF" },  // :30
    { "time": "FF2800FF", "date": "07FFFFFFFFFF0000FF" },  // :40
    { "time": "FF3200FF", "date": "07FFFFFFFFFF0000FF" }   // :50
  ]
}
```

### Changements d'heure
- **Début heure d'été** : Dernier dimanche de mars à 02:00
- **Fin heure d'été** : Dernier dimanche d'octobre à 03:00

### Horodatage
- **Event timestamp** : Date-time complète avec deviation et status

## Interface utilisateur

### Sections
1. **Format Selection** : Choisir entre Date, Time, ou Date-Time
2. **Date Configuration** : Paramétrer l'année, mois, jour du mois, jour de la semaine
3. **Time Configuration** : Paramétrer heure, minute, seconde, centièmes
4. **Timezone & Clock Status** : (Date-Time uniquement) Décalage UTC et flags d'état
5. **Generated Octet String** : Résultat hexadécimal avec bouton de copie
6. **Common Examples** : Exemples prédéfinis chargeables en un clic

### Helper visuel
Un panneau d'interprétation s'affiche automatiquement pour les dates avec wildcards, expliquant la signification exacte de la configuration.

## Références

- DLMS UA 1000-1 Part 2 Ed. 16
- Section 4.1.6.1 : Date and time formats
- Blue Book COSEM Interface Classes

## Intégration avec Single Action Schedule (Class 22)

Les chaînes d'octets générées peuvent être directement utilisées dans :
- **Attribut 4** (execution_time) de la classe 22
- Structures `execution_time_date` avec champs `time` et `date`

Exemple pour exécution quotidienne à 6h00 :
```json
{
  "time": "06000000FF",  // Généré par Time utilities
  "date": "07FFFFFFFFFF0000FF"  // Généré par Date utilities
}
```
