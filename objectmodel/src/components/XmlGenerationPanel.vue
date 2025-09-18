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

      <!-- Generation Method Status -->
      <div class="bg-blue-50 border border-blue-200 rounded-lg p-4">
        <div class="flex items-center space-x-2">
          <svg v-if="factorySupported" class="h-5 w-5 text-green-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M5 13l4 4L19 7"></path>
          </svg>
          <svg v-else class="h-5 w-5 text-yellow-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 9v2m0 4h.01m-6.938 4h13.856c1.54 0 2.502-1.667 1.732-2.5L13.732 4c-.77-.833-1.732-.833-2.5 0L4.732 13.5c-.77.833.192 2.5 1.732 2.5z"></path>
          </svg>
          <span class="font-medium text-sm">
            <span v-if="factorySupported" class="text-green-800">
              CosemClassFactory disponible - Classe {{ selectedObject?.classId }}
            </span>
            <span v-else class="text-yellow-800">
              Service XML classique - Classe {{ selectedObject?.classId }} non supportée
            </span>
          </span>
        </div>
        <div v-if="classInfo" class="mt-2 text-xs text-blue-700">
          {{ classInfo.name }} - {{ classInfo.description }}
        </div>
      </div>

      <!-- Schema Selection (only when factory not available) -->
      <div v-if="!factorySupported">
        <SchemaSelector
          :xml-schema="xmlSchema"
          @update-schema="xmlSchema = $event"
        />
      </div>

      <!-- Value Input for SetRequest (when factory is available) -->
      <div v-if="factorySupported && canSet" class="bg-white border border-gray-200 rounded-lg p-4">
        <h4 class="font-medium text-gray-800 mb-3">Valeur pour SetRequest</h4>
        <div class="space-y-3">
          <!-- Value input field -->
          <div>
            <label class="block text-sm font-medium text-gray-700 mb-1">
              Valeur ({{ selectedAttribute?.type || 'unknown' }})
            </label>
            <textarea v-if="isComplexType"
              v-model="inputValue"
              rows="4"
              class="font-mono text-sm w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
              :placeholder="valuePlaceholder"
            />
            <input v-else
              v-model="inputValue"
              type="text"
              class="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
              :placeholder="valuePlaceholder"
            />
          </div>

          <!-- Example values -->
          <div v-if="exampleValues.length > 0">
            <label class="block text-sm font-medium text-gray-700 mb-2">Valeurs d'exemple:</label>
            <div class="flex flex-wrap gap-2">
              <button
                v-for="(example, index) in exampleValues"
                :key="index"
                @click="applyExample(example)"
                class="px-2 py-1 text-xs bg-blue-100 text-blue-800 rounded hover:bg-blue-200 transition-colors"
              >
                {{ example.label }}
              </button>
            </div>
          </div>
        </div>
      </div>

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

      <!-- Error Display -->
      <div v-if="generationError" class="bg-red-50 border border-red-200 rounded-lg p-4">
        <div class="flex items-center space-x-2">
          <svg class="h-5 w-5 text-red-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 8v4m0 4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z"></path>
          </svg>
          <span class="font-medium text-red-800">Erreur de génération</span>
        </div>
        <p class="mt-2 text-sm text-red-700">{{ generationError }}</p>
      </div>

      <!-- Debug Info -->
      <div v-if="showDebugInfo" class="bg-gray-50 border border-gray-200 rounded-lg p-4 text-xs">
        <h4 class="font-medium mb-2">Informations de debug</h4>
        <div class="space-y-1 text-gray-600">
          <div>Méthode: {{ factorySupported ? 'CosemClassFactory' : 'XML Service' }}</div>
          <div>Classe {{ selectedObject?.classId }}: {{ factorySupported ? 'Supportée' : 'Non supportée' }}</div>
          <div>Type complexe: {{ isComplexType }}</div>
          <div>Classes supportées: {{ supportedClasses.join(', ') }}</div>
          <div>Service status: {{ serviceStatus.initialized ? 'OK' : 'Erreur' }}</div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, watch, onMounted } from 'vue'
import SelectedAttributeInfo from './SelectedAttributeInfo.vue'
import SchemaSelector from './SchemaSelector.vue'
import RequestButtons from './RequestButtons.vue'
import GeneratedXmlDisplay from './GeneratedXmlDisplay.vue'
import { xmlService } from '@/services/xmlService'
import { clipboardService } from '@/services/clipboardService'

// Import du service DLMS simplifié
import { dlmsService } from '@/services/dlmsService'

import type { XmlGenerationPanelProps, XmlSchema } from '@/types'

const props = defineProps<XmlGenerationPanelProps>()

// États réactifs
const xmlSchema = ref<XmlSchema>('simple')
const generatedXml = ref<string>('')
const copySuccess = ref<boolean>(false)
const generationError = ref<string>('')
const inputValue = ref<string>('')
const showDebugInfo = ref<boolean>(false)

