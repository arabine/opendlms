<template>
  <div class="h-full flex flex-col">
    <!-- Barre de recherche -->
    <div class="bg-white p-4 border-b border-gray-200">
      <input
        v-model="localSearchQuery"
        type="text"
        placeholder="Rechercher... (Clic droit pour ajouter)"
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
    <div class="flex-1 overflow-y-auto p-4" tabindex="0" @keydown="handleKeyDown">
      <!-- Onglet Procédures -->
      <div v-if="activeTab === 'procedures'">
        <div v-if="filteredProcedures.length > 0" class="space-y-1 pb-8">
          <div
            v-for="(procedure, index) in filteredProcedures"
            :key="procedure.id"
            class="relative"
          >
            <!-- Indicateur de drop au-dessus -->
            <div
              v-if="procedureDropState[procedure.id] === 'before'"
              class="absolute left-0 right-0 h-0.5 bg-blue-500 z-10 -top-0.5"
              style="pointer-events: none;"
            >
              <div class="absolute left-0 w-2 h-2 bg-blue-500 rounded-full -translate-x-1 -translate-y-0.5"></div>
            </div>

            <div
              @click="emitSelect(procedure.test)"
              @contextmenu.prevent="handleContextMenu($event, procedure)"
              draggable="true"
              @dragstart="handleDragStart($event, procedure)"
              @dragover.prevent="handleProcedureDragOver($event, procedure)"
              @dragleave="handleProcedureDragLeave(procedure)"
              @drop.prevent="handleProcedureDrop($event, procedure)"
              :class="[
                'px-3 py-2 rounded cursor-pointer transition-colors duration-150',
                'flex items-center gap-2 relative',
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

              <!-- Badge de validation pour les procédures -->
              <div
                :class="[
                  'flex-shrink-0 w-6 h-6 rounded-full flex items-center justify-center',
                  procedure.test.validated ? 'bg-green-500' : 'bg-gray-300'
                ]"
                :title="procedure.test.validated ? 'Validé' : 'Non validé'"
              >
                <svg
                  v-if="procedure.test.validated"
                  class="w-4 h-4 text-white"
                  fill="none"
                  stroke="currentColor"
                  viewBox="0 0 24 24"
                >
                  <path stroke-linecap="round" stroke-linejoin="round" stroke-width="3" d="M5 13l4 4L19 7" />
                </svg>
                <svg
                  v-else
                  class="w-3 h-3 text-gray-500"
                  fill="none"
                  stroke="currentColor"
                  viewBox="0 0 24 24"
                >
                  <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M6 18L18 6M6 6l12 12" />
                </svg>
              </div>
            </div>

            <!-- Indicateur de drop en-dessous -->
            <div
              v-if="procedureDropState[procedure.id] === 'after'"
              class="absolute left-0 right-0 h-0.5 bg-blue-500 z-10 -bottom-0.5"
              style="pointer-events: none;"
            >
              <div class="absolute left-0 w-2 h-2 bg-blue-500 rounded-full -translate-x-1 -translate-y-0.5"></div>
            </div>
          </div>
        </div>
        <div v-else class="text-center py-8 text-gray-500">
          <p>Aucune procédure trouvée</p>
        </div>
      </div>

      <!-- Onglet Test Cases -->
      <div v-if="activeTab === 'testcases'">
        <div v-if="filteredTestCases.length > 0" class="space-y-1 pb-8">
          <TreeNodeItem
            v-for="chapter in filteredTestCases"
            :key="chapter.id"
            :node="chapter"
            :selected-id="selectedTest?._id"
            :depth="0"
            @select="emitSelect"
            @toggle="emitToggle"
            @drag-drop="handleDragDrop"
            @context-menu="handleTreeContextMenu"
          />
        </div>
        <div v-else class="text-center py-8 text-gray-500">
          <p>Aucun test case trouvé</p>
        </div>
      </div>
    </div>

    <!-- Menu contextuel -->
    <div
      v-if="contextMenu.show"
      :style="{
        position: 'fixed',
        top: contextMenu.y + 'px',
        left: contextMenu.x + 'px',
        zIndex: 1000
      }"
      class="bg-white shadow-lg rounded-md border border-gray-200 py-1 min-w-[180px]"
      @click.stop
    >
      <!-- Options de création selon l'onglet -->
      <div v-if="activeTab === 'testcases'">
        <button
          @click="handleAddNew('chapter')"
          class="w-full px-4 py-2 text-left text-sm hover:bg-gray-100 flex items-center gap-2"
        >
          <span>📚</span> Nouveau chapitre
        </button>
        <button
          @click="handleAddNew('section')"
          class="w-full px-4 py-2 text-left text-sm hover:bg-gray-100 flex items-center gap-2"
        >
          <span>📄</span> Nouvelle section
        </button>
        <button
          @click="handleAddNew('test-case')"
          class="w-full px-4 py-2 text-left text-sm hover:bg-gray-100 flex items-center gap-2"
        >
          <span>🧪</span> Nouveau test case
        </button>
        <hr class="my-1 border-gray-200" />
      </div>
      <div v-else-if="activeTab === 'procedures'">
        <button
          @click="handleAddNew('procedure')"
          class="w-full px-4 py-2 text-left text-sm hover:bg-gray-100 flex items-center gap-2"
        >
          <span>📋</span> Nouvelle procédure
        </button>
        <hr class="my-1 border-gray-200" />
      </div>

      <button
        @click="handleCopy"
        class="w-full px-4 py-2 text-left text-sm hover:bg-gray-100 flex items-center gap-2"
      >
        <span>📋</span> Copier
      </button>
      <button
        v-if="copiedNode"
        @click="handlePaste"
        class="w-full px-4 py-2 text-left text-sm hover:bg-gray-100 flex items-center gap-2"
      >
        <span>📄</span> Coller
      </button>
      <button
        @click="handleDuplicate"
        class="w-full px-4 py-2 text-left text-sm hover:bg-gray-100 flex items-center gap-2"
      >
        <span>🔄</span> Dupliquer
      </button>
      <hr class="my-1 border-gray-200" />
      <button
        @click="handleDelete"
        class="w-full px-4 py-2 text-left text-sm hover:bg-red-100 text-red-600 flex items-center gap-2"
      >
        <span>🗑️</span> Supprimer
      </button>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted, onUnmounted } from 'vue'
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
  (e: 'move-node', data: { sourceId: string, targetId: string, position: 'before' | 'after' | 'inside', tab: 'procedures' | 'testcases' }): void
  (e: 'copy-node', node: AtpTreeNode): void
  (e: 'paste-node', data: { copiedNode: AtpTreeNode, targetNode: AtpTreeNode }): void
  (e: 'duplicate-node', node: AtpTreeNode): void
  (e: 'delete-node', id: string): void
  (e: 'add-test', type: 'chapter' | 'section' | 'test-case' | 'procedure'): void
}>()

