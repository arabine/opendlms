<template>
  <div class="bg-gray-50 min-h-screen">
    <div class="container mx-auto p-6">
      <div class="bg-white rounded-lg shadow-lg p-6">
        <HeaderComponent />
        
        <FileUploadComponent
          :loading="loading"
          @file-upload="handleFileUpload"
        />

        <LoadingComponent v-if="loading" />
        
        <ErrorComponent
          v-if="error"
          :error="error"
        />

        <MainContentComponent
          v-if="cosemObjects.length > 0"
          :cosem-objects="cosemObjects"
          :search-query="searchQuery"
          :selected-object="selectedObject"
          :selected-attribute="selectedAttribute"
          @update-search="searchQuery = $event"
          @select-attribute="selectAttribute"
          @toggle-object="toggleObject"
        />

        <EmptyStateComponent
          v-if="!loading && cosemObjects.length === 0"
        />
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
import { excelService } from './services/excelService'
import type { DlmsObject, DlmsAttribute, SelectAttributeEvent } from '@/types'

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