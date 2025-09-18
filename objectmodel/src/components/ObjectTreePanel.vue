<template>
  <div class="space-y-4">
    <!-- Search and Filter Section -->
    <SearchFilterComponent
      :search-query="searchQuery"
      :filtered-objects-count="filteredObjects.length"
      :total-objects-count="cosemObjects.length"
      :all-expanded="allExpanded"
      @update-search="$emit('update-search', $event)"
      @toggle-all="toggleAllObjects"
    />

    <!-- Objects Tree -->
    <div class="space-y-2">
      <h2 class="text-xl font-semibold text-gray-800 mb-4">
        Arbre des objets COSEM
      </h2>

      <div class="space-y-1 max-h-96 overflow-y-auto">
        <ObjectItemComponent
          v-for="object in filteredObjects"
          :key="object.id"
          :object="object"
          :search-query="searchQuery"
          :selected-object="selectedObject"
          :selected-attribute="selectedAttribute"
          @select-attribute="$emit('select-attribute', $event)"
          @toggle-object="$emit('toggle-object', $event)"
        />
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue'
import SearchFilterComponent from './SearchFilterComponent.vue'
import ObjectItemComponent from './ObjectItemComponent.vue'
import type { ObjectTreePanelProps, FilteredObject, SelectAttributeEvent } from '@/types'

const props = defineProps<ObjectTreePanelProps>()

defineEmits<{
  'update-search': [query: string]
  'select-attribute': [event: SelectAttributeEvent]
  'toggle-object': [objectId: number]
}>()

const filteredObjects = computed<FilteredObject[]>(() => {
  if (!props.searchQuery.trim()) {
    return props.cosemObjects.map(obj => ({
      ...obj,
      filteredAttributes: obj.attributes
    }))
  }

  const query = props.searchQuery.toLowerCase().trim()
  return props.cosemObjects
    .map(obj => {
      const objectMatches = 
        obj.name.toLowerCase().includes(query) ||
        obj.obisCode.toLowerCase().includes(query) ||
        obj.classId.toString().includes(query) ||
        obj.version.toString().includes(query)

      const filteredAttributes = obj.attributes.filter(attr => 
        attr.name.toLowerCase().includes(query) ||
        attr.type.toLowerCase().includes(query) ||
        (attr.defaultValue && attr.defaultValue.toString().toLowerCase().includes(query)) ||
        (attr.accessRights && attr.accessRights.toString().toLowerCase().includes(query))
      )

      if (objectMatches || filteredAttributes.length > 0) {
        return {
          ...obj,
          filteredAttributes: objectMatches ? obj.attributes : filteredAttributes
        }
      }
      return null
    })
    .filter((obj): obj is FilteredObject => obj !== null)
})

const allExpanded = computed<boolean>(() => {
  return filteredObjects.value.length > 0 && filteredObjects.value.every(obj => obj.expanded)
})

const toggleAllObjects = (): void => {
  const newState = !allExpanded.value
  filteredObjects.value.forEach(obj => {
    obj.expanded = newState
  })
}
</script>