<template>
  <div class="relative">
    <!-- Indicateur de drop au-dessus -->
    <div
      v-if="dropPosition === 'before'"
      class="absolute left-0 right-0 h-0.5 bg-blue-500 z-10 -top-0.5"
      style="pointer-events: none;"
    >
      <div class="absolute left-0 w-2 h-2 bg-blue-500 rounded-full -translate-x-1 -translate-y-0.5"></div>
    </div>

    <!-- Le nœud lui-même -->
    <div
      draggable="true"
      :style="{ paddingLeft: `${depth * 16}px` }"
      :class="[
        'flex items-center gap-2 px-3 py-2 rounded cursor-pointer transition-colors duration-150 relative',
        selectedId === node.id
          ? getSelectedClasses(node.test.type)
          : 'hover:bg-gray-100',
        dropPosition === 'inside' ? 'bg-blue-50 border border-blue-300' : ''
      ]"
      @click="handleClick"
      @dragstart="handleDragStart"
      @dragover.prevent="handleDragOver"
      @dragleave="handleDragLeave"
      @drop.prevent="handleDrop"
      @dragend="handleDragEnd"
      @contextmenu.prevent="handleContextMenu"
    >
      <!-- Icône expand/collapse -->
      <button
        v-if="node.children.length > 0"
        @click.stop="$emit('toggle', node.id)"
        class="flex-shrink-0 w-5 h-5 flex items-center justify-center hover:bg-gray-200 rounded"
      >
        <svg
          class="w-4 h-4 text-gray-600 transition-transform duration-200"
          :class="{ 'rotate-90': node.expanded }"
          fill="none"
          stroke="currentColor"
          viewBox="0 0 24 24"
        >
          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 5l7 7-7 7" />
        </svg>
      </button>
      <div v-else class="w-5 flex-shrink-0"></div>

      <!-- Icône du type -->
      <span :class="getIconClasses(node.test.type)">
        {{ getIcon(node.test.type) }}
      </span>

      <!-- Contenu -->
      <div class="flex-1 min-w-0">
        <div class="text-sm font-medium text-gray-900 truncate">
          {{ node.test.number || node.test.testId || 'N/A' }}
        </div>
        <div class="text-xs text-gray-600 truncate">
          {{ node.test.title }}
        </div>
      </div>

      <!-- Badge du nombre d'enfants -->
      <span
        v-if="node.children.length > 0"
        class="flex-shrink-0 px-2 py-1 text-xs font-semibold rounded-full bg-gray-200 text-gray-700"
      >
        {{ node.children.length }}
      </span>
    </div>

    <!-- Indicateur de drop en-dessous -->
    <div
      v-if="dropPosition === 'after'"
      class="absolute left-0 right-0 h-0.5 bg-blue-500 z-10 -bottom-0.5"
      style="pointer-events: none;"
    >
      <div class="absolute left-0 w-2 h-2 bg-blue-500 rounded-full -translate-x-1 -translate-y-0.5"></div>
    </div>

    <!-- Enfants (récursif) -->
    <div v-if="node.expanded && node.children.length > 0" class="mt-1">
      <TreeNodeItem
        v-for="child in node.children"
        :key="child.id"
        :node="child"
        :selected-id="selectedId"
        :depth="depth + 1"
        @select="$emit('select', $event)"
        @toggle="$emit('toggle', $event)"
        @drag-start="$emit('drag-start', $event)"
        @drag-drop="$emit('drag-drop', $event)"
        @context-menu="$emit('context-menu', $event)"
      />
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref } from 'vue'
import type { AtpTreeNode, AtpTest, TestType } from '@/types'

const props = defineProps<{
  node: AtpTreeNode
  selectedId: string | undefined
  depth: number
}>()

const emit = defineEmits<{
  (e: 'select', test: AtpTest): void
  (e: 'toggle', id: string): void
  (e: 'drag-start', data: { node: AtpTreeNode }): void
  (e: 'drag-drop', data: { sourceId: string, targetId: string, position: 'before' | 'after' | 'inside' }): void
  (e: 'context-menu', data: { node: AtpTreeNode, event: MouseEvent }): void
}>()

const dropPosition = ref<'before' | 'after' | 'inside' | null>(null)
let dragLeaveTimeout: number | null = null

const handleClick = () => {
  emit('select', props.node.test)
}

const handleDragStart = (event: DragEvent) => {
  if (event.dataTransfer) {
    event.dataTransfer.effectAllowed = 'move'
    event.dataTransfer.setData('text/plain', props.node.id)
    emit('drag-start', { node: props.node })
  }
}

const handleDragOver = (event: DragEvent) => {
  event.preventDefault()
  event.stopPropagation()

  // Annuler le timeout de dragleave s'il existe
  if (dragLeaveTimeout) {
    clearTimeout(dragLeaveTimeout)
    dragLeaveTimeout = null
  }

  if (event.dataTransfer) {
    event.dataTransfer.dropEffect = 'move'
  }

  // Calculer la position relative dans l'élément
  const target = event.currentTarget as HTMLElement
  const rect = target.getBoundingClientRect()
  const y = event.clientY - rect.top
  const height = rect.height

  // Si le nœud a des enfants, on peut le mettre "inside"
  // Sinon, on propose uniquement before/after
  const canHaveChildren = props.node.test.type === 'chapter' || props.node.test.type === 'section'

  if (canHaveChildren && y > height * 0.25 && y < height * 0.75) {
    dropPosition.value = 'inside'
  } else if (y <= height * 0.5) {
    dropPosition.value = 'before'
  } else {
    dropPosition.value = 'after'
  }
}

const handleDragLeave = (event: DragEvent) => {
  // Utiliser un timeout pour éviter le clignotement lors du passage sur les enfants
  dragLeaveTimeout = window.setTimeout(() => {
    dropPosition.value = null
  }, 50)
}

const handleDrop = (event: DragEvent) => {
  event.preventDefault()
  event.stopPropagation()

  if (dragLeaveTimeout) {
    clearTimeout(dragLeaveTimeout)
    dragLeaveTimeout = null
  }

  const position = dropPosition.value || 'inside'
  dropPosition.value = null

  if (event.dataTransfer) {
    const sourceId = event.dataTransfer.getData('text/plain')
    if (sourceId && sourceId !== props.node.id) {
      emit('drag-drop', { sourceId, targetId: props.node.id, position })
    }
  }
}

const handleDragEnd = () => {
  if (dragLeaveTimeout) {
    clearTimeout(dragLeaveTimeout)
    dragLeaveTimeout = null
  }
  dropPosition.value = null
}

const handleContextMenu = (event: MouseEvent) => {
  emit('context-menu', { node: props.node, event })
}

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
  const classes = {
    'chapter': 'text-green-600',
    'section': 'text-yellow-600',
    'procedure': 'text-cyan-600',
    'test-case': 'text-red-600'
  }
  return `text-sm ${classes[type] || 'text-gray-600'}`
}

const getSelectedClasses = (type: TestType): string => {
  const classes = {
    'chapter': 'bg-green-100 border-l-4 border-green-600',
    'section': 'bg-yellow-100 border-l-4 border-yellow-600',
    'procedure': 'bg-cyan-100 border-l-4 border-cyan-600',
    'test-case': 'bg-red-100 border-l-4 border-red-600'
  }
  return classes[type] || 'bg-gray-100 border-l-4 border-gray-600'
}
</script>

<script lang="ts">
import { defineComponent } from 'vue'

export default defineComponent({
  name: 'TreeNodeItem'
})
</script>
