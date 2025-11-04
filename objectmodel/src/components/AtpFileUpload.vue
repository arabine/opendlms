<template>
  <div class="flex flex-col gap-3">
    <!-- Ligne des boutons -->
    <div class="flex flex-wrap items-center gap-2">
      <!-- Groupe de chargement -->
      <div class="flex items-center gap-2">
        <!-- Bouton Procédures -->
        <label
          for="atp-procedures-input"
          class="cursor-pointer bg-blue-600 hover:bg-blue-700 text-white font-medium py-2 px-3 rounded-md transition-colors duration-200 inline-flex items-center text-sm whitespace-nowrap"
          :class="{ 'opacity-50 cursor-not-allowed': loading }"
        >
          <svg class="w-4 h-4 sm:mr-1.5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 12h6m-6 4h6m2 5H7a2 2 0 01-2-2V5a2 2 0 012-2h5.586a1 1 0 01.707.293l5.414 5.414a1 1 0 01.293.707V19a2 2 0 01-2 2z" />
          </svg>
          <span class="hidden sm:inline">Procédures</span>
          <span class="sm:hidden">Proc.</span>
        </label>
        <input
          id="atp-procedures-input"
          type="file"
          accept=".md,.markdown,.html,.htm,.txt"
          class="hidden"
          :disabled="loading"
          @change="handleProceduresFileChange"
        />

        <!-- Bouton Test Cases -->
        <label
          for="atp-testcases-input"
          class="cursor-pointer bg-green-600 hover:bg-green-700 text-white font-medium py-2 px-3 rounded-md transition-colors duration-200 inline-flex items-center text-sm whitespace-nowrap"
          :class="{ 'opacity-50 cursor-not-allowed': loading }"
        >
          <svg class="w-4 h-4 sm:mr-1.5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 5H7a2 2 0 00-2 2v12a2 2 0 002 2h10a2 2 0 002-2V7a2 2 0 00-2-2h-2M9 5a2 2 0 002 2h2a2 2 0 002-2M9 5a2 2 0 012-2h2a2 2 0 012 2m-6 9l2 2 4-4" />
          </svg>
          <span class="hidden sm:inline">Test Cases</span>
          <span class="sm:hidden">Tests</span>
        </label>
        <input
          id="atp-testcases-input"
          type="file"
          accept=".md,.markdown,.html,.htm,.txt"
          class="hidden"
          :disabled="loading"
          @change="handleTestCasesFileChange"
        />
      </div>

      <!-- Séparateur -->
      <div class="hidden lg:block h-6 w-px bg-gray-300"></div>

      <!-- Groupe d'actions -->
      <div class="flex flex-wrap items-center gap-2">
      <button
        @click="parseAndSave"
        :disabled="(!proceduresFile && !testCasesFile) || loading || parsing"
        class="bg-green-600 hover:bg-green-700 text-white font-medium py-2 px-3 rounded-md transition-colors duration-200 disabled:bg-gray-400 disabled:cursor-not-allowed inline-flex items-center text-sm whitespace-nowrap"
      >
        <svg v-if="parsing" class="animate-spin h-4 w-4 sm:mr-1.5" fill="none" viewBox="0 0 24 24">
          <circle class="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" stroke-width="4"></circle>
          <path class="opacity-75" fill="currentColor" d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"></path>
        </svg>
        <svg v-else class="w-4 h-4 sm:mr-1.5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M5 13l4 4L19 7" />
        </svg>
        <span class="hidden sm:inline">{{ parsing ? 'Parsing...' : 'Parser' }}</span>
      </button>

      <!-- Bouton Import JSON -->
      <label
        for="json-import-input"
        class="cursor-pointer bg-purple-600 hover:bg-purple-700 text-white font-medium py-2 px-3 rounded-md transition-colors duration-200 inline-flex items-center text-sm whitespace-nowrap"
        :class="{ 'opacity-50 cursor-not-allowed': loading }"
      >
        <svg class="w-4 h-4 sm:mr-1.5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M7 16a4 4 0 01-.88-7.903A5 5 0 1115.9 6L16 6a5 5 0 011 9.9M15 13l-3-3m0 0l-3 3m3-3v12" />
        </svg>
        <span class="hidden sm:inline">Import JSON</span>
        <span class="sm:hidden">JSON</span>
      </label>
      <input
        id="json-import-input"
        type="file"
        accept=".json"
        class="hidden"
        :disabled="loading"
        @change="handleJsonImport"
      />

      <button
        @click="exportData"
        class="bg-gray-600 hover:bg-gray-700 text-white font-medium py-2 px-3 rounded-md transition-colors duration-200 inline-flex items-center text-sm whitespace-nowrap"
      >
        <svg class="w-4 h-4 sm:mr-1.5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 16v1a3 3 0 003 3h10a3 3 0 003-3v-1m-4-4l-4 4m0 0l-4-4m4 4V4" />
        </svg>
        <span class="hidden sm:inline">Export JSON</span>
        <span class="sm:hidden">JSON</span>
      </button>

      <button
        @click="exportToWord"
        class="bg-indigo-600 hover:bg-indigo-700 text-white font-medium py-2 px-3 rounded-md transition-colors duration-200 inline-flex items-center text-sm whitespace-nowrap"
      >
        <svg class="w-4 h-4 sm:mr-1.5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 12h6m-6 4h6m2 5H7a2 2 0 01-2-2V5a2 2 0 012-2h5.586a1 1 0 01.707.293l5.414 5.414a1 1 0 01.293.707V19a2 2 0 01-2 2z" />
        </svg>
        <span class="hidden sm:inline">Export Word</span>
        <span class="sm:hidden">Word</span>
      </button>

      <button
        @click="clearDatabase"
        class="bg-red-600 hover:bg-red-700 text-white font-medium py-2 px-3 rounded-md transition-colors duration-200 inline-flex items-center text-sm whitespace-nowrap"
      >
        <svg class="w-4 h-4 sm:mr-1.5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M19 7l-.867 12.142A2 2 0 0116.138 21H7.862a2 2 0 01-1.995-1.858L5 7m5 4v6m4-6v6m1-10V4a1 1 0 00-1-1h-4a1 1 0 00-1 1v3M4 7h16" />
        </svg>
        <span class="hidden sm:inline">Effacer</span>
      </button>
      </div>

      <!-- Messages (tooltip style) -->
      <div v-if="message" :class="[
        'px-3 py-1.5 rounded-md text-sm font-medium',
        message.type === 'success' ? 'bg-green-100 text-green-800' : '',
        message.type === 'error' ? 'bg-red-100 text-red-800' : '',
        message.type === 'info' ? 'bg-blue-100 text-blue-800' : ''
      ]">
        {{ message.text }}
      </div>
    </div>

    <!-- Noms des fichiers sélectionnés -->
    <div v-if="proceduresFile || testCasesFile" class="flex flex-wrap gap-3 text-xs text-gray-600">
      <div v-if="proceduresFile" class="flex items-center gap-1.5 px-2 py-1 bg-blue-50 rounded">
        <svg class="w-3.5 h-3.5 text-blue-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 12h6m-6 4h6m2 5H7a2 2 0 01-2-2V5a2 2 0 012-2h5.586a1 1 0 01.707.293l5.414 5.414a1 1 0 01.293.707V19a2 2 0 01-2 2z" />
        </svg>
        <span class="font-medium">Proc:</span>
        <span class="truncate max-w-[200px]" :title="proceduresFile.name">{{ proceduresFile.name }}</span>
      </div>
      <div v-if="testCasesFile" class="flex items-center gap-1.5 px-2 py-1 bg-green-50 rounded">
        <svg class="w-3.5 h-3.5 text-green-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 5H7a2 2 0 00-2 2v12a2 2 0 002 2h10a2 2 0 002-2V7a2 2 0 00-2-2h-2M9 5a2 2 0 002 2h2a2 2 0 002-2M9 5a2 2 0 012-2h2a2 2 0 012 2m-6 9l2 2 4-4" />
        </svg>
        <span class="font-medium">Tests:</span>
        <span class="truncate max-w-[200px]" :title="testCasesFile.name">{{ testCasesFile.name }}</span>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref } from 'vue'
