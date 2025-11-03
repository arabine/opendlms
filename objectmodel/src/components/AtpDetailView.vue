<template>
  <div v-if="test" class="h-full overflow-y-auto bg-white">
    <!-- En-tête -->
    <div :class="[
      'p-6 border-b-4',
      getHeaderClasses(test.type)
    ]">
      <div class="flex items-start justify-between mb-4">
        <div class="flex items-center gap-3">
          <span :class="getIconClasses(test.type)">
            {{ getIcon(test.type) }}
          </span>
          <div>
            <div class="text-2xl font-bold text-gray-900">
              {{ test.number || test.testId || 'N/A' }}
            </div>
            <div :class="[
              'mt-1 px-3 py-1 rounded-full text-xs font-semibold uppercase inline-block',
              getBadgeClasses(test.type)
            ]">
              {{ getTypeLabel(test.type) }}
            </div>
          </div>
        </div>
        
        <button
          @click="copyToClipboard"
          class="px-3 py-2 text-sm bg-gray-100 hover:bg-gray-200 rounded-md transition-colors duration-150 flex items-center gap-2"
          :title="copySuccess ? 'Copié !' : 'Copier les détails'"
        >
          <svg v-if="!copySuccess" class="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M8 16H6a2 2 0 01-2-2V6a2 2 0 012-2h8a2 2 0 012 2v2m-6 12h8a2 2 0 002-2v-8a2 2 0 00-2-2h-8a2 2 0 00-2 2v8a2 2 0 002 2z" />
          </svg>
          <svg v-else class="w-4 h-4 text-green-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M5 13l4 4L19 7" />
          </svg>
          {{ copySuccess ? 'Copié !' : 'Copier' }}
        </button>
      </div>

      <h2 class="text-xl font-semibold text-gray-800 leading-tight">
        {{ test.title }}
      </h2>
    </div>

    <!-- Contenu -->
    <div class="p-6 space-y-6">
      <!-- Informations principales -->
      <div class="grid grid-cols-2 gap-4">
        <div class="bg-gray-50 rounded-lg p-4">
          <div class="text-xs font-semibold text-gray-500 uppercase mb-1">Type</div>
          <div class="text-sm font-medium text-gray-900">{{ getTypeLabel(test.type) }}</div>
        </div>

        <div v-if="test.chapter" class="bg-gray-50 rounded-lg p-4">
          <div class="text-xs font-semibold text-gray-500 uppercase mb-1">Chapitre</div>
          <div class="text-sm font-medium text-gray-900">{{ test.chapter }}</div>
        </div>

        <div v-if="test.parent" class="bg-gray-50 rounded-lg p-4">
          <div class="text-xs font-semibold text-gray-500 uppercase mb-1">Parent</div>
          <div class="text-sm font-medium text-gray-900">{{ test.parent }}</div>
        </div>

        <div class="bg-gray-50 rounded-lg p-4" :class="{'col-span-2': !test.chapter && !test.parent}">
          <div class="text-xs font-semibold text-gray-500 uppercase mb-1">ID</div>
          <div class="text-sm font-mono text-gray-900 break-all">{{ test._id }}</div>
        </div>
      </div>

      <!-- Description complète -->
      <div class="border-t border-gray-200 pt-6">
        <h3 class="text-lg font-semibold text-gray-800 mb-3">Description</h3>
        <div class="prose prose-sm max-w-none">
          <p class="text-gray-700 leading-relaxed">
            {{ test.title }}
          </p>
        </div>
      </div>

      <!-- Données de tableau (si applicable) -->
      <div v-if="test.tableData || test.tableRows" class="border-t border-gray-200 pt-6">
        <h3 class="text-lg font-semibold text-gray-800 mb-3">Données du tableau</h3>
        
        <!-- Tableau clé-valeur (format 2 colonnes) -->
        <div v-if="test.tableData" class="space-y-3">
          <div 
            v-for="(value, key) in test.tableData" 
            :key="key"
            class="bg-gray-50 rounded-lg p-4"
          >
            <div class="text-xs font-semibold text-gray-500 uppercase mb-2">{{ key }}</div>
            <div class="text-sm text-gray-900 whitespace-pre-wrap">{{ value || 'N/A' }}</div>
          </div>
        </div>

        <!-- Tableau multi-colonnes (format brut) -->
        <div v-else-if="test.tableRows && test.tableColumns" class="overflow-x-auto">
          <table class="min-w-full divide-y divide-gray-200">
            <thead class="bg-gray-100">
              <tr>
                <th 
                  v-for="(col, idx) in test.tableColumns" 
                  :key="idx"
                  class="px-4 py-3 text-left text-xs font-semibold text-gray-700 uppercase tracking-wider"
                >
                  {{ col }}
                </th>
              </tr>
            </thead>
            <tbody class="bg-white divide-y divide-gray-200">
              <tr v-for="(row, rowIdx) in test.tableRows" :key="rowIdx">
                <td 
                  v-for="(cell, cellIdx) in row" 
                  :key="cellIdx"
                  class="px-4 py-3 text-sm text-gray-900 whitespace-pre-wrap"
                >
                  {{ cell }}
                </td>
              </tr>
            </tbody>
          </table>
        </div>
      </div>

      <!-- Champs détaillés de la PROCÉDURE (si applicable) -->
      <div v-if="test.type === 'procedure' && hasProcedureFields" class="border-t border-gray-200 pt-6">
        <h3 class="text-lg font-semibold text-gray-800 mb-3">Détails de la procédure</h3>
        <div class="space-y-4">
          <div v-if="test.references" class="bg-blue-50 rounded-lg p-4">
            <div class="text-xs font-semibold text-blue-600 uppercase mb-2">References</div>
            <div class="text-sm text-gray-900">{{ test.references }}</div>
          </div>
          <div v-if="test.testPurpose" class="bg-blue-50 rounded-lg p-4">
            <div class="text-xs font-semibold text-blue-600 uppercase mb-2">Test Purpose</div>
            <div class="text-sm text-gray-900 whitespace-pre-wrap">{{ test.testPurpose }}</div>
          </div>
          <div v-if="test.procedureBody" class="bg-blue-50 rounded-lg p-4">
            <div class="text-xs font-semibold text-blue-600 uppercase mb-2">Procedure Body</div>
            <div class="text-sm text-gray-900 whitespace-pre-wrap font-mono">{{ test.procedureBody }}</div>
          </div>
        </div>
      </div>

      <!-- Champs détaillés du TEST CASE (si applicable) -->
      <div v-if="test.type === 'test-case' && hasDetailedFields" class="border-t border-gray-200 pt-6">
        <h3 class="text-lg font-semibold text-gray-800 mb-3">Détails du test case</h3>
        <div class="space-y-4">
          <div v-if="test.useCase" class="bg-green-50 rounded-lg p-4">
            <div class="text-xs font-semibold text-green-600 uppercase mb-2">Use Case</div>
            <div class="text-sm text-gray-900">{{ test.useCase }}</div>
          </div>
          <div v-if="test.scenario" class="bg-green-50 rounded-lg p-4">
            <div class="text-xs font-semibold text-green-600 uppercase mb-2">Scenario</div>
            <div class="text-sm text-gray-900">{{ test.scenario }}</div>
          </div>
          <div v-if="test.step" class="bg-green-50 rounded-lg p-4">
            <div class="text-xs font-semibold text-green-600 uppercase mb-2">Step</div>
            <div class="text-sm text-gray-900">{{ test.step }}</div>
          </div>
          <div v-if="test.references" class="bg-green-50 rounded-lg p-4">
            <div class="text-xs font-semibold text-green-600 uppercase mb-2">References</div>
            <div class="text-sm text-gray-900">{{ test.references }}</div>
          </div>
          <div v-if="test.testPurpose" class="bg-green-50 rounded-lg p-4">
            <div class="text-xs font-semibold text-green-600 uppercase mb-2">Test Purpose</div>
            <div class="text-sm text-gray-900 whitespace-pre-wrap">{{ test.testPurpose }}</div>
          </div>
          <div v-if="test.testStrategy" class="bg-green-50 rounded-lg p-4">
            <div class="text-xs font-semibold text-green-600 uppercase mb-2">Test Strategy</div>
            <div class="text-sm text-gray-900 whitespace-pre-wrap">{{ test.testStrategy }}</div>
          </div>
          <div v-if="test.aaFilter" class="bg-green-50 rounded-lg p-4">
            <div class="text-xs font-semibold text-green-600 uppercase mb-2">AA Filter</div>
            <div class="text-sm text-gray-900">{{ test.aaFilter }}</div>
          </div>
          <div v-if="test.prerequisites" class="bg-green-50 rounded-lg p-4">
            <div class="text-xs font-semibold text-green-600 uppercase mb-2">Prerequisites</div>
            <div class="text-sm text-gray-900 whitespace-pre-wrap">{{ test.prerequisites }}</div>
          </div>
          <div v-if="test.expectedResult" class="bg-green-50 rounded-lg p-4">
            <div class="text-xs font-semibold text-green-600 uppercase mb-2">Expected Result</div>
            <div class="text-sm text-gray-900 whitespace-pre-wrap">{{ test.expectedResult }}</div>
          </div>
          <div v-if="test.preamble" class="bg-green-50 rounded-lg p-4">
            <div class="text-xs font-semibold text-green-600 uppercase mb-2">Preamble</div>
            <div class="text-sm text-gray-900 whitespace-pre-wrap">{{ test.preamble }}</div>
          </div>
          
          <!-- Test Body avec étapes séparées -->
          <div v-if="test.testBodySteps && test.testBodySteps.length > 0" class="bg-green-50 rounded-lg p-4">
            <div class="text-xs font-semibold text-green-600 uppercase mb-3">Test Body Steps</div>
            <div class="space-y-2">
              <div
                v-for="(step, idx) in test.testBodySteps"
                :key="idx"
                class="bg-white rounded p-3 border-l-4 border-green-500"
              >
                <div class="text-xs font-semibold text-gray-500 mb-1">Step {{ idx + 1 }}</div>
                <div class="text-sm text-gray-900 whitespace-pre-wrap">{{ step }}</div>
              </div>
            </div>
          </div>
          <div v-else-if="test.testBody" class="bg-green-50 rounded-lg p-4">
            <div class="text-xs font-semibold text-green-600 uppercase mb-2">Test Body</div>
            <div class="text-sm text-gray-900 whitespace-pre-wrap">{{ test.testBody }}</div>
          </div>
          
          <div v-if="test.postamble" class="bg-green-50 rounded-lg p-4">
            <div class="text-xs font-semibold text-green-600 uppercase mb-2">Postamble</div>
            <div class="text-sm text-gray-900 whitespace-pre-wrap">{{ test.postamble }}</div>
          </div>
          <div v-if="test.comment" class="bg-green-50 rounded-lg p-4">
            <div class="text-xs font-semibold text-green-600 uppercase mb-2">Comment</div>
            <div class="text-sm text-gray-900 whitespace-pre-wrap">{{ test.comment }}</div>
          </div>
        </div>
      </div>

      <!-- Métadonnées -->
      <div class="border-t border-gray-200 pt-6">
        <h3 class="text-lg font-semibold text-gray-800 mb-3">Métadonnées</h3>
        <div class="space-y-2">
          <div class="flex items-center gap-2 text-sm">
            <span class="font-medium text-gray-600">Horodatage:</span>
            <span class="text-gray-900">{{ formatDate(test.timestamp) }}</span>
          </div>
          <div v-if="test._rev" class="flex items-center gap-2 text-sm">
            <span class="font-medium text-gray-600">Révision:</span>
            <span class="text-gray-900 font-mono text-xs">{{ test._rev }}</span>
          </div>
        </div>
      </div>

      <!-- Actions -->
      <div class="border-t border-gray-200 pt-6">
        <h3 class="text-lg font-semibold text-gray-800 mb-3">Actions</h3>
        <div class="flex gap-3 flex-wrap">
          <button
            @click="$emit('edit', test)"
            class="px-4 py-2 bg-blue-600 hover:bg-blue-700 text-white rounded-md transition-colors duration-150 flex items-center gap-2"
          >
            <svg class="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M11 5H6a2 2 0 00-2 2v11a2 2 0 002 2h11a2 2 0 002-2v-5m-1.414-9.414a2 2 0 112.828 2.828L11.828 15H9v-2.828l8.586-8.586z" />
            </svg>
            Éditer
          </button>

          <button
            @click="confirmDelete"
            class="px-4 py-2 bg-red-600 hover:bg-red-700 text-white rounded-md transition-colors duration-150 flex items-center gap-2"
          >
            <svg class="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M19 7l-.867 12.142A2 2 0 0116.138 21H7.862a2 2 0 01-1.995-1.858L5 7m5 4v6m4-6v6m1-10V4a1 1 0 00-1-1h-4a1 1 0 00-1 1v3M4 7h16" />
            </svg>
            Supprimer
          </button>

          <button
            @click="exportTest"
            class="px-4 py-2 bg-gray-600 hover:bg-gray-700 text-white rounded-md transition-colors duration-150 flex items-center gap-2"
          >
            <svg class="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 16v1a3 3 0 003 3h10a3 3 0 003-3v-1m-4-4l-4 4m0 0l-4-4m4 4V4" />
            </svg>
            Exporter en JSON
          </button>
        </div>
      </div>
    </div>
  </div>

  <!-- État vide -->
  <div v-else class="h-full flex items-center justify-center bg-gray-50">
    <div class="text-center">
      <svg class="w-24 h-24 mx-auto mb-4 text-gray-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
        <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 12h6m-6 4h6m2 5H7a2 2 0 01-2-2V5a2 2 0 012-2h5.586a1 1 0 01.707.293l5.414 5.414a1 1 0 01.293.707V19a2 2 0 01-2 2z" />
      </svg>
      <p class="text-lg font-semibold text-gray-600 mb-2">Aucun test sélectionné</p>
      <p class="text-sm text-gray-500">Cliquez sur un élément dans l'arbre pour voir ses détails</p>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed } from 'vue'
