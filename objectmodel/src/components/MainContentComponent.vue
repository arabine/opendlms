<template>
  <div class="grid grid-cols-1 lg:grid-cols-2 gap-6">
    <!-- Left Panel: Objects Tree -->
    <ObjectTreePanel
      :cosem-objects="cosemObjects"
      :search-query="searchQuery"
      :selected-object="selectedObject"
      :selected-attribute="selectedAttribute"
      @update-search="$emit('update-search', $event)"
      @select-attribute="$emit('select-attribute', $event)"
      @toggle-object="$emit('toggle-object', $event)"
    />

    <!-- Right Panel: XML Generation -->
    <XmlGenerationPanel
      :selected-object="selectedObject"
      :selected-attribute="selectedAttribute"
    />
  </div>
</template>

<script setup lang="ts">
import ObjectTreePanel from './ObjectTreePanel.vue'
import XmlGenerationPanel from './XmlGenerationPanel.vue'
import type { MainContentProps, SelectAttributeEvent } from '@/types'

defineProps<MainContentProps>()

defineEmits<{
  'update-search': [query: string]
  'select-attribute': [event: SelectAttributeEvent]
  'toggle-object': [objectId: number]
}>()
</script>