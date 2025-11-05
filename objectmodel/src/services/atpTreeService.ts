import type { AtpTest, AtpTreeNode } from '@/types'

class AtpTreeService {
  /**
   * Organiser les tests en structure d'arbre hiérarchique
   * Sépare les procédures et les test cases en deux arbres distincts
   */
  buildTree(tests: AtpTest[]): {
    procedureTree: AtpTreeNode[],
    testCaseTree: AtpTreeNode[],
    procedures: AtpTest[],
    testCases: AtpTest[]
  } {
    // Séparer les procédures des test cases
    const procedures = tests.filter(t => t.type === 'procedure')
    const testCases = tests.filter(t => t.type === 'test-case')
    const chapters = tests.filter(t => t.type === 'chapter')
    const sections = tests.filter(t => t.type === 'section')

    // Trier par order (ou par défaut alphabétiquement si pas d'order)
    const sortByOrder = (a: AtpTest, b: AtpTest) => {
      if (a.order !== undefined && b.order !== undefined) {
        return a.order - b.order
      }
      if (a.order !== undefined) return -1
      if (b.order !== undefined) return 1
      return (a.title || '').localeCompare(b.title || '')
    }

    const sortedProcedures = [...procedures].sort(sortByOrder)
    const sortedChapters = [...chapters].sort(sortByOrder)

    // Construire l'arbre des procédures (plat, juste une liste)
    const procedureTree: AtpTreeNode[] = sortedProcedures.map(proc => ({
      id: proc._id,
      test: proc,
      children: [],
      expanded: false
    }))

    // Construire l'arbre des test cases (hiérarchique avec chapitres/sections)
    const testCaseTree: AtpTreeNode[] = sortedChapters.map(chapter =>
      this.createTestCaseNode(chapter, sections, testCases)
    )

    // Trouver les sections orphelines (sans parent valide)
    const usedSections = new Set<string>()
    testCaseTree.forEach(chapterNode => {
      chapterNode.children.forEach(child => {
        if (child.test.type === 'section') {
          usedSections.add(child.test._id)
        }
      })
    })
    const orphanSections = sections.filter(s => !usedSections.has(s._id)).sort(sortByOrder)

    // Trouver les test cases orphelins (sans parent/chapter valide)
    const usedTestCases = new Set<string>()
    const collectUsedTestCases = (node: AtpTreeNode) => {
      node.children.forEach(child => {
        if (child.test.type === 'test-case') {
          usedTestCases.add(child.test._id)
        }
        collectUsedTestCases(child)
      })
    }
    testCaseTree.forEach(collectUsedTestCases)
    const orphanTestCases = testCases.filter(t => !usedTestCases.has(t._id)).sort(sortByOrder)

    // Ajouter les orphelins à la fin de l'arbre
    orphanSections.forEach(section => {
      testCaseTree.push({
        id: section._id,
        test: section,
        children: [],
        expanded: false
      })
    })
    orphanTestCases.forEach(testCase => {
      testCaseTree.push({
        id: testCase._id,
        test: testCase,
        children: [],
        expanded: false
      })
    })

    // Log des statistiques de construction de l'arbre
    console.log('Tree built:', {
      procedures: procedures.length,
      chapters: chapters.length,
      sections: sections.length,
      testCases: testCases.length,
      orphanSections: orphanSections.length,
      orphanTestCases: orphanTestCases.length,
      procedureTreeNodes: procedureTree.length,
      testCaseTreeNodes: testCaseTree.length
    })

    return {
      procedureTree,
      testCaseTree,
      procedures: sortedProcedures,
      testCases: testCases.sort((a, b) => (a.testId || '').localeCompare(b.testId || ''))
    }
  }

