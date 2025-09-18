<template>
  <div class="space-y-4">
    <!-- Empty State -->
    <div v-if="!selectedAttribute" class="text-center py-12 bg-gray-50 rounded-lg border-2 border-dashed border-gray-300">
      <div class="text-gray-500">
        <svg class="mx-auto h-12 w-12 text-gray-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 12h6m-6 4h6m2 5H7a2 2 0 01-2-2V5a2 2 0 012-2h5.586a1 1 0 01.707.293l5.414 5.414a1 1 0 01.293.707V19a2 2 0 01-2 2z" />
        </svg>
        <p class="mt-2 text-lg font-medium">Sélectionnez un attribut</p>
        <p class="text-sm">Cliquez sur un attribut dans l'arbre pour générer les requêtes XML</p>
      </div>
    </div>

    <!-- XML Generation Interface -->
    <div v-else class="space-y-4">
      <!-- Selected Attribute Info -->
      <SelectedAttributeInfo
        :selected-object="selectedObject!"
        :selected-attribute="selectedAttribute"
      />

      <!-- Schema Selection -->
      <SchemaSelector
        :xml-schema="xmlSchema"
        @update-schema="xmlSchema = $event"
      />

      <!-- Request Generation Buttons -->
      <RequestButtons
        :can-set="canSet"
        @generate-get="generateGetRequest"
        @generate-set="generateSetRequest"
      />

      <!-- Generated XML Display -->
      <GeneratedXmlDisplay
        v-if="generatedXml"
        :generated-xml="generatedXml"
        :copy-success="copySuccess"
        @copy-to-clipboard="copyToClipboard"
      />
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, watch } from 'vue'
import SelectedAttributeInfo from './SelectedAttributeInfo.vue'
import SchemaSelector from './SchemaSelector.vue'
import RequestButtons from './RequestButtons.vue'
import GeneratedXmlDisplay from './GeneratedXmlDisplay.vue'
import { xmlService } from '@/services/xmlService'
import { clipboardService } from '@/services/clipboardService'
import type { XmlGenerationPanelProps, XmlSchema } from '@/types'

const props = defineProps<XmlGenerationPanelProps>()

const xmlSchema = ref<XmlSchema>('simple')
const generatedXml = ref<string>('')
const copySuccess = ref<boolean>(false)

const canSet = computed<boolean>(() => {
  return xmlService.canSet(props.selectedAttribute)
})

watch(() => props.selectedAttribute, () => {
  // Reset generated XML when attribute changes
  generatedXml.value = ''
  copySuccess.value = false
})

const generateGetRequest = (): void => {
  if (!props.selectedAttribute || !props.selectedObject) return

  if (xmlSchema.value === 'simple') {
    generatedXml.value = xmlService.generateSimpleGetXml(props.selectedObject, props.selectedAttribute)
  } else {
    generatedXml.value = xmlService.generateStandardGetXml(props.selectedObject, props.selectedAttribute)
  }
}

const generateSetRequest = (): void => {
  if (!props.selectedAttribute || !props.selectedObject || !canSet.value) return

  if (xmlSchema.value === 'simple') {
    generatedXml.value = xmlService.generateSimpleSetXml(props.selectedObject, props.selectedAttribute)
  } else {
    generatedXml.value = xmlService.generateStandardSetXml(props.selectedObject, props.selectedAttribute)
  }
}

const copyToClipboard = async (): Promise<void> => {
  try {
    const success = await clipboardService.copyToClipboard(generatedXml.value)
    if (success) {
      copySuccess.value = true
      setTimeout(() => {
        copySuccess.value = false
      }, 2000)
    }
  } catch (err) {
    console.error('Failed to copy to clipboard:', err)
  }
}
</script>