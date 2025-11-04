<template>
  <div class="flex flex-col gap-2">
    <!-- Première ligne : badges principaux -->
    <div class="flex flex-wrap items-center gap-2">
      <div class="flex items-center gap-1.5 px-2 py-1 bg-purple-100 rounded-md">
        <span class="text-base sm:text-lg font-bold text-purple-700">{{ stats.total }}</span>
        <span class="text-xs text-purple-600 hidden sm:inline">Total</span>
      </div>

      <div class="flex items-center gap-1.5 px-2 py-1 bg-green-100 rounded-md">
        <span class="text-base sm:text-lg font-bold text-green-700">{{ stats.chapters }}</span>
        <span class="text-xs text-green-600 hidden sm:inline">Chapters</span>
        <span class="text-xs text-green-600 sm:hidden">Ch</span>
      </div>

      <div class="flex items-center gap-1.5 px-2 py-1 bg-yellow-100 rounded-md">
        <span class="text-base sm:text-lg font-bold text-yellow-700">{{ stats.sections }}</span>
        <span class="text-xs text-yellow-600 hidden sm:inline">Sections</span>
        <span class="text-xs text-yellow-600 sm:hidden">Sec</span>
      </div>
    </div>

    <!-- Deuxième ligne : badges de validation plus gros -->
    <div class="flex flex-wrap items-center gap-3">
      <div class="flex items-center gap-2 px-4 py-2 bg-red-100 rounded-lg border-2 border-red-300">
        <span class="text-xl sm:text-2xl font-bold text-red-700">{{ stats.validatedTests }}/{{ stats.tests }}</span>
        <span class="text-sm text-red-600">Tests validés</span>
      </div>

      <div class="flex items-center gap-2 px-4 py-2 bg-cyan-100 rounded-lg border-2 border-cyan-300">
        <span class="text-xl sm:text-2xl font-bold text-cyan-700">{{ stats.validatedProcedures }}/{{ stats.procedures }}</span>
        <span class="text-sm text-cyan-600">Proc. validées</span>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import type { AtpTestStats } from '@/types'

defineProps<{
  stats: AtpTestStats
}>()
</script>
