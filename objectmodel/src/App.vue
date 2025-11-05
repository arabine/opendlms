<template>
  <div class="bg-gray-50 h-screen flex flex-col">
    <div class="flex-shrink-0 bg-white shadow-lg">
      <div class="container mx-auto px-6 py-4">
        <HeaderComponent />

        <!-- Onglets de navigation -->
        <div class="flex border-b border-gray-200">
          <button
            @click="activeTab = 'cosem'"
            :class="[
              'px-6 py-3 font-semibold transition-colors duration-200',
              activeTab === 'cosem'
                ? 'border-b-2 border-blue-600 text-blue-600'
                : 'text-gray-600 hover:text-gray-800'
            ]"
          >
            📊 COSEM Object Model
          </button>
          <button
            @click="activeTab = 'atp'"
            :class="[
              'px-6 py-3 font-semibold transition-colors duration-200',
              activeTab === 'atp'
                ? 'border-b-2 border-blue-600 text-blue-600'
                : 'text-gray-600 hover:text-gray-800'
            ]"
          >
            📋 Tests ATP
          </button>
          <button
            @click="activeTab = 'datetime'"
            :class="[
              'px-6 py-3 font-semibold transition-colors duration-200',
              activeTab === 'datetime'
                ? 'border-b-2 border-blue-600 text-blue-600'
                : 'text-gray-600 hover:text-gray-800'
            ]"
          >
            🕐 Date Time Utilities
          </button>
        </div>
      </div>
    </div>

    <div class="flex-1 overflow-hidden">
      <!-- Contenu COSEM -->
      <div v-show="activeTab === 'cosem'" class="h-full flex flex-col">
        <div class="flex-shrink-0 container mx-auto px-6 py-4">
          <FileUploadComponent
            :loading="loading"
            @file-upload="handleFileUpload"
          />

          <LoadingComponent v-if="loading" />

          <ErrorComponent
            v-if="error"
            :error="error"
          />

          <EmptyStateComponent
            v-if="!loading && cosemObjects.length === 0"
          />
        </div>

        <div v-if="cosemObjects.length > 0" class="flex-1 overflow-hidden container mx-auto px-6">
          <MainContentComponent
            :cosem-objects="cosemObjects"
            :search-query="searchQuery"
            :selected-object="selectedObject"
            :selected-attribute="selectedAttribute"
            @update-search="searchQuery = $event"
            @select-attribute="selectAttribute"
            @toggle-object="toggleObject"
          />
        </div>
      </div>

      <!-- Contenu ATP -->
      <div v-show="activeTab === 'atp'" class="h-full">
        <AtpManager />
      </div>

      <!-- Contenu Date Time Utilities -->
      <div v-show="activeTab === 'datetime'" class="h-full">
        <DateTimeUtilities />
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref } from 'vue'
import HeaderComponent from './components/HeaderComponent.vue'
import FileUploadComponent from './components/FileUploadComponent.vue'
import LoadingComponent from './components/LoadingComponent.vue'
import ErrorComponent from './components/ErrorComponent.vue'
import MainContentComponent from './components/MainContentComponent.vue'
import EmptyStateComponent from './components/EmptyStateComponent.vue'
import AtpManager from './components/AtpManager.vue'
import DateTimeUtilities from './components/DateTimeUtilities.vue'
import { excelService } from './services/excelService'
import type { DlmsObject, DlmsAttribute, SelectAttributeEvent } from '@/types'

const activeTab = ref<'cosem' | 'atp' | 'datetime'>('cosem')
const loading = ref<boolean>(false)
const error = ref<string | null>(null)
const cosemObjects = ref<DlmsObject[]>([])
const searchQuery = ref<string>('')
const selectedObject = ref<DlmsObject | null>(null)
const selectedAttribute = ref<DlmsAttribute | null>(null)

const selectAttribute = (event: SelectAttributeEvent): void => {
  selectedObject.value = event.object
  selectedAttribute.value = event.attribute
}

const handleFileUpload = async (file: File | null): Promise<void> => {
  if (!file) return

  loading.value = true
  error.value = null
  cosemObjects.value = []
  selectedObject.value = null
  selectedAttribute.value = null

  try {
    const data = await excelService.readExcelFile(file)
    cosemObjects.value = excelService.parseObjectModel(data)
  } catch (err) {
    error.value = `Erreur lors du chargement du fichier: ${err instanceof Error ? err.message : String(err)}`
    console.error('Error loading file:', err)
  } finally {
    loading.value = false
  }
}

const toggleObject = (objectId: number): void => {
  const object = cosemObjects.value.find(obj => obj.id === objectId)
  if (object) {
    object.expanded = !object.expanded
  }
}
</script>