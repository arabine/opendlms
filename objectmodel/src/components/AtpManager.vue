<template>
  <div class="flex flex-col h-screen">
    <!-- En-tête avec upload et stats -->
    <div class="flex-shrink-0 p-4 bg-gray-50 border-b border-gray-200">
      <div class="flex flex-col lg:flex-row lg:items-center lg:justify-between gap-4">
        <AtpFileUpload
          :loading="loading"
          @tests-updated="loadTests"
        />

        <div class="flex flex-col sm:flex-row items-start sm:items-center gap-4">
          <AtpStats
            v-if="tests.length > 0"
            :stats="stats"
            class="flex-shrink-0"
          />

          <!-- Bouton d'ajout -->
          <button
            v-if="tests.length > 0"
            @click="openAddModal"
            class="flex-shrink-0 px-4 py-2 bg-green-600 hover:bg-green-700 text-white rounded-md transition-colors flex items-center gap-2 text-sm whitespace-nowrap"
          >
            <svg class="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 4v16m8-8H4" />
            </svg>
            Ajouter
          </button>
        </div>
      </div>
    </div>

    <!-- Layout deux colonnes : Tree View + Detail View -->
    <div v-if="tests.length > 0" class="flex-1 flex flex-col md:flex-row gap-4 p-4 overflow-hidden">
      <!-- Colonne gauche : Tree View -->
      <div class="w-full md:w-1/3 h-1/2 md:h-auto bg-white rounded-lg shadow-lg overflow-hidden border border-gray-200 flex flex-col">
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
          @move-node="handleMoveNode"
          @paste-node="handlePasteNode"
          @duplicate-node="handleDuplicateNode"
          @delete-node="handleDeleteNode"
        />
      </div>

      <!-- Colonne droite : Detail View -->
      <div class="flex-1 bg-white rounded-lg shadow-lg overflow-hidden border border-gray-200 flex flex-col min-h-0">
        <AtpDetailView
          :test="selectedTest"
          @edit="openEditModal"
          @delete="handleDeleteTest"
        />
      </div>
    </div>

    <!-- État vide -->
    <div v-if="!loading && tests.length === 0" class="flex-1 flex items-center justify-center">
      <div class="text-center py-12 bg-white rounded-lg shadow-lg px-8">
        <svg class="w-16 h-16 mx-auto mb-4 text-gray-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 12h6m-6 4h6m2 5H7a2 2 0 01-2-2V5a2 2 0 012-2h5.586a1 1 0 01.707.293l5.414 5.414a1 1 0 01.293.707V19a2 2 0 01-2 2z" />
        </svg>
        <p class="text-lg font-semibold text-gray-600 mb-2">Aucun test chargé</p>
        <p class="text-sm text-gray-500">Chargez un fichier ATP pour commencer</p>
      </div>
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
import type { AtpTest, AtpTestStats, AtpTreeNode, TestType } from '@/types'

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

    // Initialiser les ordres manquants
    await initializeOrdersIfNeeded()

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