const localSearchQuery = ref<string>(props.searchQuery)
const activeTab = ref<'procedures' | 'testcases'>('procedures')
const copiedNode = ref<AtpTreeNode | null>(null)
const contextMenu = ref({
  show: false,
  x: 0,
  y: 0,
  node: null as AtpTreeNode | null
})
const draggedNode = ref<AtpTreeNode | null>(null)
const procedureDropState = ref<Record<string, 'before' | 'after' | null>>({})

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

// Gestion du drag & drop
const handleDragStart = (event: DragEvent, node: AtpTreeNode) => {
  if (event.dataTransfer) {
    draggedNode.value = node
    event.dataTransfer.effectAllowed = 'move'
    event.dataTransfer.setData('text/plain', node.id)
  }
}

const handleProcedureDragOver = (event: DragEvent, targetNode: AtpTreeNode) => {
  event.preventDefault()
  event.stopPropagation()

  if (event.dataTransfer) {
    event.dataTransfer.dropEffect = 'move'
  }

  // Calculer la position relative dans l'élément
  const target = event.currentTarget as HTMLElement
  const rect = target.getBoundingClientRect()
  const y = event.clientY - rect.top
  const height = rect.height

  // Pour les procédures, on propose uniquement before/after (pas inside)
  if (y <= height * 0.5) {
    procedureDropState.value[targetNode.id] = 'before'
  } else {
    procedureDropState.value[targetNode.id] = 'after'
  }
}

