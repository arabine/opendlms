<template>
  <div class="space-y-6">
    <AtpFileUpload 
      :loading="loading"
      @tests-updated="loadTests"
    />

    <AtpStats 
      v-if="tests.length > 0"
      :stats="stats"
    />

    <!-- Bouton d'ajout -->
    <div v-if="tests.length > 0" class="flex justify-end">
      <button
        @click="openAddModal"
        class="px-6 py-2 bg-green-600 hover:bg-green-700 text-white rounded-md transition-colors flex items-center gap-2"
      >
        <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 4v16m8-8H4" />
        </svg>
        Ajouter un test
      </button>
    </div>

    <!-- Layout deux colonnes : Tree View + Detail View -->
    <div v-if="tests.length > 0" class="grid grid-cols-12 gap-6 h-[600px]">
      <!-- Colonne gauche : Tree View -->
      <div class="col-span-4 bg-white rounded-lg shadow-lg overflow-hidden border border-gray-200">
        <AtpTreeView
          :procedure-tree="procedureTree"
          :test-case-tree="testCaseTree"
          :procedures="procedures"
          :test-cases="testCases"
          :selected-test="selectedTest"
          :search-query="searchQuery"
          @select="handleSelectTest"
          @toggle-procedure="handleToggleProcedure"
          @toggle-testcase="handleToggleTestCase"
          @update-search="searchQuery = $event"
        />
      </div>

      <!-- Colonne droite : Detail View -->
      <div class="col-span-8 bg-white rounded-lg shadow-lg overflow-hidden border border-gray-200">
        <AtpDetailView
          :test="selectedTest"
          @edit="openEditModal"
          @delete="handleDeleteTest"
        />
      </div>
    </div>

    <!-- État vide -->
    <div v-if="!loading && tests.length === 0" class="text-center py-12 bg-white rounded-lg shadow-lg">
      <svg class="w-16 h-16 mx-auto mb-4 text-gray-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
        <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 12h6m-6 4h6m2 5H7a2 2 0 01-2-2V5a2 2 0 012-2h5.586a1 1 0 01.707.293l5.414 5.414a1 1 0 01.293.707V19a2 2 0 01-2 2z" />
      </svg>
      <p class="text-lg font-semibold text-gray-600 mb-2">Aucun test chargé</p>
      <p class="text-sm text-gray-500">Chargez un fichier ATP pour commencer</p>
    </div>

    <!-- Modal d'édition -->
    <AtpEditModal
      :is-open="isEditModalOpen"
      :test="testToEdit"
      @close="closeEditModal"
      @save="handleSaveTest"
    />

    <!-- Notifications -->
    <div
      v-if="notification"
      class="fixed bottom-4 right-4 px-6 py-3 rounded-lg shadow-lg transition-all z-50"
      :class="notification.type === 'success' ? 'bg-green-600 text-white' : 'bg-red-600 text-white'"
    >
      {{ notification.message }}
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, computed } from 'vue'
import AtpFileUpload from './AtpFileUpload.vue'
import AtpStats from './AtpStats.vue'
import AtpTreeView from './AtpTreeView.vue'
import AtpDetailView from './AtpDetailView.vue'
import AtpEditModal from './AtpEditModal.vue'
import { atpDatabaseService } from '@/services/atpDatabaseService'
import { atpTreeService } from '@/services/atpTreeService'
import type { AtpTest, AtpTestStats, AtpTreeNode } from '@/types'

const loading = ref<boolean>(false)
const tests = ref<AtpTest[]>([])
const searchQuery = ref<string>('')
const selectedTest = ref<AtpTest | null>(null)
const procedureTree = ref<AtpTreeNode[]>([])
const testCaseTree = ref<AtpTreeNode[]>([])
const procedures = ref<AtpTest[]>([])
const testCases = ref<AtpTest[]>([])
const isEditModalOpen = ref<boolean>(false)
const testToEdit = ref<AtpTest | null>(null)
const notification = ref<{ type: 'success' | 'error', message: string } | null>(null)

const stats = computed<AtpTestStats>(() => {
  return {
    total: tests.value.length,
    chapters: tests.value.filter(t => t.type === 'chapter').length,
    sections: tests.value.filter(t => t.type === 'section').length,
    procedures: procedures.value.length,
    tests: testCases.value.length
  }
})

const loadTests = async (): Promise<void> => {
  try {
    loading.value = true
    tests.value = await atpDatabaseService.getAllTests()
    
    // Construire les structures en arbre séparées
    const tree = atpTreeService.buildTree(tests.value)
    procedureTree.value = tree.procedureTree
    testCaseTree.value = tree.testCaseTree
    procedures.value = tree.procedures
    testCases.value = tree.testCases
  } catch (error) {
    console.error('Error loading tests:', error)
    showNotification('error', 'Erreur lors du chargement des tests')
  } finally {
    loading.value = false
  }
}

const handleSelectTest = (test: AtpTest): void => {
  selectedTest.value = test
}

const handleToggleProcedure = (id: string): void => {
  procedureTree.value = atpTreeService.toggleNode(procedureTree.value, id)
}

const handleToggleTestCase = (id: string): void => {
  testCaseTree.value = atpTreeService.toggleNode(testCaseTree.value, id)
}

const openAddModal = (): void => {
  testToEdit.value = null
  isEditModalOpen.value = true
}

const openEditModal = (test: AtpTest): void => {
  testToEdit.value = test
  isEditModalOpen.value = true
}

const closeEditModal = (): void => {
  isEditModalOpen.value = false
  testToEdit.value = null
}

const handleSaveTest = async (test: AtpTest): Promise<void> => {
  try {
    if (testToEdit.value) {
      // Mise à jour
      await atpDatabaseService.updateTest(test)
      showNotification('success', 'Test mis à jour avec succès')
      
      // Mettre à jour le test sélectionné si c'est celui en cours
      if (selectedTest.value?._id === test._id) {
        selectedTest.value = test
      }
    } else {
      // Création
      await atpDatabaseService.saveTest(test)
      showNotification('success', 'Test créé avec succès')
    }
    
    // Recharger les tests
    await loadTests()
  } catch (error) {
    console.error('Error saving test:', error)
    showNotification('error', 'Erreur lors de la sauvegarde du test')
  }
}

const handleDeleteTest = async (test: AtpTest): Promise<void> => {
  try {
    await atpDatabaseService.deleteTest(test._id)
    showNotification('success', 'Test supprimé avec succès')
    
    // Déselectionner si c'est le test en cours
    if (selectedTest.value?._id === test._id) {
      selectedTest.value = null
    }
    
    // Recharger les tests
    await loadTests()
  } catch (error) {
    console.error('Error deleting test:', error)
    showNotification('error', 'Erreur lors de la suppression du test')
  }
}

const showNotification = (type: 'success' | 'error', message: string): void => {
  notification.value = { type, message }
  setTimeout(() => {
    notification.value = null
  }, 3000)
}

onMounted(() => {
  loadTests()
})
</script>