import type { FileUploadAtpProps } from '@/types'
import { atpParserService } from '@/services/atpParserService'
import { atpDatabaseService } from '@/services/atpDatabaseService'

defineProps<FileUploadAtpProps>()

const emit = defineEmits<{
  (e: 'tests-updated'): void
}>()

const proceduresFile = ref<File | null>(null)
const testCasesFile = ref<File | null>(null)
const parsing = ref<boolean>(false)
const message = ref<{ type: 'success' | 'error' | 'info', text: string } | null>(null)

const handleProceduresFileChange = (event: Event): void => {
  const target = event.target as HTMLInputElement
  const file = target.files?.[0]
  
  if (file) {
    proceduresFile.value = file
    message.value = null
  }
}

const handleTestCasesFileChange = (event: Event): void => {
  const target = event.target as HTMLInputElement
  const file = target.files?.[0]
  
  if (file) {
    testCasesFile.value = file
    message.value = null
  }
}

const parseAndSave = async (): Promise<void> => {
  if (!proceduresFile.value && !testCasesFile.value) {
    message.value = { type: 'error', text: '❌ Veuillez charger au moins un fichier (procédures ou test cases)' }
    return
  }

  parsing.value = true
  message.value = { type: 'info', text: 'Parsing des fichiers en cours...' }

  try {
    let allTests: any[] = []

    // Parser les procédures si présentes
    if (proceduresFile.value) {
      message.value = { type: 'info', text: '📝 Parsing des procédures...' }
      const procedures = await atpParserService.parseFile(proceduresFile.value, 'procedures')
      allTests = allTests.concat(procedures)
      console.log(`✅ ${procedures.length} éléments parsés depuis le fichier procédures`)
      
      // Afficher le détail des types
      const procedureStats = procedures.reduce((acc, test) => {
        acc[test.type] = (acc[test.type] || 0) + 1
        return acc
      }, {} as Record<string, number>)
      console.log('  Types détectés:', procedureStats)
    }

    // Parser les test cases si présents
    if (testCasesFile.value) {
      message.value = { type: 'info', text: '✅ Parsing des test cases...' }
      const testCases = await atpParserService.parseFile(testCasesFile.value, 'test-cases')
      allTests = allTests.concat(testCases)
      console.log(`✅ ${testCases.length} éléments parsés depuis le fichier test cases`)
      
      // Afficher le détail des types
      const testCaseStats = testCases.reduce((acc, test) => {
        acc[test.type] = (acc[test.type] || 0) + 1
        return acc
      }, {} as Record<string, number>)
      console.log('  Types détectés:', testCaseStats)
    }

    message.value = { type: 'info', text: `${allTests.length} éléments trouvés. Enregistrement...` }

    // Effacer d'abord la base
    await atpDatabaseService.clearDatabase()

    // Insérer tous les tests
    const inserted = await atpDatabaseService.saveTests(allTests)
    
    // Compter les types
    const stats = allTests.reduce((acc, test) => {
      acc[test.type] = (acc[test.type] || 0) + 1
      return acc
    }, {} as Record<string, number>)
    
    const statsText = Object.entries(stats)
      .map(([type, count]) => `${count} ${type}`)
      .join(', ')
    
    message.value = { 
      type: 'success', 
      text: `✅ ${inserted} éléments enregistrés avec succès! (${statsText})` 
    }
    emit('tests-updated')

    // Effacer le message après 5 secondes
    setTimeout(() => {
      message.value = null
    }, 5000)
  } catch (error) {
    message.value = {
      type: 'error',
      text: `❌ Erreur: ${error instanceof Error ? error.message : String(error)}`
    }
    console.error('Error parsing ATP files:', error)
  } finally {
    parsing.value = false
  }
}

