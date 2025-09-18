<template>
  <div
    @click="$emit('select-attribute')"
    class="attribute-row tree-connector py-2 text-sm attribute-selectable"
    :class="{ 'attribute-selected': isSelected }"
  >
    <div class="flex items-start space-x-3">
      <span class="font-mono text-gray-500 text-xs w-8 flex-shrink-0">{{ attribute.number }}</span>
      <div class="flex-1 min-w-0">
        <div class="font-medium text-gray-900" v-html="highlightText(attribute.name)"></div>
        <div class="text-xs text-gray-600 mt-1">
          <span v-html="highlightText(attribute.type)"></span>
          <span v-if="attribute.defaultValue" class="ml-2 text-blue-600" v-html="highlightText(`Défaut: ${attribute.defaultValue}`)"></span>
        </div>
        <div v-if="attribute.accessRights" class="text-xs text-green-600 mt-1" v-html="highlightText(attribute.accessRights)"></div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import type { AttributeItemProps } from '@/types'

const props = defineProps<AttributeItemProps>()

defineEmits<{
  'select-attribute': []
}>()

const highlightText = (text: string): string => {
  if (!props.searchQuery.trim() || !text) return text
  
  const query = props.searchQuery.trim()
  const regex = new RegExp(`(${query.replace(/[.*+?^${}()|[\]\\]/g, '\\$&')})`, 'gi')
  return text.replace(regex, '<span class="search-highlight">$1</span>')
}
</script>