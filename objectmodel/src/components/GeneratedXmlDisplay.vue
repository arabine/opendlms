<template>
  <div class="space-y-3">
    <div class="flex items-center justify-between">
      <h4 class="font-semibold text-gray-800">XML Généré</h4>
      <button
        @click="$emit('copy-to-clipboard')"
        :class="copySuccess ? 'copy-success' : ''"
        class="px-3 py-1 text-sm bg-blue-600 text-white rounded hover:bg-blue-700 transition-colors"
      >
        {{ copySuccess ? 'Copié!' : 'Copier' }}
      </button>
    </div>
    <div class="bg-gray-900 text-green-400 p-4 rounded-lg overflow-x-auto">
      <pre class="xml-output text-sm">{{ generatedXml }}</pre>
    </div>
  </div>
</template>

<script setup lang="ts">
import type { GeneratedXmlDisplayProps } from '@/types'

defineProps<GeneratedXmlDisplayProps>()

defineEmits<{
  'copy-to-clipboard': []
}>()
</script>