import type { AtpTest, TestType } from '@/types'

const props = defineProps<{
  test: AtpTest | null
}>()

const emit = defineEmits<{
  edit: [test: AtpTest]
  delete: [test: AtpTest]
}>()

const copySuccess = ref<boolean>(false)

const hasProcedureFields = computed(() => {
  if (!props.test || props.test.type !== 'procedure') return false
  return !!(
    props.test.references ||
    props.test.testPurpose ||
    props.test.procedureBody
  )
})

const hasDetailedFields = computed(() => {
  if (!props.test || props.test.type !== 'test-case') return false
  return !!(
    props.test.useCase ||
    props.test.scenario ||
    props.test.step ||
    props.test.references ||
    props.test.testPurpose ||
    props.test.testStrategy ||
    props.test.aaFilter ||
    props.test.prerequisites ||
    props.test.expectedResult ||
    props.test.preamble ||
    props.test.testBody ||
    props.test.testBodySteps ||
    props.test.postamble ||
    props.test.comment
  )
})

const getIcon = (type: TestType): string => {
  const icons = {
    'chapter': '📚',
    'section': '📄',
    'procedure': '📋',
    'test-case': '🧪'
  }
  return icons[type] || '📝'
}

const getIconClasses = (type: TestType): string => {
  return 'text-4xl'
}

