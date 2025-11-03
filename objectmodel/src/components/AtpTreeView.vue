<template>
  <div class="h-full overflow-y-auto">
    <!-- Barre de recherche -->
    <div class="sticky top-0 bg-white p-4 border-b border-gray-200 z-10">
      <input
        v-model="localSearchQuery"
        type="text"
        placeholder="Rechercher..."
        class="w-full border border-gray-300 rounded-md px-3 py-2 focus:outline-none focus:ring-2 focus:ring-blue-500"
        @input="emitSearchChange"
      />
    </div>

    <div class="p-4">
      <!-- Chapitres et hiérarchie -->
      <div v-if="filteredChapters.length > 0" class="mb-6">
        <h3 class="text-sm font-semibold text-gray-600 uppercase mb-2">Structure des chapitres</h3>
        <div class="space-y-1">
          <TreeNodeItem
            v-for="chapter in filteredChapters"
            :key="chapter.id"
            :node="chapter"
            :selected-id="selectedTest?._id"
            :depth="0"
            @select="emitSelect"
            @toggle="emitToggle"
          />
        </div>
      </div>

      <!-- Procédures -->
      <div v-if="filteredProcedures.length > 0">
        <h3 class="text-sm font-semibold text-gray-600 uppercase mb-2">Procédures</h3>
        <div class="space-y-1">
          <div
            v-for="procedure in filteredProcedures"
            :key="procedure._id"
            @click="emitSelect(procedure)"
            :class="[
              'px-3 py-2 rounded cursor-pointer transition-colors duration-150',
              'flex items-center gap-2',
              selectedTest?._id === procedure._id
                ? 'bg-cyan-100 border-l-4 border-cyan-600'
                : 'hover:bg-gray-100'
            ]"
          >
            <span class="text-cyan-600 text-sm">📋</span>
            <div class="flex-1 min-w-0">
              <div class="text-sm font-medium text-gray-900 truncate">
                {{ procedure.number }}
              </div>
              <div class="text-xs text-gray-600 truncate">
                {{ procedure.title }}
              </div>
            </div>
          </div>
        </div>
      </div>

      <!-- État vide -->
      <div v-if="filteredChapters.length === 0 && filteredProcedures.length === 0" class="text-center py-8 text-gray-500">
        <p>Aucun résultat trouvé</p>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed } from 'vue'
import { atpTreeService } from '@/services/atpTreeService'
import type { AtpTreeNode, AtpTest } from '@/types'
import TreeNodeItem from './TreeNodeItem.vue'

const props = defineProps<{
  chapters: AtpTreeNode[]
  procedures: AtpTest[]
  selectedTest: AtpTest | null
  searchQuery: string
}>()

const emit = defineEmits<{
  (e: 'select', test: AtpTest): void
  (e: 'toggle', id: string): void
  (e: 'update-search', query: string): void
}>()

const localSearchQuery = ref<string>(props.searchQuery)

const filteredChapters = computed(() => {
  return atpTreeService.filterTree(props.chapters, localSearchQuery.value)
})

const filteredProcedures = computed(() => {
  return atpTreeService.filterProcedures(props.procedures, localSearchQuery.value)
})

const emitSelect = (test: AtpTest) => {
  emit('select', test)
}

const emitToggle = (id: string) => {
  emit('toggle', id)
}

const emitSearchChange = () => {
  emit('update-search', localSearchQuery.value)
}
</script>
