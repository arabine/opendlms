<template>
  <div>
    <!-- Le nœud lui-même -->
    <div
      :style="{ paddingLeft: `${depth * 16}px` }"
      :class="[
        'flex items-center gap-2 px-3 py-2 rounded cursor-pointer transition-colors duration-150',
        selectedId === node.id
          ? getSelectedClasses(node.test.type)
          : 'hover:bg-gray-100'
      ]"
      @click="handleClick"
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
      />
    </div>
  </div>
</template>

<script setup lang="ts">
import type { AtpTreeNode, AtpTest, TestType } from '@/types'

const props = defineProps<{
  node: AtpTreeNode
  selectedId: string | undefined
  depth: number
}>()

const emit = defineEmits<{
  (e: 'select', test: AtpTest): void
  (e: 'toggle', id: string): void
}>()

const handleClick = () => {
  emit('select', props.node.test)
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
