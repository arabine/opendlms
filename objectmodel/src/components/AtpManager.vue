<template>
  <div class="flex flex-col h-full">
    <!-- En-tête avec stats et boutons -->
    <div class="flex-shrink-0 p-4 bg-gray-50 border-b border-gray-200">
      <div class="flex flex-col md:flex-row items-start md:items-center justify-between gap-4">
        <!-- Statistiques à gauche -->
        <div v-if="tests.length > 0" class="flex items-center">
          <AtpStats :stats="stats" />
        </div>
        <div v-else class="flex-1"></div>

        <!-- Boutons de chargement à droite -->
        <div class="flex items-center">
          <AtpFileUpload
            :loading="loading"
            @tests-updated="loadTests"
          />
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
          @add-test="(type) => openAddModal(type)"
        />
      </div>

      <!-- Colonne droite : Detail View -->
      <div class="flex-1 bg-white rounded-lg shadow-lg overflow-hidden border border-gray-200 flex flex-col min-h-0">
        <AtpDetailView
          :test="selectedTest"
          @update="handleUpdateTest"
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
const notification = ref<{ type: 'success' | 'error', message: string } | null>(null)

const stats = computed<AtpTestStats>(() => {
  // Compter les test-cases et procédures validés
  const validatedTestCases = testCases.value.filter(t => t.validated === true)
  const validatedProcedures = procedures.value.filter(p => p.validated === true)

  return {
    total: tests.value.length,
    chapters: tests.value.filter(t => t.type === 'chapter').length,
    sections: tests.value.filter(t => t.type === 'section').length,
    procedures: procedures.value.length,
    tests: testCases.value.length,
    validatedTests: validatedTestCases.length,
    validatedProcedures: validatedProcedures.length
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

const openAddModal = async (type: TestType): Promise<void> => {
  try {
    let parent: string | null = null
    let chapter: string | null = null
    let section: string | null = null
    const defaultType: TestType = type // Utiliser le type passé en paramètre
    let number: string | undefined = undefined

    // Si un test est sélectionné, utiliser son contexte pour définir parent/chapter
    if (selectedTest.value) {
      const selected = selectedTest.value

      if (type === 'section') {
        // Pour une section, utiliser le chapitre sélectionné si c'est un chapitre
        if (selected.type === 'chapter') {
          chapter = selected.number || null
        } else {
          chapter = selected.chapter || null
        }
        parent = null
      } else if (type === 'test-case') {
        // Pour un test-case, utiliser le contexte de la sélection
        if (selected.type === 'chapter') {
          chapter = selected.number || null
        } else {
          chapter = selected.chapter || null
        }
        parent = null
      } else if (type === 'procedure' || type === 'chapter') {
        // Procédures et chapitres n'ont pas de parent
        parent = null
        chapter = null
      }
    }

    // Générer un ID unique et un numéro temporaire
    const timestamp = Date.now()
    const newId = `${defaultType}_${timestamp}`

    // Créer l'élément avec des placeholders selon le type
    const newTest: AtpTest = {
      _id: newId,
      type: defaultType,
      title: `[New ${defaultType}]`,
      parent,
      chapter,
      section,
      timestamp: new Date().toISOString(),
      order: 999999 // Sera réorganisé après
    }

    // Ajouter des champs spécifiques selon le type
    if (defaultType === 'chapter' || defaultType === 'section') {
      const count = tests.value.filter(t => t.type === defaultType).length + 1
      newTest.number = defaultType === 'chapter' ? `${count}` : `${chapter}.${count}`
    } else if (defaultType === 'test-case') {
      newTest.testId = 'NEW-TC-001'
      newTest.useCase = ''
      newTest.scenario = ''
      newTest.testPurpose = ''
      newTest.testStrategy = ''
      newTest.prerequisites = ''
      newTest.preamble = ''
      newTest.testBody = ''
      newTest.testBodySteps = ['']
      newTest.postamble = ''
      newTest.comment = ''
    } else if (defaultType === 'procedure') {
      newTest.references = ''
      newTest.testPurpose = ''
      newTest.procedureBody = ''
      newTest.procedureSteps = ['']
    }

    // Sauvegarder immédiatement
    await atpDatabaseService.saveTest(newTest)

    // Réorganiser les ordres
    await reorderSiblings(parent || null, chapter || null)

    // Recharger et sélectionner le nouvel élément
    await loadTests()

    // Sélectionner et auto-expand au nouveau test
    selectedTest.value = tests.value.find(t => t._id === newId) || null

    if (defaultType === 'procedure') {
      procedureTree.value = expandToNode(procedureTree.value, newId)
    } else {
      testCaseTree.value = expandToNode(testCaseTree.value, newId)
    }

    showNotification('success', 'Élément créé - modifiez les champs dans le panneau de droite')
  } catch (error) {
    console.error('Error creating test:', error)
    showNotification('error', 'Erreur lors de la création')
  }
}

const handleUpdateTest = async (test: AtpTest): Promise<void> => {
  try {
    // Capturer l'état d'expansion avant le reload
    const procedureExpandedState = captureExpandedState(procedureTree.value)
    const testCaseExpandedState = captureExpandedState(testCaseTree.value)

    // Mettre à jour le test dans la base de données
    await atpDatabaseService.updateTest(test)
    showNotification('success', 'Test mis à jour avec succès')

    // Mettre à jour le test sélectionné
    if (selectedTest.value?._id === test._id) {
      selectedTest.value = test
    }

    // Recharger les tests
    await loadTests()

    // Restaurer l'état d'expansion
    procedureTree.value = restoreExpandedState(procedureTree.value, procedureExpandedState)
    testCaseTree.value = restoreExpandedState(testCaseTree.value, testCaseExpandedState)
  } catch (error) {
    console.error('Error updating test:', error)
    showNotification('error', 'Erreur lors de la mise à jour du test')
  }
}

const handleDeleteTest = async (test: AtpTest): Promise<void> => {
  try {
    // Capturer l'état d'expansion avant le reload
    const procedureExpandedState = captureExpandedState(procedureTree.value)
    const testCaseExpandedState = captureExpandedState(testCaseTree.value)

    await atpDatabaseService.deleteTest(test._id)
    showNotification('success', 'Test supprimé avec succès')

    // Déselectionner si c'est le test en cours
    if (selectedTest.value?._id === test._id) {
      selectedTest.value = null
    }

    // Recharger les tests
    await loadTests()

    // Restaurer l'état d'expansion
    procedureTree.value = restoreExpandedState(procedureTree.value, procedureExpandedState)
    testCaseTree.value = restoreExpandedState(testCaseTree.value, testCaseExpandedState)
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
    // STRUCTURE PLATE : sections et test-cases sont au même niveau sous le chapitre
    let newParent: string | null | undefined
    let newChapter: string | null | undefined
    let adjustedPosition = data.position

    if (data.position === 'inside') {
      // On peut seulement drop "inside" un chapitre
      if (targetTest.type === 'chapter') {
        newParent = null
        newChapter = targetTest.number
      } else {
        // Si on essaie de drop "inside" autre chose, on le met "after" à la place
        newParent = targetTest.parent
        newChapter = targetTest.chapter
        adjustedPosition = 'after'
      }
    } else {
      // Pour before/after : on est toujours au même niveau
      // Sections et test-cases partagent le même parent (null) et le même chapitre
      newParent = targetTest.parent
      newChapter = targetTest.chapter
    }

    // Trouver tous les éléments frères (même parent et chapitre), triés par ordre
    // Note: sections et test-cases peuvent être mélangés (structure plate)
    const siblings = tests.value
      .filter(t =>
        t._id !== data.sourceId &&
        (t.parent || null) === (newParent || null) &&
        (t.chapter || null) === (newChapter || null) &&
        (t.type === 'section' || t.type === 'test-case')
      )
      .sort((a, b) => (a.order || 0) - (b.order || 0))

    // Calculer le nouvel ordre en plaçant exactement entre deux éléments
    let newOrder: number

    if (adjustedPosition === 'before') {
      // Trouver l'élément juste avant la cible
      const targetIndex = siblings.findIndex(s => s._id === targetTest._id)
      if (targetIndex > 0) {
        // Il y a un élément avant : placer entre les deux
        const prevOrder = siblings[targetIndex - 1].order || 0
        const currOrder = targetTest.order || 0
        newOrder = (prevOrder + currOrder) / 2
      } else {
        // C'est le premier : placer avant
        const currOrder = targetTest.order || 0
        newOrder = currOrder - 1
      }
    } else if (adjustedPosition === 'after') {
      // Trouver l'élément juste après la cible
      const targetIndex = siblings.findIndex(s => s._id === targetTest._id)
      if (targetIndex >= 0 && targetIndex < siblings.length - 1) {
        // Il y a un élément après : placer entre les deux
        const currOrder = targetTest.order || 0
        const nextOrder = siblings[targetIndex + 1].order || 0
        newOrder = (currOrder + nextOrder) / 2
      } else {
        // C'est le dernier : placer après
        const currOrder = targetTest.order || 0
        newOrder = currOrder + 1
      }
    } else {
      // Insérer comme dernier enfant
      if (siblings.length > 0) {
        const maxOrder = Math.max(...siblings.map(s => s.order || 0))
        newOrder = maxOrder + 1
      } else {
        newOrder = 0
      }
    }

    // Mettre à jour la source
    sourceTest.parent = newParent
    sourceTest.chapter = newChapter
    sourceTest.order = newOrder

    await atpDatabaseService.updateTest(sourceTest)

    // Réorganiser tous les ordres pour avoir des valeurs entières séquentielles
    await reorderSiblings(newParent, newChapter)

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
// Structure plate : sections et test-cases sont mélangés
const reorderSiblings = async (
  parent: string | null | undefined,
  chapter: string | null | undefined
): Promise<void> => {
  // Normaliser les valeurs undefined en null pour les comparaisons
  const normalizedParent = parent || null
  const normalizedChapter = chapter || null

  // Trouver tous les éléments du même niveau (sections ET test-cases)
  const siblings = tests.value
    .filter(t =>
      (t.parent || null) === normalizedParent &&
      (t.chapter || null) === normalizedChapter &&
      (t.type === 'section' || t.type === 'test-case')
    )
    .sort((a, b) => (a.order || 0) - (b.order || 0))

  console.log(`Reordering ${siblings.length} siblings (sections + test-cases)`, { parent: normalizedParent, chapter: normalizedChapter })

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

// Auto-expand jusqu'à un nœud spécifique pour le rendre visible
const expandToNode = (nodes: AtpTreeNode[], targetId: string): AtpTreeNode[] => {
  const expand = (node: AtpTreeNode): { node: AtpTreeNode, found: boolean } => {
    // Si c'est le nœud cible, on le retourne (pas besoin de l'expand lui-même)
    if (node.id === targetId) {
      return { node, found: true }
    }

    // Vérifier récursivement dans les enfants
    let foundInChildren = false
    const updatedChildren = node.children.map(child => {
      const result = expand(child)
      if (result.found) {
        foundInChildren = true
      }
      return result.node
    })

    // Si trouvé dans un enfant, on expand ce nœud
    if (foundInChildren) {
      return {
        node: {
          ...node,
          expanded: true,
          children: updatedChildren
        },
        found: true
      }
    }

    // Sinon, on retourne le nœud inchangé
    return {
      node: {
        ...node,
        children: updatedChildren
      },
      found: false
    }
  }

  return nodes.map(node => expand(node).node)
}

// Gestion de la copie/colle
const handlePasteNode = async (data: { copiedNode: AtpTreeNode, targetNode: AtpTreeNode }): Promise<void> => {
  try {
    // Capturer l'état d'expansion avant le reload
    const procedureExpandedState = captureExpandedState(procedureTree.value)
    const testCaseExpandedState = captureExpandedState(testCaseTree.value)

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

    // Restaurer l'état d'expansion
    procedureTree.value = restoreExpandedState(procedureTree.value, procedureExpandedState)
    testCaseTree.value = restoreExpandedState(testCaseTree.value, testCaseExpandedState)
  } catch (error) {
    console.error('Error pasting node:', error)
    showNotification('error', 'Erreur lors du collage')
  }
}

// Gestion de la duplication
const handleDuplicateNode = async (node: AtpTreeNode): Promise<void> => {
  try {
    // Capturer l'état d'expansion avant le reload
    const procedureExpandedState = captureExpandedState(procedureTree.value)
    const testCaseExpandedState = captureExpandedState(testCaseTree.value)

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

    // Restaurer l'état d'expansion
    procedureTree.value = restoreExpandedState(procedureTree.value, procedureExpandedState)
    testCaseTree.value = restoreExpandedState(testCaseTree.value, testCaseExpandedState)
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

    // Capturer l'état d'expansion avant le reload
    const procedureExpandedState = captureExpandedState(procedureTree.value)
    const testCaseExpandedState = captureExpandedState(testCaseTree.value)

    // Supprimer le test et tous ses enfants
    await deleteNodeAndChildren(id)

    showNotification('success', 'Élément supprimé avec succès')

    // Déselectionner si c'est le test en cours
    if (selectedTest.value?._id === id) {
      selectedTest.value = null
    }

    // Recharger les tests
    await loadTests()

    // Restaurer l'état d'expansion
    procedureTree.value = restoreExpandedState(procedureTree.value, procedureExpandedState)
    testCaseTree.value = restoreExpandedState(testCaseTree.value, testCaseExpandedState)
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
