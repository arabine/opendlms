<template>
  <div class="flex flex-col sm:flex-row flex-wrap items-start sm:items-center gap-3">
    <h2 class="text-lg font-semibold text-gray-900 hidden sm:block">
      📋 ATP Manager
    </h2>

    <!-- Groupe de chargement -->
    <div class="flex flex-wrap items-center gap-2">
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

      <button
        @click="exportData"
        class="bg-gray-600 hover:bg-gray-700 text-white font-medium py-2 px-3 rounded-md transition-colors duration-200 inline-flex items-center text-sm whitespace-nowrap"
      >
        <svg class="w-4 h-4 sm:mr-1.5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 16v1a3 3 0 003 3h10a3 3 0 003-3v-1m-4-4l-4 4m0 0l-4-4m4 4V4" />
        </svg>
        <span class="hidden sm:inline">Export</span>
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
      'w-full sm:w-auto px-3 py-1.5 rounded-md text-sm font-medium',
      message.type === 'success' ? 'bg-green-100 text-green-800' : '',
      message.type === 'error' ? 'bg-red-100 text-red-800' : '',
      message.type === 'info' ? 'bg-blue-100 text-blue-800' : ''
    ]">
      {{ message.text }}
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
</script>
