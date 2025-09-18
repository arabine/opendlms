<template>
  <div class="space-y-3">
    <h4 class="font-semibold text-gray-800">Générer les requêtes</h4>
    
    <div class="grid grid-cols-2 gap-3">
      <button
        @click="$emit('generate-get')"
        class="px-4 py-2 bg-green-600 text-white rounded-lg hover:bg-green-700 transition-colors"
      >
        Get Request
      </button>
      <button
        @click="$emit('generate-set')"
        :disabled="!canSet"
        class="px-4 py-2 bg-orange-600 text-white rounded-lg hover:bg-orange-700 transition-colors disabled:bg-gray-400 disabled:cursor-not-allowed"
        :title="!canSet ? 'Cet attribut ne peut pas être modifié' : ''"
      >
        Set Request
      </button>
    </div>
  </div>
</template>

<script setup lang="ts">
import type { RequestButtonsProps } from '@/types'

defineProps<RequestButtonsProps>()

defineEmits<{
  'generate-get': []
  'generate-set': []
}>()
</script>