// États calculés
const factorySupported = computed<boolean>(() => {
  return props.selectedObject ? dlmsService.isClassSupported(props.selectedObject.classId) : false
})

const classInfo = computed(() => {
  return props.selectedObject ? dlmsService.getClassInfo(props.selectedObject.classId) : null
})

const canSet = computed<boolean>(() => {
  return xmlService.canSet(props.selectedAttribute)
})

const isComplexType = computed<boolean>(() => {
  return props.selectedAttribute ? dlmsService.isComplexType(props.selectedAttribute) : false
})

const valuePlaceholder = computed<string>(() => {
  return props.selectedAttribute ? dlmsService.getValuePlaceholder(props.selectedAttribute) : 'Entrez une valeur...'
})

const exampleValues = computed(() => {
  if (!props.selectedObject || !props.selectedAttribute || !factorySupported.value) return []
  return dlmsService.getExampleValues(props.selectedObject, props.selectedAttribute)
})

const supportedClasses = computed(() => {
  return dlmsService.getSupportedClasses()
})

const serviceStatus = computed(() => {
  return dlmsService.getStatus()
})

// Watchers
watch(() => props.selectedAttribute, () => {
  // Reset lors du changement d'attribut
  generatedXml.value = ''
  copySuccess.value = false
  generationError.value = ''
  inputValue.value = ''
})

// Méthodes
const applyExample = (example: any) => {
  if (typeof example.value === 'object') {
    inputValue.value = JSON.stringify(example.value, null, 2)
  } else {
    inputValue.value = String(example.value)
  }
}

const parseInputValue = (): any => {
  if (!inputValue.value.trim()) {
    throw new Error('Veuillez entrer une valeur')
  }
  
  if (!props.selectedAttribute) {
    return inputValue.value
  }
  
  const type = props.selectedAttribute.type.toLowerCase()
  
  try {
    if (type.includes('boolean')) {
      return inputValue.value.toLowerCase() === 'true'
    } else if (type.includes('unsigned') || type.includes('integer')) {
      const parsed = parseInt(inputValue.value, 10)
      if (isNaN(parsed)) {
        throw new Error('Valeur numérique invalide')
      }
      return parsed
    } else if (type.includes('structure') || type.includes('array')) {
      return JSON.parse(inputValue.value)
    } else {
      return inputValue.value
    }
  } catch (error) {
    if (error instanceof SyntaxError) {
      throw new Error('Format JSON invalide')
    }
    throw error
  }
}

const generateGetRequest = (): void => {
  if (!props.selectedAttribute || !props.selectedObject) return

  generationError.value = ''
  
  try {
    if (factorySupported.value) {
      // Utiliser le service DLMS avec factory
      const result = dlmsService.generateGetRequest(props.selectedObject, props.selectedAttribute)
      
      if (result.success) {
        generatedXml.value = result.xml!
      } else {
        throw new Error(result.error || 'Erreur inconnue')
      }
    } else {
      // Utiliser le service XML classique
      if (xmlSchema.value === 'simple') {
        generatedXml.value = xmlService.generateSimpleGetXml(props.selectedObject, props.selectedAttribute)
      } else {
        generatedXml.value = xmlService.generateStandardGetXml(props.selectedObject, props.selectedAttribute)
      }
    }
  } catch (error) {
    generationError.value = `Erreur génération GET: ${error}`
    console.error('Erreur génération GET:', error)
  }
}

const generateSetRequest = (): void => {
  if (!props.selectedAttribute || !props.selectedObject || !canSet.value) return

  generationError.value = ''
  
  try {
    if (factorySupported.value) {
      // Utiliser le service DLMS avec factory
      const value = parseInputValue()
      const result = dlmsService.generateSetRequest(props.selectedObject, props.selectedAttribute, value)
      
      if (result.success) {
        generatedXml.value = result.xml!
      } else {
        throw new Error(result.error || 'Erreur inconnue')
      }
    } else {
      // Utiliser le service XML classique
      if (xmlSchema.value === 'simple') {
        generatedXml.value = xmlService.generateSimpleSetXml(props.selectedObject, props.selectedAttribute)
      } else {
        generatedXml.value = xmlService.generateStandardSetXml(props.selectedObject, props.selectedAttribute)
      }
    }
  } catch (error) {
    generationError.value = `Erreur génération SET: ${error}`
    console.error('Erreur génération SET:', error)
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

// Initialisation
onMounted(() => {
  // Activer les infos de debug en mode développement
  showDebugInfo.value = process.env.NODE_ENV === 'development'
  
  // Afficher les capacités en mode développement
  if (showDebugInfo.value) {
    console.log('[XmlGenerationPanel] Initialisation avec DlmsService')
    dlmsService.demonstrateCapabilities()
  }
})
</script>

<style scoped>
.copy-success {
  @apply bg-green-600;
}
</style>