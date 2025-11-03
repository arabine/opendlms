<template>
  <div class="grid grid-cols-2 md:grid-cols-5 gap-4 mb-6">
    <div class="bg-gradient-to-br from-purple-500 to-purple-600 rounded-lg p-4 text-white shadow-lg">
      <div class="text-3xl font-bold">{{ stats.total }}</div>
      <div class="text-sm opacity-90 mt-1">Total éléments</div>
    </div>
    
    <div class="bg-gradient-to-br from-green-500 to-green-600 rounded-lg p-4 text-white shadow-lg">
      <div class="text-3xl font-bold">{{ stats.chapters }}</div>
      <div class="text-sm opacity-90 mt-1">Chapitres</div>
    </div>
    
    <div class="bg-gradient-to-br from-yellow-500 to-yellow-600 rounded-lg p-4 text-white shadow-lg">
      <div class="text-3xl font-bold">{{ stats.sections }}</div>
      <div class="text-sm opacity-90 mt-1">Sections</div>
    </div>
    
    <div class="bg-gradient-to-br from-cyan-500 to-cyan-600 rounded-lg p-4 text-white shadow-lg">
      <div class="text-3xl font-bold">{{ stats.procedures }}</div>
      <div class="text-sm opacity-90 mt-1">Procédures</div>
    </div>
    
    <div class="bg-gradient-to-br from-red-500 to-red-600 rounded-lg p-4 text-white shadow-lg">
      <div class="text-3xl font-bold">{{ stats.tests }}</div>
      <div class="text-sm opacity-90 mt-1">Cas de test</div>
    </div>
  </div>
</template>

<script setup lang="ts">
import type { AtpTestStats } from '@/types'

defineProps<{
  stats: AtpTestStats
}>()
</script>