const exportData = async (): Promise<void> => {
  try {
    const json = await atpDatabaseService.exportToJson()
    const blob = new Blob([json], { type: 'application/json' })
    const url = URL.createObjectURL(blob)
    
    const a = document.createElement('a')
    a.href = url
    a.download = `dlms_tests_${new Date().toISOString().split('T')[0]}.json`
    a.click()
    
    URL.revokeObjectURL(url)
    message.value = { type: 'success', text: '✅ Export réussi' }
    
    setTimeout(() => {
      message.value = null
    }, 3000)
  } catch (error) {
    message.value = {
      type: 'error',
      text: `❌ Erreur lors de l'export: ${error instanceof Error ? error.message : String(error)}`
    }
  }
}

const clearDatabase = async (): Promise<void> => {
  if (!confirm('Êtes-vous sûr de vouloir effacer toute la base de données?')) {
    return
  }

  try {
    await atpDatabaseService.clearDatabase()
    message.value = { type: 'success', text: '✅ Base de données effacée' }
    emit('tests-updated')

    setTimeout(() => {
      message.value = null
    }, 3000)
  } catch (error) {
    message.value = {
      type: 'error',
      text: `❌ Erreur lors de l'effacement: ${error instanceof Error ? error.message : String(error)}`
    }
  }
}

