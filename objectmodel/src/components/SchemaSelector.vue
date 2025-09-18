<template>
  <div class="bg-white p-4 rounded-lg border">
    <h4 class="font-semibold text-gray-800 mb-3">Schéma XML</h4>
    <div class="space-y-2">
      <label class="flex items-center">
        <input 
          type="radio" 
          :value="'simple'" 
          :checked="xmlSchema === 'simple'"
          @change="handleSchemaChange('simple')"
          class="mr-2"
        >
        <span>Schéma simple</span>
      </label>
      <label class="flex items-center">
        <input 
          type="radio" 
          :value="'standard'" 
          :checked="xmlSchema === 'standard'"
          @change="handleSchemaChange('standard')"
          class="mr-2"
        >
        <span>Schéma standard</span>
      </label>
    </div>
  </div>
</template>

<script setup lang="ts">
import type { SchemaSelectorProps, XmlSchema } from '@/types'

defineProps<SchemaSelectorProps>()

const emit = defineEmits<{
  'update-schema': [schema: XmlSchema]
}>()

const handleSchemaChange = (schema: XmlSchema): void => {
  emit('update-schema', schema)
}
</script>