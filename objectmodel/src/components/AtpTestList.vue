<template>
  <div>
    <!-- Filtres -->
    <div class="bg-gray-50 border border-gray-200 rounded-lg p-4 mb-4 flex flex-wrap gap-4 items-center">
      <div class="flex items-center gap-2">
        <label for="filter-type" class="font-semibold text-gray-700">Type:</label>
        <select
          id="filter-type"
          v-model="localFilterType"
          class="border border-gray-300 rounded-md px-3 py-2 focus:outline-none focus:ring-2 focus:ring-blue-500"
          @change="emitFilterChange"
        >
          <option value="all">Tous</option>
          <option value="chapter">Chapitres</option>
          <option value="section">Sections</option>
          <option value="procedure">Procédures</option>
          <option value="test-case">Cas de test</option>
        </select>
      </div>

      <div class="flex items-center gap-2 flex-1">
        <label for="search-input" class="font-semibold text-gray-700">Recherche:</label>
        <input
          id="search-input"
          v-model="localSearchQuery"
          type="text"
          placeholder="Rechercher..."
          class="border border-gray-300 rounded-md px-3 py-2 flex-1 focus:outline-none focus:ring-2 focus:ring-blue-500"
          @input="emitSearchChange"
        />
      </div>
    </div>

    <!-- Liste des tests -->
    <div v-if="filteredTests.length === 0" class="text-center py-12 text-gray-500">
      <svg class="w-16 h-16 mx-auto mb-4 opacity-50" fill="none" stroke="currentColor" viewBox="0 0 24 24">
        <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 12h6m-6 4h6m2 5H7a2 2 0 01-2-2V5a2 2 0 012-2h5.586a1 1 0 01.707.293l5.414 5.414a1 1 0 01.293.707V19a2 2 0 01-2 2z" />
      </svg>
      <p class="text-lg font-semibold">Aucun test trouvé</p>
      <p class="text-sm">Chargez un fichier ATP pour commencer</p>
    </div>

    <div v-else class="space-y-3">
      <div
        v-for="test in filteredTests"
        :key="test._id"
        :class="[
          'border-l-4 rounded-lg p-4 transition-all duration-200 cursor-pointer hover:shadow-md',
          getTestClasses(test.type)
        ]"
      >
        <div class="flex justify-between items-start mb-2">
          <span class="font-bold text-lg text-gray-800">
            {{ test.number || test.testId || 'N/A' }}
          </span>
          <span :class="[
            'px-3 py-1 rounded-full text-xs font-semibold uppercase',
            getTypeBadgeClasses(test.type)
          ]">
            {{ getTypeLabel(test.type) }}
          </span>
        </div>
        
        <div class="text-gray-900 font-medium mb-2">
          {{ test.title }}
        </div>
        
        <div class="text-sm text-gray-600">
          Ligne: {{ test.line }}
          <span v-if="test.parent"> | Parent: {{ test.parent }}</span>
          <span v-if="test.chapter"> | Chapitre: {{ test.chapter }}</span>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed } from 'vue'
import type { AtpTest, TestType } from '@/types'

const props = defineProps<{
  tests: AtpTest[]
  filterType: TestType | 'all'
  searchQuery: string
}>()

const emit = defineEmits<{
  (e: 'update-filter-type', value: TestType | 'all'): void
  (e: 'update-search-query', value: string): void
}>()

const localFilterType = ref<TestType | 'all'>(props.filterType)
const localSearchQuery = ref<string>(props.searchQuery)

const filteredTests = computed(() => {
  let filtered = props.tests

  // Filtrer par type
  if (localFilterType.value !== 'all') {
    filtered = filtered.filter(test => test.type === localFilterType.value)
  }

  // Filtrer par recherche
  if (localSearchQuery.value) {
    const query = localSearchQuery.value.toLowerCase()
    filtered = filtered.filter(test => {
      const searchableText = `${test.title || ''} ${test.number || ''} ${test.testId || ''}`.toLowerCase()
      return searchableText.includes(query)
    })
  }

  // Trier par ligne
  return filtered.sort((a, b) => a.line - b.line)
})

const emitFilterChange = () => {
  emit('update-filter-type', localFilterType.value)
}

const emitSearchChange = () => {
  emit('update-search-query', localSearchQuery.value)
}

const getTestClasses = (type: TestType): string => {
  const classes = {
    'chapter': 'bg-green-50 border-green-500 hover:bg-green-100',
    'section': 'bg-yellow-50 border-yellow-500 hover:bg-yellow-100',
    'procedure': 'bg-cyan-50 border-cyan-500 hover:bg-cyan-100',
    'test-case': 'bg-red-50 border-red-500 hover:bg-red-100'
  }
  return classes[type] || 'bg-gray-50 border-gray-500'
}

const getTypeBadgeClasses = (type: TestType): string => {
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
    'test-case': 'Test'
  }
  return labels[type] || type
}
</script>