// Initialiser les ordres pour les tests qui n'en ont pas encore
const initializeOrdersIfNeeded = async (): Promise<void> => {
  // Grouper par parent/chapter/type
  const groups = new Map<string, AtpTest[]>()

  for (const test of tests.value) {
    const key = `${test.type}:${test.parent || 'null'}:${test.chapter || 'null'}`
    if (!groups.has(key)) {
      groups.set(key, [])
    }
    groups.get(key)!.push(test)
  }

  // Assigner des ordres séquentiels si nécessaire
  const testsToUpdate: AtpTest[] = []
  for (const group of groups.values()) {
    // Trier par title/testId pour avoir un ordre cohérent
    group.sort((a, b) => (a.title || a.testId || '').localeCompare(b.title || b.testId || ''))

    for (let i = 0; i < group.length; i++) {
      if (group[i].order === undefined) {
        group[i].order = i
        testsToUpdate.push(group[i])
      }
    }
  }

  // Sauvegarder seulement les tests modifiés
  for (const test of testsToUpdate) {
    await atpDatabaseService.updateTest(test)
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

// Gestion du drag & drop
const handleMoveNode = async (data: {
  sourceId: string,
  targetId: string,
  position: 'before' | 'after' | 'inside',
  tab: 'procedures' | 'testcases'
}): Promise<void> => {
  try {
    // Sauvegarder l'état d'expansion avant le déplacement
    const expandedState = data.tab === 'procedures'
      ? captureExpandedState(procedureTree.value)
      : captureExpandedState(testCaseTree.value)

    // Mettre à jour le parent et l'ordre dans la base de données
    const sourceTest = tests.value.find(t => t._id === data.sourceId)
    const targetTest = tests.value.find(t => t._id === data.targetId)

    if (!sourceTest || !targetTest) {
      showNotification('error', 'Tests introuvables')
      return
    }

    // Déterminer le nouveau parent selon la position
    let newParent: string | null | undefined
    let newChapter: string | null | undefined

    if (data.position === 'inside') {
      newParent = targetTest.type === 'chapter' ? targetTest.number : targetTest.parent
      newChapter = targetTest.chapter || targetTest.number
    } else {
      // Si before/after, on garde le même parent que la cible
      newParent = targetTest.parent
      newChapter = targetTest.chapter
    }

    // Trouver tous les éléments frères (même parent) dans la nouvelle position
    const siblings = tests.value.filter(t =>
      t._id !== data.sourceId &&
      t.parent === newParent &&
      t.chapter === newChapter &&
      t.type === sourceTest.type
    )

    // Calculer le nouvel ordre
    let newOrder: number

    if (data.position === 'before') {
      // Insérer avant la cible
      newOrder = targetTest.order !== undefined ? targetTest.order - 0.5 : 0
    } else if (data.position === 'after') {
      // Insérer après la cible
      newOrder = targetTest.order !== undefined ? targetTest.order + 0.5 : siblings.length
    } else {
      // Insérer comme dernier enfant
      const childrenOrders = tests.value
        .filter(t => t.parent === newParent && t.chapter === newChapter && t.type === sourceTest.type)
        .map(t => t.order || 0)
      newOrder = childrenOrders.length > 0 ? Math.max(...childrenOrders) + 1 : 0
    }

    // Mettre à jour la source
    sourceTest.parent = newParent
    sourceTest.chapter = newChapter
    sourceTest.order = newOrder

    await atpDatabaseService.updateTest(sourceTest)

    // Réorganiser tous les ordres pour avoir des valeurs entières séquentielles
    await reorderSiblings(newParent, newChapter, sourceTest.type)

    showNotification('success', 'Élément déplacé avec succès')

    // Recharger les tests pour synchroniser
    await loadTests()

    // Restaurer l'état d'expansion
    if (data.tab === 'procedures') {
      procedureTree.value = restoreExpandedState(procedureTree.value, expandedState)
    } else {
      testCaseTree.value = restoreExpandedState(testCaseTree.value, expandedState)
    }
  } catch (error) {
    console.error('Error moving node:', error)
    showNotification('error', 'Erreur lors du déplacement')
  }
}

// Réorganiser les ordres d'un groupe de frères pour avoir des valeurs entières séquentielles
const reorderSiblings = async (
  parent: string | null | undefined,
  chapter: string | null | undefined,
  type: TestType
): Promise<void> => {
  // Trouver tous les éléments du même niveau
  const siblings = tests.value
    .filter(t => t.parent === parent && t.chapter === chapter && t.type === type)
    .sort((a, b) => (a.order || 0) - (b.order || 0))

  // Réassigner des ordres séquentiels
  for (let i = 0; i < siblings.length; i++) {
    if (siblings[i].order !== i) {
      siblings[i].order = i
      await atpDatabaseService.updateTest(siblings[i])
    }
  }
}

// Capturer l'état d'expansion de tous les nœuds
const captureExpandedState = (nodes: AtpTreeNode[]): Map<string, boolean> => {
  const state = new Map<string, boolean>()

  const capture = (node: AtpTreeNode) => {
    state.set(node.id, node.expanded)
    node.children.forEach(capture)
  }

  nodes.forEach(capture)
  return state
}

// Restaurer l'état d'expansion de tous les nœuds
const restoreExpandedState = (nodes: AtpTreeNode[], state: Map<string, boolean>): AtpTreeNode[] => {
  const restore = (node: AtpTreeNode): AtpTreeNode => {
    const expanded = state.get(node.id) ?? node.expanded
    return {
      ...node,
      expanded,
      children: node.children.map(restore)
    }
  }

  return nodes.map(restore)
}

// Gestion de la copie/colle
const handlePasteNode = async (data: { copiedNode: AtpTreeNode, targetNode: AtpTreeNode }): Promise<void> => {
  try {
    // Dupliquer le nœud copié
    const duplicatedNode = atpTreeService.duplicateNode(data.copiedNode)

    // Mettre à jour les relations parent/enfant
    duplicatedNode.test.parent = data.targetNode.test.type === 'chapter'
      ? data.targetNode.test.number
      : data.targetNode.test.parent
    duplicatedNode.test.chapter = data.targetNode.test.chapter || data.targetNode.test.number

    // Sauvegarder dans la base de données (le nœud et ses enfants)
    await saveNodeAndChildren(duplicatedNode)

    showNotification('success', 'Élément collé avec succès')

    // Recharger les tests
    await loadTests()
  } catch (error) {
    console.error('Error pasting node:', error)
    showNotification('error', 'Erreur lors du collage')
  }
}

// Gestion de la duplication
const handleDuplicateNode = async (node: AtpTreeNode): Promise<void> => {
  try {
    // Dupliquer le nœud
    const duplicatedNode = atpTreeService.duplicateNode(node)

    // Conserver les mêmes relations parent/enfant que l'original
    duplicatedNode.test.parent = node.test.parent
    duplicatedNode.test.chapter = node.test.chapter

    // Sauvegarder dans la base de données (le nœud et ses enfants)
    await saveNodeAndChildren(duplicatedNode)

    showNotification('success', 'Élément dupliqué avec succès')

    // Recharger les tests
    await loadTests()
  } catch (error) {
    console.error('Error duplicating node:', error)
    showNotification('error', 'Erreur lors de la duplication')
  }
}

// Gestion de la suppression via menu contextuel
const handleDeleteNode = async (id: string): Promise<void> => {
  try {
    const test = tests.value.find(t => t._id === id)
    if (!test) return

    // Confirmer avant suppression
    if (!confirm(`Êtes-vous sûr de vouloir supprimer "${test.title}" ?`)) {
      return
    }

    // Supprimer le test et tous ses enfants
    await deleteNodeAndChildren(id)

    showNotification('success', 'Élément supprimé avec succès')

    // Déselectionner si c'est le test en cours
    if (selectedTest.value?._id === id) {
      selectedTest.value = null
    }

    // Recharger les tests
    await loadTests()
  } catch (error) {
    console.error('Error deleting node:', error)
    showNotification('error', 'Erreur lors de la suppression')
  }
}

// Fonction auxiliaire pour sauvegarder un nœud et tous ses enfants
const saveNodeAndChildren = async (node: AtpTreeNode): Promise<void> => {
  // Sauvegarder le nœud
  await atpDatabaseService.saveTest(node.test)

  // Sauvegarder récursivement tous les enfants
  for (const child of node.children) {
    await saveNodeAndChildren(child)
  }
}

// Fonction auxiliaire pour supprimer un nœud et tous ses enfants
const deleteNodeAndChildren = async (id: string): Promise<void> => {
  // Trouver le nœud dans l'arbre
  const node = atpTreeService.findNode(procedureTree.value, id) ||
               atpTreeService.findNode(testCaseTree.value, id)

  if (!node) return

  // Supprimer récursivement tous les enfants
  for (const child of node.children) {
    await deleteNodeAndChildren(child.id)
  }

  // Supprimer le nœud lui-même
  await atpDatabaseService.deleteTest(id)
}

onMounted(() => {
  loadTests()
})
</script>
