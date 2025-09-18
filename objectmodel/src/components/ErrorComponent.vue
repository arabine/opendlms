<template>
  <div class="bg-red-100 border border-red-400 text-red-700 px-4 py-3 rounded mb-4">
    <strong>Erreur:</strong> {{ error }}
  </div>
</template>

<script setup lang="ts">
import type { ErrorComponentProps } from '@/types'

defineProps<ErrorComponentProps>()
</script>
