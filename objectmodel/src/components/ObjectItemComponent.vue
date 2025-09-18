<template>
  <div class="border border-gray-200 rounded-lg overflow-hidden">
    <!-- Object Header -->
    <div
      @click="$emit('toggle-object', object.id)"
      class="bg-blue-50 p-3 cursor-pointer hover:bg-blue-100 transition-colors flex items-center justify-between compact-node"
    >
      <div>
        <h3 class="text-base font-semibold text-blue-800" v-html="highlightText(object.name)"></h3>
        <div class="text-sm text-blue-600">
          <span v-html="highlightText(`class_id = ${object.classId}, version = ${object.version}`)"></span>
          <span class="ml-2" v-html="highlightText(`(${object.obisCode})`)"></span>
        </div>
      </div>
      <div class="text-blue-600">
        <svg
          :class="object.expanded ? 'rotate-90' : ''"
          class="w-4 h-4 transform transition-transform"
          fill="none"
          stroke="currentColor"
          viewBox="0 0 24 24"
        >
          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 5l7 7-7 7"></path>
        </svg>
      </div>
    </div>

    <!-- Attributes -->
    <div v-if="object.expanded" class="bg-white">
      <div v-if="object.filteredAttributes.length === 0" class="p-3 text-gray-500 italic text-sm">
        Aucun attribut trouvé pour cet objet
      </div>
      <div v-else class="tree-line">
        <AttributeItemComponent
          v-for="(attr, index) in object.filteredAttributes"
          :key="index"
          :attribute="attr"
          :object="object"
          :search-query="searchQuery"
          :is-selected="isAttributeSelected(object, attr)"
          @select-attribute="$emit('select-attribute', { object, attribute: attr })"
        />
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import AttributeItemComponent from './AttributeItemComponent.vue'
import type { ObjectItemProps, SelectAttributeEvent, DlmsAttribute } from '@/types'

const props = defineProps<ObjectItemProps>()

defineEmits<{
  'select-attribute': [event: SelectAttributeEvent]
  'toggle-object': [objectId: number]
}>()

const highlightText = (text: string): string => {
  if (!props.searchQuery.trim() || !text) return text
  
  const query = props.searchQuery.trim()
  const regex = new RegExp(`(${query.replace(/[.*+?^${}()|[\]\\]/g, '\\$&')})`, 'gi')
  return text.replace(regex, '<span class="search-highlight">$1</span>')
}

const isAttributeSelected = (object: typeof props.object, attribute: DlmsAttribute): boolean => {
  return !!(props.selectedObject &&
           props.selectedAttribute &&
           props.selectedObject.id === object.id &&
           props.selectedAttribute.number === attribute.number)
}
</script>