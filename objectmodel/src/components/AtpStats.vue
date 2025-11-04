<template>
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

    <div class="flex items-center gap-1.5 px-2 py-1 bg-cyan-100 rounded-md">
      <span class="text-base sm:text-lg font-bold text-cyan-700">{{ stats.procedures }}</span>
      <span class="text-xs text-cyan-600 hidden sm:inline">Proc.</span>
      <span class="text-xs text-cyan-600 sm:hidden">P</span>
    </div>

    <div class="flex items-center gap-1.5 px-2 py-1 bg-red-100 rounded-md">
      <span class="text-base sm:text-lg font-bold text-red-700">{{ stats.tests }}</span>
      <span class="text-xs text-red-600 hidden sm:inline">Tests</span>
      <span class="text-xs text-red-600 sm:hidden">T</span>
    </div>
  </div>
</template>

<script setup lang="ts">
import type { AtpTestStats } from '@/types'

defineProps<{
  stats: AtpTestStats
}>()
</script>