const handleJsonImport = async (event: Event): Promise<void> => {
  const target = event.target as HTMLInputElement
  const file = target.files?.[0]

  if (!file) return

  try {
    message.value = { type: 'info', text: 'Lecture du fichier JSON...' }

    // Lire le fichier
    const text = await file.text()
    const importedData = JSON.parse(text)

    // Vérifier le format
    if (!Array.isArray(importedData)) {
      throw new Error('Le fichier JSON doit contenir un tableau de tests')
    }

    console.log(`Import JSON: ${importedData.length} tests trouvés dans le fichier`)

    // Récupérer les tests existants
    const existingTests = await atpDatabaseService.getAllTests()
    const existingIds = new Set(existingTests.map(t => t._id))

    console.log(`Import JSON: ${existingTests.length} tests déjà dans la base`)

    // Filtrer pour ne garder que les nouveaux tests et nettoyer les champs PouchDB
    const newTests = importedData
      .filter(test => {
        if (!test._id) {
          console.warn('Test sans _id ignoré:', test)
          return false
        }
        return !existingIds.has(test._id)
      })
      .map(test => {
        // Créer une copie sans les champs internes PouchDB
        const { _rev, ...cleanTest } = test
        return cleanTest
      })

    console.log(`Import JSON: ${newTests.length} nouveaux tests à importer`)

    if (newTests.length === 0) {
      message.value = { type: 'info', text: 'ℹ️ Aucun nouveau test à importer (tous existent déjà)' }
      setTimeout(() => {
        message.value = null
      }, 3000)
      return
    }

    message.value = { type: 'info', text: `Importation de ${newTests.length} nouveaux tests...` }

    // Importer les nouveaux tests
    const imported = await atpDatabaseService.saveTests(newTests)
    console.log(`Import JSON: ${imported} tests effectivement importés`)

    // Compter les types
    const stats = newTests.reduce((acc, test) => {
      acc[test.type] = (acc[test.type] || 0) + 1
      return acc
    }, {} as Record<string, number>)

    const statsText = Object.entries(stats)
      .map(([type, count]) => `${count} ${type}`)
      .join(', ')

    message.value = {
      type: 'success',
      text: `✅ ${imported} nouveaux tests importés! (${statsText}). ${importedData.length - newTests.length} existants ignorés.`
    }

    emit('tests-updated')

    // Effacer le message après 5 secondes
    setTimeout(() => {
      message.value = null
    }, 5000)

    // Réinitialiser l'input file
    target.value = ''
  } catch (error) {
    message.value = {
      type: 'error',
      text: `❌ Erreur lors de l'import: ${error instanceof Error ? error.message : String(error)}`
    }
    console.error('Error importing JSON:', error)

    // Réinitialiser l'input file
    target.value = ''
  }
}

