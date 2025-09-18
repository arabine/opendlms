<template>
  <div class="mb-6">
    <div class="flex items-center space-x-4 mb-4">
      <div class="flex-1">
        <label for="search" class="block text-sm font-medium text-gray-700 mb-2">
          Rechercher dans l'arbre :
        </label>
        <input
          id="search"
          :value="searchQuery"
          @input="handleSearchInput"
          type="text"
          placeholder="Rechercher par nom d'objet, attribut, code OBIS ou version..."
          class="w-full px-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
        >
      </div>
      <div class="flex items-center space-x-2 mt-6">
        <button
          @click="$emit('update-search', '')"
          :disabled="!searchQuery"
          class="px-4 py-2 bg-gray-200 text-gray-700 rounded-lg hover:bg-gray-300 disabled:opacity-50 disabled:cursor-not-allowed"
        >
          Effacer
        </button>
      </div>
    </div>
    
    <div class="flex justify-between items-center text-sm text-gray-600">
      <span>{{ filteredObjectsCount }} objet(s) trouvé(s) sur {{ totalObjectsCount }}</span>
      <button
        v-if="filteredObjectsCount > 0"
        @click="$emit('toggle-all')"
        class="text-blue-600 hover:text-blue-800"
      >
        {{ allExpanded ? 'Réduire tout' : 'Développer tout' }}
      </button>
    </div>
  </div>
</template>

<script setup lang="ts">
import type { SearchFilterProps } from '@/types'


defineProps<SearchFilterProps>()

const emit = defineEmits<{
  'update-search': [query: string]
  'toggle-all': []
}>()

const handleSearchInput = (event: Event): void => {
  const target = event.target as HTMLInputElement
  emit('update-search', target.value)
}


</script>