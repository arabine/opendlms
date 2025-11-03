<template>
  <div class="bg-blue-50 border border-blue-200 rounded-lg p-6 mb-6">
    <h2 class="text-xl font-semibold text-blue-900 mb-4">
      📋 Gestionnaire de Tests ATP
    </h2>
    
    <div class="space-y-4">
      <!-- Zone de chargement de fichier -->
      <div class="flex items-center gap-4">
        <label
          for="atp-file-input"
          class="cursor-pointer bg-blue-600 hover:bg-blue-700 text-white font-semibold py-2 px-4 rounded-lg transition-colors duration-200 inline-flex items-center"
          :class="{ 'opacity-50 cursor-not-allowed': loading }"
        >
          <svg class="w-5 h-5 mr-2" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M7 16a4 4 0 01-.88-7.903A5 5 0 1115.9 6L16 6a5 5 0 011 9.9M15 13l-3-3m0 0l-3 3m3-3v12" />
          </svg>
          Charger fichier ATP
        </label>
        <input
          id="atp-file-input"
          type="file"
          accept=".docx,.txt"
          class="hidden"
          :disabled="loading"
          @change="handleFileChange"
        />
        
        <span v-if="fileName" class="text-gray-600 text-sm">
          {{ fileName }}
        </span>
      </div>

      <!-- Boutons d'action -->
      <div class="flex gap-3">
        <button
          @click="parseAndSave"
          :disabled="!selectedFile || loading || parsing"
          class="bg-green-600 hover:bg-green-700 text-white font-semibold py-2 px-4 rounded-lg transition-colors duration-200 disabled:bg-gray-400 disabled:cursor-not-allowed inline-flex items-center"
        >
          <svg v-if="parsing" class="animate-spin h-5 w-5 mr-2" fill="none" viewBox="0 0 24 24">
            <circle class="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" stroke-width="4"></circle>
            <path class="opacity-75" fill="currentColor" d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"></path>
          </svg>
          <svg v-else class="w-5 h-5 mr-2" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M21 21l-6-6m2-5a7 7 0 11-14 0 7 7 0 0114 0z" />
          </svg>
          {{ parsing ? 'Parsing...' : 'Parser et Enregistrer' }}
        </button>

        <button
          @click="exportData"
          class="bg-gray-600 hover:bg-gray-700 text-white font-semibold py-2 px-4 rounded-lg transition-colors duration-200 inline-flex items-center"
        >
          <svg class="w-5 h-5 mr-2" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 16v1a3 3 0 003 3h10a3 3 0 003-3v-1m-4-4l-4 4m0 0l-4-4m4 4V4" />
          </svg>
          Exporter JSON
        </button>

        <button
          @click="clearDatabase"
          class="bg-red-600 hover:bg-red-700 text-white font-semibold py-2 px-4 rounded-lg transition-colors duration-200 inline-flex items-center"
        >
          <svg class="w-5 h-5 mr-2" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M19 7l-.867 12.142A2 2 0 0116.138 21H7.862a2 2 0 01-1.995-1.858L5 7m5 4v6m4-6v6m1-10V4a1 1 0 00-1-1h-4a1 1 0 00-1 1v3M4 7h16" />
          </svg>
          Effacer la base
        </button>
      </div>

      <!-- Messages -->
      <div v-if="message" :class="[
        'p-4 rounded-lg',
        message.type === 'success' ? 'bg-green-100 text-green-800 border border-green-200' : '',
        message.type === 'error' ? 'bg-red-100 text-red-800 border border-red-200' : '',
        message.type === 'info' ? 'bg-blue-100 text-blue-800 border border-blue-200' : ''
      ]">
        {{ message.text }}
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

const selectedFile = ref<File | null>(null)
const fileName = ref<string>('')
const parsing = ref<boolean>(false)
const message = ref<{ type: 'success' | 'error' | 'info', text: string } | null>(null)

const handleFileChange = (event: Event): void => {
  const target = event.target as HTMLInputElement
  const file = target.files?.[0]
  
  if (file) {
    selectedFile.value = file
    fileName.value = file.name
    message.value = null
  }
}

const parseAndSave = async (): Promise<void> => {
  if (!selectedFile.value) return

  parsing.value = true
  message.value = { type: 'info', text: 'Parsing du fichier en cours...' }

  try {
    const tests = await atpParserService.parseFile(selectedFile.value)
    message.value = { type: 'info', text: `${tests.length} éléments trouvés. Enregistrement...` }

    // Effacer d'abord la base
    await atpDatabaseService.clearDatabase()

    // Insérer les nouveaux tests
    const inserted = await atpDatabaseService.saveTests(tests)
    
    message.value = { type: 'success', text: `✅ ${inserted} éléments enregistrés avec succès!` }
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
    console.error('Error parsing ATP file:', error)
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