  /**
   * Créer un nœud d'arbre pour les test cases (structure PLATE : sections et tests au même niveau)
   */
  private createTestCaseNode(
    chapter: AtpTest,
    allSections: AtpTest[],
    allTestCases: AtpTest[]
  ): AtpTreeNode {
    const node: AtpTreeNode = {
      id: chapter._id,
      test: chapter,
      children: [],
      expanded: false
    }

    // Fonction de tri par order
    const sortByOrder = (a: AtpTest, b: AtpTest) => {
      if (a.order !== undefined && b.order !== undefined) {
        return a.order - b.order
      }
      if (a.order !== undefined) return -1
      if (b.order !== undefined) return 1
      return (a.title || a.testId || '').localeCompare(b.title || b.testId || '')
    }

    // Trouver toutes les sections ET tous les test-cases de ce chapitre
    const chapterSections = allSections
      .filter(s => s.chapter === chapter.number)
      .map(s => ({ type: 'section' as const, item: s }))

    const chapterTests = allTestCases
      .filter(t => t.chapter === chapter.number)
      .map(t => ({ type: 'test' as const, item: t }))

    // Combiner sections et tests, puis trier par order
    const allChildren = [...chapterSections, ...chapterTests]
      .sort((a, b) => sortByOrder(a.item, b.item))

    // Créer les nœuds dans l'ordre (PLAT - pas de hiérarchie section > test)
    node.children = allChildren.map(child => ({
      id: child.item._id,
      test: child.item,
      children: [], // Toujours vide - structure plate
      expanded: false
    }))

    return node
  }

  /**
   * Trouver un nœud dans l'arbre par ID
   */
  findNode(nodes: AtpTreeNode[], id: string): AtpTreeNode | null {
    for (const node of nodes) {
      if (node.id === id) {
        return node
      }
      if (node.children.length > 0) {
        const found = this.findNode(node.children, id)
        if (found) return found
      }
    }
    return null
  }

  /**
   * Basculer l'état expanded d'un nœud
   */
  toggleNode(nodes: AtpTreeNode[], id: string): AtpTreeNode[] {
    return nodes.map(node => {
      if (node.id === id) {
        return { ...node, expanded: !node.expanded }
      }
      if (node.children.length > 0) {
        return { ...node, children: this.toggleNode(node.children, id) }
      }
      return node
    })
  }

  /**
   * Filtrer l'arbre selon une recherche
   */
  filterTree(nodes: AtpTreeNode[], query: string): AtpTreeNode[] {
    if (!query) return nodes

    const lowerQuery = query.toLowerCase()

    return nodes
      .map(node => {
        const matchesNode = this.testMatchesQuery(node.test, lowerQuery)
        const filteredChildren = this.filterTree(node.children, query)

        if (matchesNode || filteredChildren.length > 0) {
          return {
            ...node,
            children: filteredChildren,
            expanded: filteredChildren.length > 0 // Auto-expand si des enfants correspondent
          }
        }
        return null
      })
      .filter((node): node is AtpTreeNode => node !== null)
  }

  /**
   * Vérifier si un test correspond à la requête de recherche
   */
  private testMatchesQuery(test: AtpTest, query: string): boolean {
    const searchableText = `${test.title || ''} ${test.number || ''} ${test.testId || ''}`.toLowerCase()
    return searchableText.includes(query)
  }

  /**
   * Filtrer les procédures selon une recherche
   */
  filterProcedures(procedures: AtpTest[], query: string): AtpTest[] {
    if (!query) return procedures
    const lowerQuery = query.toLowerCase()
    return procedures.filter(proc => this.testMatchesQuery(proc, lowerQuery))
  }

  /**
   * Déplacer un nœud dans l'arbre (drag & drop)
   */
  moveNode(
    nodes: AtpTreeNode[],
    sourceId: string,
    targetId: string,
    position: 'before' | 'after' | 'inside' = 'inside'
  ): AtpTreeNode[] {
    // Trouver les nœuds source et target
    const sourceNode = this.findNode(nodes, sourceId)
    const targetNode = this.findNode(nodes, targetId)

    if (!sourceNode || !targetNode || sourceId === targetId) {
      return nodes
    }

    // Vérifier que la source n'est pas un parent de la cible (éviter les boucles)
    if (this.isAncestor(sourceNode, targetId)) {
      return nodes
    }

    // Retirer le nœud source de sa position actuelle
    const nodesWithoutSource = this.removeNode(nodes, sourceId)

    // Ajouter le nœud selon la position
    let updatedNodes: AtpTreeNode[]
    if (position === 'inside') {
      updatedNodes = this.addNodeAsChild(nodesWithoutSource, sourceNode, targetId)
    } else if (position === 'before') {
      updatedNodes = this.insertNodeBefore(nodesWithoutSource, sourceNode, targetId)
    } else {
      updatedNodes = this.insertNodeAfter(nodesWithoutSource, sourceNode, targetId)
    }

    return updatedNodes
  }