const getHeaderClasses = (type: TestType): string => {
  const classes = {
    'chapter': 'border-green-500 bg-green-50',
    'section': 'border-yellow-500 bg-yellow-50',
    'procedure': 'border-cyan-500 bg-cyan-50',
    'test-case': 'border-red-500 bg-red-50'
  }
  return classes[type] || 'border-gray-500 bg-gray-50'
}

const getBadgeClasses = (type: TestType): string => {
  const classes = {
    'chapter': 'bg-green-600 text-white',
    'section': 'bg-yellow-600 text-white',
    'procedure': 'bg-cyan-600 text-white',
    'test-case': 'bg-red-600 text-white'
  }
  return classes[type] || 'bg-gray-600 text-white'
}

const getTypeLabel = (type: TestType): string => {
  const labels = {
    'chapter': 'Chapitre',
    'section': 'Section',
    'procedure': 'Procédure',
    'test-case': 'Cas de test'
  }
  return labels[type] || type
}

const formatDate = (timestamp: string): string => {
  return new Date(timestamp).toLocaleString('fr-FR', {
    year: 'numeric',
    month: 'long',
    day: 'numeric',
    hour: '2-digit',
    minute: '2-digit'
  })
}

const copyToClipboard = async () => {
  if (!props.test) return

  const text = JSON.stringify(props.test, null, 2)
  
  try {
    await navigator.clipboard.writeText(text)
    copySuccess.value = true
    setTimeout(() => {
      copySuccess.value = false
    }, 2000)
  } catch (err) {
    console.error('Erreur lors de la copie:', err)
  }
}

const confirmDelete = () => {
  if (!props.test) return

  if (confirm(`Êtes-vous sûr de vouloir supprimer "${props.test.title}" ?`)) {
    emit('delete', props.test)
  }
}

const exportTest = () => {
  if (!props.test) return

  const json = JSON.stringify(props.test, null, 2)
  const blob = new Blob([json], { type: 'application/json' })
  const url = URL.createObjectURL(blob)
  
  const a = document.createElement('a')
  a.href = url
  a.download = `${props.test._id}.json`
  a.click()
  
  URL.revokeObjectURL(url)
}
</script>