const exportToWord = async (): Promise<void> => {
  try {
    message.value = { type: 'info', text: 'Génération du document Word...' }

    // Récupérer tous les tests
    const allTests = await atpDatabaseService.getAllTests()

    if (allTests.length === 0) {
      message.value = { type: 'error', text: '❌ Aucun test à exporter' }
      setTimeout(() => {
        message.value = null
      }, 3000)
      return
    }

    // Séparer par type
    const chapters = allTests.filter(t => t.type === 'chapter').sort((a, b) => (a.order || 0) - (b.order || 0))
    const sections = allTests.filter(t => t.type === 'section').sort((a, b) => (a.order || 0) - (b.order || 0))
    const testCases = allTests.filter(t => t.type === 'test-case').sort((a, b) => (a.order || 0) - (b.order || 0))
    const procedures = allTests.filter(t => t.type === 'procedure').sort((a, b) => (a.order || 0) - (b.order || 0))

    // Générer le HTML
    let html = `<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>ATP Tests Export</title>
  <style>
    body {
      font-family: Arial, Calibri, sans-serif;
      margin: 40px;
      line-height: 1.6;
    }
    h1 {
      color: #2c3e50;
      border-bottom: 3px solid #3498db;
      padding-bottom: 10px;
      margin-top: 30px;
    }
    h2 {
      color: #34495e;
      border-bottom: 2px solid #95a5a6;
      padding-bottom: 8px;
      margin-top: 25px;
    }
    h3 {
      color: #555;
      margin-top: 20px;
    }
    table {
      border-collapse: collapse;
      width: 100%;
      margin: 15px 0 30px 0;
      page-break-inside: avoid;
    }
    td, th {
      border: 1px solid #bdc3c7;
      padding: 10px;
      vertical-align: top;
      text-align: left;
    }
    td:first-child {
      width: 25%;
      font-weight: bold;
      background-color: #ecf0f1;
    }
    .test-id {
      color: #e74c3c;
      font-weight: bold;
    }
    .chapter-icon { color: #27ae60; }
    .section-icon { color: #f39c12; }
    .test-icon { color: #e74c3c; }
    .procedure-icon { color: #3498db; }
    .step {
      margin: 5px 0;
      padding-left: 20px;
    }
    pre {
      white-space: pre-wrap;
      word-wrap: break-word;
      background-color: #f8f9fa;
      padding: 10px;
      border-left: 3px solid #3498db;
    }
  </style>
</head>
<body>
  <h1>📋 ATP Tests Export</h1>
  <p><strong>Date d'export:</strong> ${new Date().toLocaleDateString('fr-FR', {
    year: 'numeric',
    month: 'long',
    day: 'numeric',
    hour: '2-digit',
    minute: '2-digit'
  })}</p>
  <p><strong>Total:</strong> ${allTests.length} tests (${chapters.length} chapitres, ${sections.length} sections, ${procedures.length} procédures, ${testCases.length} test cases)</p>
  <hr>
`

    // Section Test Cases
    if (testCases.length > 0 || chapters.length > 0) {
      html += '<h1>🧪 Test Cases</h1>\n'

      chapters.forEach(chapter => {
        html += `<h2 class="chapter-icon">📚 ${chapter.number || ''} ${chapter.title}</h2>\n`

        // Sections de ce chapitre
        const chapterSections = sections.filter(s => s.parent === chapter.number)

        chapterSections.forEach(section => {
          html += `<h3 class="section-icon">📄 ${section.number || ''} ${section.title}</h3>\n`

          // Tests de cette section
          const sectionTests = testCases.filter(t => t.parent === section.number)
          sectionTests.forEach(test => {
            html += generateTestCaseTable(test)
          })
        })

        // Tests directement dans le chapitre (sans section)
        const directTests = testCases.filter(t => t.chapter === chapter.number && !t.parent)
        directTests.forEach(test => {
          html += generateTestCaseTable(test)
        })
      })

      // Tests orphelins (sans chapitre)
      const orphanTests = testCases.filter(t => !t.chapter)
      if (orphanTests.length > 0) {
        html += '<h2>🧪 Test Cases (non classés)</h2>\n'
        orphanTests.forEach(test => {
          html += generateTestCaseTable(test)
        })
      }
    }

    // Section Procédures
    if (procedures.length > 0) {
      html += '<h1 class="procedure-icon">📋 Procédures</h1>\n'
      procedures.forEach(proc => {
        html += generateProcedureTable(proc)
      })
    }

    html += '</body></html>'

    // Télécharger
    const blob = new Blob([html], { type: 'text/html;charset=utf-8' })
    const url = URL.createObjectURL(blob)

    const a = document.createElement('a')
    a.href = url
    a.download = `ATP_Tests_${new Date().toISOString().split('T')[0]}.html`
    a.click()

    URL.revokeObjectURL(url)

    message.value = { type: 'success', text: '✅ Export Word réussi' }

    setTimeout(() => {
      message.value = null
    }, 3000)
  } catch (error) {
    message.value = {
      type: 'error',
      text: `❌ Erreur lors de l'export Word: ${error instanceof Error ? error.message : String(error)}`
    }
    console.error('Error exporting to Word:', error)
  }
}