  /**
   * Vérifier si un nœud est l'ancêtre d'un autre
   */
  private isAncestor(node: AtpTreeNode, targetId: string): boolean {
    if (node.id === targetId) return true

    for (const child of node.children) {
      if (this.isAncestor(child, targetId)) return true
    }

    return false
  }

  /**
   * Retirer un nœud de l'arbre
   */
  private removeNode(nodes: AtpTreeNode[], id: string): AtpTreeNode[] {
    return nodes
      .filter(node => node.id !== id)
      .map(node => ({
        ...node,
        children: this.removeNode(node.children, id)
      }))
  }

  /**
   * Ajouter un nœud comme enfant d'un autre nœud
   */
  private addNodeAsChild(nodes: AtpTreeNode[], nodeToAdd: AtpTreeNode, targetId: string): AtpTreeNode[] {
    return nodes.map(node => {
      if (node.id === targetId) {
        return {
          ...node,
          children: [...node.children, { ...nodeToAdd }],
          expanded: node.expanded || true // Garder l'état expanded ou l'ouvrir si fermé
        }
      }
      return {
        ...node,
        children: this.addNodeAsChild(node.children, nodeToAdd, targetId)
      }
    })
  }

  /**
   * Insérer un nœud avant un autre nœud
   */
  private insertNodeBefore(nodes: AtpTreeNode[], nodeToAdd: AtpTreeNode, targetId: string): AtpTreeNode[] {
    const result: AtpTreeNode[] = []

    for (const node of nodes) {
      if (node.id === targetId) {
        // Insérer le nouveau nœud avant le nœud cible
        result.push({ ...nodeToAdd })
        result.push(node)
      } else {
        // Vérifier récursivement dans les enfants
        const updatedChildren = this.insertNodeBefore(node.children, nodeToAdd, targetId)
        if (updatedChildren !== node.children) {
          result.push({ ...node, children: updatedChildren })
        } else {
          result.push(node)
        }
      }
    }

    return result
  }

  /**
   * Insérer un nœud après un autre nœud
   */
  private insertNodeAfter(nodes: AtpTreeNode[], nodeToAdd: AtpTreeNode, targetId: string): AtpTreeNode[] {
    const result: AtpTreeNode[] = []

    for (const node of nodes) {
      if (node.id === targetId) {
        // Insérer le nœud cible puis le nouveau nœud
        result.push(node)
        result.push({ ...nodeToAdd })
      } else {
        // Vérifier récursivement dans les enfants
        const updatedChildren = this.insertNodeAfter(node.children, nodeToAdd, targetId)
        if (updatedChildren !== node.children) {
          result.push({ ...node, children: updatedChildren })
        } else {
          result.push(node)
        }
      }
    }

    return result
  }

  /**
   * Dupliquer un nœud (copie)
   */
  duplicateNode(node: AtpTreeNode): AtpTreeNode {
    const timestamp = Date.now()

    // Copier le test en excluant _rev pour éviter les conflits PouchDB
    const { _rev, ...testWithoutRev } = node.test

    const newTest = {
      ...testWithoutRev,
      _id: `${node.test.type}_${timestamp}_copy`,
      title: `${node.test.title} (copie)`,
      timestamp: new Date().toISOString()
    }

    return {
      id: newTest._id,
      test: newTest,
      children: node.children.map(child => this.duplicateNode(child)),
      expanded: false
    }
  }

  /**
   * Ajouter un nœud à la racine d'un arbre
   */
  addNodeToRoot(nodes: AtpTreeNode[], node: AtpTreeNode): AtpTreeNode[] {
    return [...nodes, node]
  }
}

export const atpTreeService = new AtpTreeService()
