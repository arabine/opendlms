import PouchDB from 'pouchdb'
import type { AtpTest, AtpTestStats } from '@/types'

class AtpDatabaseService {
  private db: PouchDB.Database<AtpTest>

  constructor() {
    this.db = new PouchDB<AtpTest>('dlms_atp_tests')
  }

  /**
   * Enregistrer un test dans la base de données
   */
  async saveTest(test: AtpTest): Promise<void> {
    try {
      await this.db.put(test)
    } catch (error) {
      console.error('Error saving test:', error)
      throw error
    }
  }

  /**
   * Enregistrer plusieurs tests en une seule opération
   */
  async saveTests(tests: AtpTest[]): Promise<number> {
    try {
      const results = await this.db.bulkDocs(tests)
      return results.filter(r => (r as any).ok).length
    } catch (error) {
      console.error('Error saving tests:', error)
      throw error
    }
  }

  /**
   * Récupérer tous les tests
   */
  async getAllTests(): Promise<AtpTest[]> {
    try {
      const result = await this.db.allDocs<AtpTest>({ include_docs: true })
      return result.rows.map(row => row.doc!).filter(doc => doc !== undefined)
    } catch (error) {
      console.error('Error getting tests:', error)
      throw error
    }
  }

  /**
   * Récupérer un test par ID
   */
  async getTest(id: string): Promise<AtpTest | null> {
    try {
      return await this.db.get(id)
    } catch (error) {
      if ((error as any).status === 404) {
        return null
      }
      console.error('Error getting test:', error)
      throw error
    }
  }

  /**
   * Supprimer un test
   */
  async deleteTest(id: string): Promise<void> {
    try {
      const doc = await this.db.get(id)
      await this.db.remove(doc)
    } catch (error) {
      console.error('Error deleting test:', error)
      throw error
    }
  }

  /**
   * Mettre à jour un test
   */
  async updateTest(test: AtpTest): Promise<void> {
    try {
      // Récupérer le document existant pour avoir le _rev
      const existing = await this.db.get(test._id)
      // Fusionner avec les nouvelles données
      const updated = {
        ...test,
        _rev: existing._rev
      }
      await this.db.put(updated)
    } catch (error) {
      console.error('Error updating test:', error)
      throw error
    }
  }

  /**
   * Supprimer tous les tests
   */
  async clearDatabase(): Promise<void> {
    try {
      const result = await this.db.allDocs()
      const docs = result.rows.map(row => ({
        _id: row.id,
        _rev: row.value.rev,
        _deleted: true
      }))
      await this.db.bulkDocs(docs)
    } catch (error) {
      console.error('Error clearing database:', error)
      throw error
    }
  }

  /**
   * Obtenir les statistiques des tests
   */
  async getStats(): Promise<AtpTestStats> {
    try {
      const tests = await this.getAllTests()
      return {
        total: tests.length,
        chapters: tests.filter(t => t.type === 'chapter').length,
        sections: tests.filter(t => t.type === 'section').length,
        procedures: tests.filter(t => t.type === 'procedure').length,
        tests: tests.filter(t => t.type === 'test-case').length
      }
    } catch (error) {
      console.error('Error getting stats:', error)
      return {
        total: 0,
        chapters: 0,
        sections: 0,
        procedures: 0,
        tests: 0
      }
    }
  }

  /**
   * Exporter tous les tests en JSON
   */
  async exportToJson(): Promise<string> {
    try {
      const tests = await this.getAllTests()
      return JSON.stringify(tests, null, 2)
    } catch (error) {
      console.error('Error exporting tests:', error)
      throw error
    }
  }
}

export const atpDatabaseService = new AtpDatabaseService()
