import mammoth from 'mammoth'
import type { AtpTest } from '@/types'

class AtpParserService {
  /**
   * Lire et parser un fichier Word ou texte
   */
  async parseFile(file: File): Promise<AtpTest[]> {
    try {
      let content: AtpTest[]

      if (file.name.endsWith('.docx')) {
        // Parser le fichier Word avec extraction des tableaux
        content = await this.parseWordFile(file)
      } else if (file.name.endsWith('.txt')) {
        // Fallback sur l'ancien parsing texte
        const textContent = await this.readTextFile(file)
        content = this.parseContent(textContent)
      } else {
        throw new Error('Format de fichier non supporté. Utilisez .docx ou .txt')
      }

      // Ajouter les timestamps côté client
      content.forEach(test => {
        test.timestamp = test.timestamp || new Date().toISOString()
      })

      return content
    } catch (error) {
      console.error('Error parsing file:', error)
      throw error
    }
  }

  /**
   * Parser un fichier Word avec extraction des tableaux
   */
  private async parseWordFile(file: File): Promise<AtpTest[]> {
    try {
      const arrayBuffer = await file.arrayBuffer()
      
      // Extraire le HTML avec les tableaux préservés
      const result = await mammoth.convertToHtml({ 
        arrayBuffer,
        convertImage: mammoth.images.inline(() => ({ src: '' }))
      })
      
      const html = result.value
      
      // Parser le HTML pour extraire les tests et tableaux
      return this.parseHtmlContent(html)
    } catch (error) {
      console.error('Error parsing Word file:', error)
      throw new Error('Erreur lors de la lecture du fichier Word')
    }
  }

  /**
   * Lire un fichier Word avec mammoth (fallback)
   */
  private async readWordFileBasic(file: File): Promise<string> {
    try {
      const arrayBuffer = await file.arrayBuffer()
      const result = await mammoth.extractRawText({ arrayBuffer })
      return result.value
    } catch (error) {
      console.error('Error reading Word file:', error)
      throw new Error('Erreur lors de la lecture du fichier Word')
    }
  }

  /**
   * Parser le HTML pour extraire les tests et tableaux
   */
  private parseHtmlContent(html: string): AtpTest[] {
    const parser = new DOMParser()
    const doc = parser.parseFromString(html, 'text/html')
    
    const results: AtpTest[] = []
    let idCounter = 1
    let currentChapter: { number: string; title: string } | null = null

    // Extraire les paragraphes (titres/chapitres)
    const paragraphs = doc.querySelectorAll('p, h1, h2, h3, h4, h5, h6')
    paragraphs.forEach((para) => {
      const text = para.textContent?.trim() || ''
      if (!text) return

      // Détecter les chapitres principaux (ex: "7 Test Suites")
      const chapterMatch = text.match(/^(\d+)[\.\s]+(.+?)$/)
      if (chapterMatch && chapterMatch[1].length <= 2) {
        const chapterNum = chapterMatch[1]
        const chapterTitle = chapterMatch[2].trim()
        currentChapter = { number: chapterNum, title: chapterTitle }
        
        results.push({
          _id: `chapter_${chapterNum}_${idCounter++}`,
          type: 'chapter',
          number: chapterNum,
          title: chapterTitle,
          timestamp: new Date().toISOString()
        })
        return
      }

      // Détecter les sous-sections (ex: "6.3.1 WriteAttributes")
      const sectionMatch = text.match(/^(\d+\.\d+(?:\.\d+)?)\s+(.+?)$/)
      if (sectionMatch) {
        const sectionNum = sectionMatch[1]
        const sectionTitle = sectionMatch[2].trim()
        
        const type = sectionNum.startsWith('6.') ? 'procedure' : 'section'
        
        results.push({
          _id: `section_${sectionNum.replace(/\./g, '_')}_${idCounter++}`,
          type: type,
          number: sectionNum,
          title: sectionTitle,
          parent: currentChapter ? currentChapter.number : null,
          timestamp: new Date().toISOString()
        })
      }
    })

    // Extraire les tableaux
    const tables = doc.querySelectorAll('table')
    tables.forEach((table) => {
      const rows = table.querySelectorAll('tr')
      if (rows.length < 2) return // Pas assez de lignes

      // Extraire les en-têtes
      const headerCells = rows[0].querySelectorAll('th, td')
      const headers = Array.from(headerCells).map(cell => cell.textContent?.trim() || '')

      // Détecter si c'est un tableau de test case (première cellule = "Test Case")
      if (headers[0] === 'Test Case' && headers.length === 2) {
        const testCaseHeader = headers[1]
        
        // Extraire le test ID et le titre
        let testId = ''
        let testTitle = ''
        
        if (testCaseHeader.includes('ACESM-') && testCaseHeader.includes('-TC')) {
          const parts = testCaseHeader.split(':', 1)
          if (parts.length >= 1) {
            testId = parts[0].trim()
            testTitle = testCaseHeader.substring(testId.length + 1).trim()
          } else {
            testId = testCaseHeader
            testTitle = testCaseHeader
          }
        } else {
          testTitle = testCaseHeader
        }

        // Extraire toutes les données du tableau
        const tableData: Record<string, string> = {}
        for (let i = 1; i < rows.length; i++) {
          const cells = rows[i].querySelectorAll('td')
          if (cells.length >= 2) {
            const key = cells[0].textContent?.trim() || ''
            const value = cells[1].textContent?.trim() || ''
            if (key) {
              tableData[key] = value
            }
          }
        }

        // Créer l'entrée de test case
        results.push({
          _id: `test_${testId || 'unknown'}_${idCounter++}`,
          type: 'test-case',
          testId: testId,
          title: testTitle,
          chapter: currentChapter ? currentChapter.number : null,
          timestamp: new Date().toISOString(),
          tableColumns: headers,
          tableData: tableData,
          // Champs détaillés
          useCase: tableData['Use Case'] || '',
          scenario: tableData['Scenario'] || '',
          testPurpose: tableData['Test purpose'] || '',
          testStrategy: tableData['Test strategy'] || '',
          aaFilter: tableData['AA filter'] || '',
          prerequisites: tableData['Prerequisites'] || '',
          preamble: tableData['Preamble'] || '',
          testBody: tableData['Test body'] || '',
          postamble: tableData['Postamble'] || '',
          comment: tableData['Comment'] || ''
        })
      }
    })

    return results
  }

