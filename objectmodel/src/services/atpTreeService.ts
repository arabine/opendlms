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
    
    // Construire l'arbre des procédures (plat, juste une liste)
    const procedureTree: AtpTreeNode[] = procedures.map(proc => ({
      id: proc._id,
      test: proc,
      children: [],
      expanded: false
    }))
    
    // Construire l'arbre des test cases (hiérarchique avec chapitres/sections)
    const testCaseTree: AtpTreeNode[] = chapters.map(chapter => 
      this.createTestCaseNode(chapter, sections, testCases)
    )

    return { 
      procedureTree, 
      testCaseTree,
      procedures: procedures.sort((a, b) => (a.title || '').localeCompare(b.title || '')),
      testCases: testCases.sort((a, b) => (a.testId || '').localeCompare(b.testId || ''))
    }
  }

  /**
   * Créer un nœud d'arbre pour les test cases (structure hiérarchique)
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

    // Trouver les sections qui appartiennent à ce chapitre
    const chapterSections = allSections.filter(s => s.parent === chapter.number)
    
    // Trouver les tests qui appartiennent directement à ce chapitre
    const directTests = allTestCases.filter(t => 
      t.chapter === chapter.number && !t.parent
    )

    // Créer les nœuds pour les sections
    node.children = chapterSections.map(section => {
      const sectionNode: AtpTreeNode = {
        id: section._id,
        test: section,
        children: [],
        expanded: false
      }
      
      // Ajouter les tests de cette section
      const sectionTests = allTestCases.filter(t => t.parent === section.number)
      sectionNode.children = sectionTests.map(t => ({
        id: t._id,
        test: t,
        children: [],
        expanded: false
      }))
      
      return sectionNode
    })

    // Ajouter les tests directs
    node.children.push(...directTests.map(t => ({
      id: t._id,
      test: t,
      children: [],
      expanded: false
    })))

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
}

export const atpTreeService = new AtpTreeService()
