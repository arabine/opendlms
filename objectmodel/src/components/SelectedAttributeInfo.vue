<template>
  <div class="bg-blue-50 p-4 rounded-lg">
    <h3 class="text-lg font-semibold text-blue-800 mb-2">Attribut sélectionné</h3>
    <div class="text-sm space-y-1">
      <p><strong>Objet:</strong> {{ selectedObject.name }}</p>
      <p><strong>Code OBIS:</strong> {{ selectedObject.obisCode }}</p>
      <p><strong>Classe ID:</strong> {{ selectedObject.classId }}</p>
      <p><strong>Attribut:</strong> {{ selectedAttribute.name }} (#{{ selectedAttribute.number }})</p>
      <p><strong>Type:</strong> {{ selectedAttribute.type }}</p>
    </div>
  </div>
</template>

<script setup lang="ts">
import type { SelectedAttributeInfoProps } from '@/types'

defineProps<SelectedAttributeInfoProps>()
</script>