  /**
   * Lire un fichier texte
   */
  private async readTextFile(file: File): Promise<string> {
    return new Promise((resolve, reject) => {
      const reader = new FileReader()
      reader.onload = (e) => {
        resolve(e.target?.result as string)
      }
      reader.onerror = () => {
        reject(new Error('Erreur lors de la lecture du fichier'))
      }
      reader.readAsText(file)
    })
  }

  /**
   * Parser le contenu texte et extraire les tests (fallback)
   */
  private parseContent(content: string): AtpTest[] {
    const lines = content.split('\n')
    const results: AtpTest[] = []
    let currentChapter: { number: string; title: string } | null = null
    let idCounter = 1

    for (let i = 0; i < lines.length; i++) {
      const line = lines[i].trim()
      if (!line) continue

      // Détecter les chapitres principaux (ex: "7 Test Suites", "6 Test procedures")
      const chapterMatch = line.match(/^(\d+)[\.\s]+(.+?)(?:\[|$)/)
      if (chapterMatch && chapterMatch[1].length <= 2) {
        const chapterNum = chapterMatch[1]
        const chapterTitle = chapterMatch[2].trim()
        currentChapter = { number: chapterNum, title: chapterTitle }
        
        results.push({
          _id: `chapter_${chapterNum}_${idCounter++}`,
          type: 'chapter',
          number: chapterNum,
          title: chapterTitle,
          line: i + 1,
          timestamp: new Date().toISOString()
        })
        continue
      }

      // Détecter les sous-sections (ex: "6.3.1 WriteAttributes")
      const sectionMatch = line.match(/^(\d+\.\d+(?:\.\d+)?)\s+(.+?)(?:\[|$)/)
      if (sectionMatch) {
        const sectionNum = sectionMatch[1]
        const sectionTitle = sectionMatch[2].trim()
        
        const type = sectionNum.startsWith('6.') ? 'procedure' : 'section'
        
        results.push({
          _id: `section_${sectionNum.replace(/\./g, '_')}_${idCounter++}`,
          type: type,
          number: sectionNum,
          title: sectionTitle,
          line: i + 1,
          parent: currentChapter ? currentChapter.number : null,
          timestamp: new Date().toISOString()
        })
        continue
      }

      // Détecter les cas de test (ex: "ACESM-CV-TC01: Push connectivity verification")
      const testMatch = line.match(/(ACESM-[A-Z]+-TC\d+):?\s*(.+?)(?:\[|$)/)
      if (testMatch) {
        const testId = testMatch[1]
        const testTitle = testMatch[2].trim()
        
        results.push({
          _id: `test_${testId}_${idCounter++}`,
          type: 'test-case',
          testId: testId,
          title: testTitle,
          line: i + 1,
          chapter: currentChapter ? currentChapter.number : null,
          timestamp: new Date().toISOString()
        })
      }
    }

    return results
  }
}

export const atpParserService = new AtpParserService()
