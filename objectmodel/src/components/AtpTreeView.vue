<template>
  <div class="h-full flex flex-col">
    <!-- Barre de recherche -->
    <div class="bg-white p-4 border-b border-gray-200">
      <input
        v-model="localSearchQuery"
        type="text"
        placeholder="Rechercher..."
        class="w-full border border-gray-300 rounded-md px-3 py-2 focus:outline-none focus:ring-2 focus:ring-blue-500"
        @input="emitSearchChange"
      />
    </div>

    <!-- Onglets -->
    <div class="bg-white border-b border-gray-200">
      <div class="flex">
        <button
          @click="activeTab = 'procedures'"
          :class="[
            'flex-1 px-4 py-3 text-sm font-medium transition-colors',
            activeTab === 'procedures'
              ? 'text-blue-600 border-b-2 border-blue-600 bg-blue-50'
              : 'text-gray-600 hover:text-gray-900 hover:bg-gray-50'
          ]"
        >
          📋 Procédures ({{ procedures.length }})
        </button>
        <button
          @click="activeTab = 'testcases'"
          :class="[
            'flex-1 px-4 py-3 text-sm font-medium transition-colors',
            activeTab === 'testcases'
              ? 'text-green-600 border-b-2 border-green-600 bg-green-50'
              : 'text-gray-600 hover:text-gray-900 hover:bg-gray-50'
          ]"
        >
          🧪 Test Cases ({{ countTestCases(testCaseTree) }})
        </button>
      </div>
    </div>

    <!-- Contenu des onglets -->
    <div class="flex-1 overflow-y-auto p-4">
      <!-- Onglet Procédures -->
      <div v-if="activeTab === 'procedures'">
        <div v-if="filteredProcedures.length > 0" class="space-y-1">
          <div
            v-for="procedure in filteredProcedures"
            :key="procedure.id"
            @click="emitSelect(procedure.test)"
            :class="[
              'px-3 py-2 rounded cursor-pointer transition-colors duration-150',
              'flex items-center gap-2',
              selectedTest?._id === procedure.test._id
                ? 'bg-blue-100 border-l-4 border-blue-600'
                : 'hover:bg-gray-100'
            ]"
          >
            <span class="text-blue-600 text-sm">📋</span>
            <div class="flex-1 min-w-0">
              <div class="text-sm font-medium text-gray-900 truncate">
                {{ procedure.test.title }}
              </div>
            </div>
          </div>
        </div>
        <div v-else class="text-center py-8 text-gray-500">
          <p>Aucune procédure trouvée</p>
        </div>
      </div>

      <!-- Onglet Test Cases -->
      <div v-if="activeTab === 'testcases'">
        <div v-if="filteredTestCases.length > 0" class="space-y-1">
          <TreeNodeItem
            v-for="chapter in filteredTestCases"
            :key="chapter.id"
            :node="chapter"
            :selected-id="selectedTest?._id"
            :depth="0"
            @select="emitSelect"
            @toggle="emitToggle"
          />
        </div>
        <div v-else class="text-center py-8 text-gray-500">
          <p>Aucun test case trouvé</p>
        </div>
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
  procedureTree: AtpTreeNode[]
  testCaseTree: AtpTreeNode[]
  procedures: AtpTest[]
  testCases: AtpTest[]
  selectedTest: AtpTest | null
  searchQuery: string
}>()

const emit = defineEmits<{
  (e: 'select', test: AtpTest): void
  (e: 'toggle-procedure', id: string): void
  (e: 'toggle-testcase', id: string): void
  (e: 'update-search', query: string): void
}>()

const localSearchQuery = ref<string>(props.searchQuery)
const activeTab = ref<'procedures' | 'testcases'>('procedures')

const filteredProcedures = computed(() => {
  return atpTreeService.filterTree(props.procedureTree, localSearchQuery.value)
})

const filteredTestCases = computed(() => {
  return atpTreeService.filterTree(props.testCaseTree, localSearchQuery.value)
})

const countTestCases = (tree: AtpTreeNode[]): number => {
  let count = 0
  const countRecursive = (nodes: AtpTreeNode[]) => {
    nodes.forEach(node => {
      if (node.test.type === 'test-case') count++
      if (node.children.length > 0) countRecursive(node.children)
    })
  }
  countRecursive(tree)
  return count
}

const emitSelect = (test: AtpTest) => {
  emit('select', test)
}

const emitToggle = (id: string) => {
  if (activeTab.value === 'procedures') {
    emit('toggle-procedure', id)
  } else {
    emit('toggle-testcase', id)
  }
}

const emitSearchChange = () => {
  emit('update-search', localSearchQuery.value)
}
</script>