function generateTestCaseTable(test: any): string {
  let html = `<h3 class="test-icon">🧪 <span class="test-id">${test.testId || test.number || ''}</span> ${test.title}</h3>\n`
  html += '<table>\n'

  if (test.useCase) {
    html += `  <tr><td>Use Case</td><td>${escapeHtml(test.useCase)}</td></tr>\n`
  }
  if (test.scenario) {
    html += `  <tr><td>Scenario</td><td>${escapeHtml(test.scenario)}</td></tr>\n`
  }
  if (test.testPurpose) {
    html += `  <tr><td>Test Purpose</td><td>${escapeHtml(test.testPurpose)}</td></tr>\n`
  }
  if (test.testStrategy) {
    html += `  <tr><td>Test Strategy</td><td>${escapeHtml(test.testStrategy)}</td></tr>\n`
  }
  if (test.aaFilter) {
    html += `  <tr><td>AA Filter</td><td>${escapeHtml(test.aaFilter)}</td></tr>\n`
  }
  if (test.prerequisites) {
    html += `  <tr><td>Prerequisites</td><td>${escapeHtml(test.prerequisites)}</td></tr>\n`
  }
  if (test.preamble) {
    html += `  <tr><td>Preamble</td><td><pre>${escapeHtml(test.preamble)}</pre></td></tr>\n`
  }

  // Test Body avec étapes
  if (test.testBodySteps && test.testBodySteps.length > 0) {
    const stepsHtml = test.testBodySteps
      .map((step: string, idx: number) => `<div class="step"><strong>Step ${idx + 1}:</strong> ${escapeHtml(step)}</div>`)
      .join('\n')
    html += `  <tr><td>Test Body</td><td>${stepsHtml}</td></tr>\n`
  } else if (test.testBody) {
    html += `  <tr><td>Test Body</td><td><pre>${escapeHtml(test.testBody)}</pre></td></tr>\n`
  }

  if (test.postamble) {
    html += `  <tr><td>Postamble</td><td><pre>${escapeHtml(test.postamble)}</pre></td></tr>\n`
  }
  if (test.expectedResult) {
    html += `  <tr><td>Expected Result</td><td>${escapeHtml(test.expectedResult)}</td></tr>\n`
  }
  if (test.comment) {
    html += `  <tr><td>Comment</td><td>${escapeHtml(test.comment)}</td></tr>\n`
  }

  html += '</table>\n'
  return html
}

function generateProcedureTable(proc: any): string {
  let html = `<h3 class="procedure-icon">📋 ${proc.number || ''} ${proc.title}</h3>\n`
  html += '<table>\n'

  if (proc.references) {
    html += `  <tr><td>References</td><td>${escapeHtml(proc.references)}</td></tr>\n`
  }
  if (proc.testPurpose) {
    html += `  <tr><td>Test Purpose</td><td>${escapeHtml(proc.testPurpose)}</td></tr>\n`
  }

  // Procedure Body avec étapes
  if (proc.procedureBody) {
    const steps = proc.procedureBody.split('\n\n').filter((s: string) => s.trim())
    if (steps.length > 1) {
      const stepsHtml = steps
        .map((step: string, idx: number) => `<div class="step"><strong>Étape ${idx + 1}:</strong> ${escapeHtml(step)}</div>`)
        .join('\n')
      html += `  <tr><td>Procedure Body</td><td>${stepsHtml}</td></tr>\n`
    } else {
      html += `  <tr><td>Procedure Body</td><td><pre>${escapeHtml(proc.procedureBody)}</pre></td></tr>\n`
    }
  }

  html += '</table>\n'
  return html
}

function escapeHtml(text: string): string {
  if (!text) return ''
  return text
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;')
    .replace(/'/g, '&#039;')
    .replace(/\n/g, '<br>')
}
</script>
