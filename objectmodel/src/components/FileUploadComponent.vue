<template>
  <div class="mb-8 p-4 border-2 border-dashed border-gray-300 rounded-lg hover:border-blue-500 transition-colors">
    <div class="text-center">
      <input
        ref="fileInput"
        @change="handleFileUpload"
        accept=".xlsx,.xls"
        class="hidden"
        id="file-upload"
        type="file"
      >
      <label
        for="file-upload"
        class="inline-block bg-blue-600 text-white px-6 py-3 rounded-lg cursor-pointer hover:bg-blue-700 transition-colors"
        :class="{ 'opacity-50 cursor-not-allowed': loading }"
      >
        {{ loading ? 'Chargement...' : 'Charger un fichier XLSX' }}
      </label>
      <p class="text-gray-600 mt-2">
        Sélectionnez un fichier Excel contenant le modèle d'objets DLMS/COSEM
      </p>
    </div>
  </div>
</template>

<script setup lang="ts">
import type { FileUploadProps } from '@/types'

defineProps<FileUploadProps>()

const emit = defineEmits<{
  'file-upload': [file: File | null]
}>()

const handleFileUpload = (event: Event): void => {
  const target = event.target as HTMLInputElement
  const file = target.files?.[0] || null
  emit('file-upload', file)
}
</script>