const handleProcedureDragLeave = (targetNode: AtpTreeNode) => {
  procedureDropState.value[targetNode.id] = null
}

const handleProcedureDrop = (event: DragEvent, targetNode: AtpTreeNode) => {
  event.preventDefault()
  event.stopPropagation()

  const position = procedureDropState.value[targetNode.id] || 'after'
  procedureDropState.value[targetNode.id] = null

  if (draggedNode.value && draggedNode.value.id !== targetNode.id) {
    emit('move-node', {
      sourceId: draggedNode.value.id,
      targetId: targetNode.id,
      position,
      tab: activeTab.value
    })
  }
  draggedNode.value = null
}

const handleDrop = (event: DragEvent, targetNode: AtpTreeNode) => {
  event.preventDefault()
  if (draggedNode.value && draggedNode.value.id !== targetNode.id) {
    emit('move-node', {
      sourceId: draggedNode.value.id,
      targetId: targetNode.id,
      position: 'inside',
      tab: activeTab.value
    })
  }
  draggedNode.value = null
}

const handleDragDrop = (data: { sourceId: string, targetId: string, position: 'before' | 'after' | 'inside' }) => {
  emit('move-node', {
    sourceId: data.sourceId,
    targetId: data.targetId,
    position: data.position,
    tab: activeTab.value
  })
}

// Gestion du menu contextuel
const handleContextMenu = (event: MouseEvent, node: AtpTreeNode) => {
  event.preventDefault()
  contextMenu.value = {
    show: true,
    x: event.clientX,
    y: event.clientY,
    node
  }
}

const handleTreeContextMenu = (data: { node: AtpTreeNode, event: MouseEvent }) => {
  handleContextMenu(data.event, data.node)
}

const closeContextMenu = () => {
  contextMenu.value.show = false
}

// Actions du menu contextuel
const handleAddNew = (type: 'chapter' | 'section' | 'test-case' | 'procedure') => {
  emit('add-test', type)
  closeContextMenu()
}

const handleCopy = () => {
  if (contextMenu.value.node) {
    copiedNode.value = contextMenu.value.node
  }
  closeContextMenu()
}

const handlePaste = () => {
  if (copiedNode.value && contextMenu.value.node) {
    emit('paste-node', {
      copiedNode: copiedNode.value,
      targetNode: contextMenu.value.node
    })
  }
  closeContextMenu()
}

const handleDuplicate = () => {
  if (contextMenu.value.node) {
    emit('duplicate-node', contextMenu.value.node)
  }
  closeContextMenu()
}

const handleDelete = () => {
  if (contextMenu.value.node) {
    emit('delete-node', contextMenu.value.node.id)
  }
  closeContextMenu()
}

// Gestion des raccourcis clavier
const handleKeyDown = (event: KeyboardEvent) => {
  // Copier (Ctrl+C)
  if (event.ctrlKey && event.key === 'c' && props.selectedTest) {
    const node = activeTab.value === 'procedures'
      ? atpTreeService.findNode(props.procedureTree, props.selectedTest._id)
      : atpTreeService.findNode(props.testCaseTree, props.selectedTest._id)

    if (node) {
      copiedNode.value = node
    }
    event.preventDefault()
  }

  // Coller (Ctrl+V)
  if (event.ctrlKey && event.key === 'v' && copiedNode.value && props.selectedTest) {
    const targetNode = activeTab.value === 'procedures'
      ? atpTreeService.findNode(props.procedureTree, props.selectedTest._id)
      : atpTreeService.findNode(props.testCaseTree, props.selectedTest._id)

    if (targetNode) {
      emit('paste-node', targetNode)
    }
    event.preventDefault()
  }

  // Supprimer (Delete)
  if (event.key === 'Delete' && props.selectedTest) {
    emit('delete-node', props.selectedTest._id)
    event.preventDefault()
  }
}

// Fermer le menu contextuel quand on clique ailleurs
const handleClickOutside = (event: MouseEvent) => {
  if (contextMenu.value.show) {
    closeContextMenu()
  }
}

onMounted(() => {
  document.addEventListener('click', handleClickOutside)
})

onUnmounted(() => {
  document.removeEventListener('click', handleClickOutside)
})
</script>
