import type { AtpTest, AtpTreeNode } from '@/types'

class AtpTreeService {
  /**
   * Organiser les tests en structure d'arbre hiérarchique
   */
  buildTree(tests: AtpTest[]): { chapters: AtpTreeNode[], procedures: AtpTest[] } {
    // Séparer les procédures des autres éléments
    const procedures = tests.filter(t => t.type === 'procedure')
    const otherTests = tests.filter(t => t.type !== 'procedure')

    // Trier par ligne pour maintenir l'ordre
    otherTests.sort((a, b) => a.line - b.line)

    // Créer les nœuds de chapitres
    const chapters = otherTests
      .filter(t => t.type === 'chapter')
      .map(chapter => this.createNode(chapter, otherTests))

    return { chapters, procedures: procedures.sort((a, b) => a.line - b.line) }
  }

  /**
   * Créer un nœud d'arbre récursivement
   */
  private createNode(test: AtpTest, allTests: AtpTest[]): AtpTreeNode {
    const node: AtpTreeNode = {
      id: test._id,
      test,
      children: [],
      expanded: false
    }

    // Si c'est un chapitre, trouver ses sections et tests
    if (test.type === 'chapter') {
      // Trouver les sections qui appartiennent à ce chapitre
      const sections = allTests.filter(t => 
        t.type === 'section' && t.parent === test.number
      )

      // Trouver les tests qui appartiennent directement à ce chapitre
      const directTests = allTests.filter(t => 
        t.type === 'test-case' && t.chapter === test.number && !t.parent
      )

      // Créer les nœuds pour les sections
      node.children = sections.map(section => this.createNode(section, allTests))

      // Ajouter les tests directs comme nœuds
      node.children.push(...directTests.map(t => ({
        id: t._id,
        test: t,
        children: [],
        expanded: false
      })))
    }

    // Si c'est une section, trouver ses tests
    if (test.type === 'section') {
      const sectionTests = allTests.filter(t => 
        t.type === 'test-case' && t.parent === test.number
      )

      node.children = sectionTests.map(t => ({
        id: t._id,
        test: t,
        children: [],
        expanded: false
      }))
    }

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
