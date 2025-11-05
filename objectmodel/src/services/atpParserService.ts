import type { AtpTest } from '@/types'

class AtpParserService {
  /**
   * Lire et parser un fichier HTML, Markdown ou texte contenant la structure ATP
   */
  async parseFile(file: File, fileType?: 'procedures' | 'test-cases' | 'auto'): Promise<AtpTest[]> {
    try {
      let content: AtpTest[]
      const type = fileType || 'auto' // Par défaut auto-détection

      if (file.name.endsWith('.md') || file.name.endsWith('.markdown')) {
        // Parser le fichier Markdown
        const markdownContent = await this.readTextFile(file)
        
        if (type === 'procedures') {
          content = this.parseMarkdownProcedures(markdownContent)
        } else if (type === 'test-cases') {
          content = this.parseMarkdownTestCases(markdownContent)
        } else {
          // Auto-détection
          content = this.parseMarkdownFile(markdownContent)
        }
      } else if (file.name.endsWith('.html') || file.name.endsWith('.htm')) {
        // Parser le fichier HTML avec extraction des procédures et test cases
        const htmlContent = await this.readTextFile(file)
        content = await this.parseHtmlFile(htmlContent)
      } else if (file.name.endsWith('.txt')) {
        // Fallback sur l'ancien parsing texte
        const textContent = await this.readTextFile(file)
        content = this.parseContent(textContent)
      } else {
        throw new Error('Format de fichier non supporté. Utilisez .md, .html ou .txt')
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
   * Parser un fichier HTML ATP
   */
  private async parseHtmlFile(htmlContent: string): Promise<AtpTest[]> {
    try {
      // Parser le HTML pour extraire les tests et tableaux
      return this.parseHtmlContent(htmlContent)
    } catch (error) {
      console.error('Error parsing HTML file:', error)
      throw new Error('Erreur lors de la lecture du fichier HTML')
    }
  }

  /**
   * Parser un fichier Markdown ATP
   */
  private parseMarkdownFile(markdownContent: string): AtpTest[] {
    try {
      const lines = markdownContent.split('\n')
      const results: AtpTest[] = []
      let idCounter = 1
      let currentChapter: { number: string; title: string } | null = null
      let currentSection: { number: string; title: string } | null = null
      let lineIndex = 0

      while (lineIndex < lines.length) {
        const line = lines[lineIndex].trim()

        // Détecter les chapitres (##)
        if (line.startsWith('## ') && !line.startsWith('### ')) {
          const chapterTitle = line.substring(3).trim()
          const chapterNum = String(results.filter(r => r.type === 'chapter').length + 1)
          
          currentChapter = { number: chapterNum, title: chapterTitle }
          currentSection = null
          
          results.push({
            _id: `chapter_${chapterNum}_${idCounter++}`,
            type: 'chapter',
            number: chapterNum,
            title: chapterTitle,
            timestamp: new Date().toISOString()
          })
          
          lineIndex++
          continue
        }

        // Détecter les sections (###) qui ne sont pas des test cases
        if (line.startsWith('### ') && !line.includes('ACESM-')) {
          const sectionTitle = line.substring(4).trim()
          const sectionNum = currentChapter ? `${currentChapter.number}.${results.filter(r => r.type === 'section' && r.chapter === currentChapter?.number).length + 1}` : String(idCounter)
          
          currentSection = { number: sectionNum, title: sectionTitle }
          
          results.push({
            _id: `section_${sectionNum.replace(/\./g, '_')}_${idCounter++}`,
            type: 'section',
            number: sectionNum,
            title: sectionTitle,
            parent: currentChapter ? currentChapter.number : null,
            chapter: currentChapter ? currentChapter.number : null,
            timestamp: new Date().toISOString()
          })
          
          lineIndex++
          continue
        }

        // Détecter les test cases (### ou #### avec ACESM-)
        if ((line.startsWith('### ') || line.startsWith('#### ')) && line.includes('ACESM-')) {
          const testMatch = line.match(/###[#]?\s+(ACESM-[A-Z0-9]+-TC\d+)\s*[,:]\s*(.+)/)
          if (testMatch) {
            const testId = testMatch[1]
            const testTitle = testMatch[2].trim()
            
            // Chercher le tableau associé
            let tableStartIndex = lineIndex + 1
            while (tableStartIndex < lines.length && !lines[tableStartIndex].trim().startsWith('+-')) {
              tableStartIndex++
            }
            
            if (tableStartIndex < lines.length) {
              const tableData = this.parseMarkdownTable(lines, tableStartIndex)
              
              results.push({
                _id: `test_${testId}_${idCounter++}`,
                type: 'test-case',
                testId: testId,
                title: testTitle,
                chapter: currentChapter ? currentChapter.number : null,
                section: currentSection ? currentSection.number : null,
                timestamp: new Date().toISOString(),
                tableColumns: ['Field', 'Value'],
                tableData: tableData,
                // Champs détaillés
                references: tableData['References'] || '',
                testPurpose: tableData['Test purpose'] || '',
                testStrategy: tableData['Test strategy'] || '',
                aaFilter: tableData['AA filter'] || tableData['AAs used'] || '',
                prerequisites: tableData['Prerequisites'] || '',
                expectedResult: tableData['Expected result'] || '',
                preamble: tableData['Preamble'] || '',
                testBody: tableData['Test body'] || '',
                testBodySteps: tableData['Test body'] ? [tableData['Test body']] : [],
                postamble: tableData['Postamble'] || '',
                comment: tableData['Comment'] || ''
              })
            }
          }
          
          lineIndex++
          continue
        }

        // Détecter les procédures (Table X -- Nom)
        if (line.match(/^Table\s+\d+\s+--\s+.+/) && !line.includes('ACESM-')) {
          const tableNameMatch = line.match(/^Table\s+(\d+)\s+--\s+(.+)/)
          if (tableNameMatch) {
            const procedureName = tableNameMatch[2].trim()
            
            // Chercher le tableau associé
            let tableStartIndex = lineIndex + 1
            while (tableStartIndex < lines.length && !lines[tableStartIndex].trim().startsWith('+-')) {
              tableStartIndex++
            }
            
            if (tableStartIndex < lines.length) {
              const tableData = this.parseMarkdownTable(lines, tableStartIndex)
              
              // Vérifier que c'est bien une procédure (a "Procedure" comme première clé)
              if (tableData['Procedure']) {
                results.push({
                  _id: `procedure_${procedureName.replace(/[^a-zA-Z0-9]/g, '_')}_${idCounter++}`,
                  type: 'procedure',
                  title: procedureName,
                  chapter: currentChapter ? currentChapter.number : null,
                  section: currentSection ? currentSection.number : null,
                  timestamp: new Date().toISOString(),
                  tableColumns: ['Procedure', 'Data'],
                  tableData: tableData,
                  references: tableData['References'] || '',
                  testPurpose: tableData['Test purpose'] || '',
                  procedureBody: tableData['Procedure body'] || ''
                })
              }
            }
          }
          
          lineIndex++
          continue
        }

        lineIndex++
      }

      console.log(`Parsed ${results.length} elements from Markdown`)
      return results
    } catch (error) {
      console.error('Error parsing Markdown file:', error)
      throw new Error('Erreur lors du parsing Markdown')
    }
  }

  /**
   * Parser un tableau Markdown Grid Table
   */
  private parseMarkdownTable(lines: string[], startIndex: number): any {
    const tableData: Record<string, string> = {}
    let currentKey = ''
    let currentValue = ''
    let i = startIndex
    let inTable = false

    while (i < lines.length) {
      const line = lines[i]

      // Ligne de séparation (début ou fin de tableau)
      if (line.trim().startsWith('+-') || line.trim().startsWith('+=')) {
        if (inTable && currentKey) {
          // Sauvegarder la paire clé-valeur précédente
          tableData[currentKey] = currentValue.trim()
          currentKey = ''
          currentValue = ''
        }
        inTable = true
        i++
        continue
      }

      // Ligne de contenu du tableau
      if (line.trim().startsWith('|') && inTable) {
        const cells = this.parseTableRow(line)
        
        if (cells.length >= 2) {
          const key = cells[0].trim()
          const value = cells[1].trim()

          // Nouvelle clé-valeur
          if (key && !key.startsWith('[') && !key.match(/^-+$/)) {
            // Sauvegarder la paire précédente si elle existe
            if (currentKey) {
              tableData[currentKey] = currentValue.trim()
            }
            currentKey = key
            currentValue = value
          } else if (currentKey) {
            // Continuation de la valeur précédente
            if (currentValue) {
              currentValue += '\n' + value
            } else {
              currentValue = value
            }
          }
        }
        i++
        continue
      }

      // Fin du tableau
      if (inTable && !line.trim().startsWith('|') && !line.trim().startsWith('+')) {
        if (currentKey) {
          tableData[currentKey] = currentValue.trim()
        }
        break
      }

      i++
    }

    // Sauvegarder la dernière paire si nécessaire
    if (currentKey && !tableData[currentKey]) {
      tableData[currentKey] = currentValue.trim()
    }

    // Extraire les champs spécifiques pour les test cases
    const result: any = {
      tableData,
      endIndex: i
    }

    // Si c'est un test case, extraire les champs détaillés
    if (tableData['**Test Case**']) {
      const testCaseHeader = tableData['**Test Case**']
      let testId = ''
      let testTitle = ''
      
      if (testCaseHeader.includes('ACESM-') && testCaseHeader.includes('-TC')) {
        const colonIndex = testCaseHeader.indexOf(':')
        if (colonIndex > 0) {
          testId = testCaseHeader.substring(0, colonIndex).trim()
          testTitle = testCaseHeader.substring(colonIndex + 1).trim()
        } else {
          testId = testCaseHeader
          testTitle = testCaseHeader
        }
      }

      result.testId = testId
      result.title = testTitle
      result.useCase = tableData['Use Case'] || ''
      result.scenario = tableData['Scenario'] || ''
      result.step = tableData['Step'] || ''
      result.references = tableData['References'] || tableData['R eferences'] || ''
      result.testPurpose = tableData['Test purpose'] || ''
      result.testStrategy = tableData['Test strategy'] || ''
      result.aaFilter = tableData['AA filter'] || tableData['AAs used'] || ''
      result.prerequisites = tableData['Prerequisites'] || tableData['Prer equisites'] || ''
      result.expectedResult = tableData['Expected result'] || ''
      result.preamble = tableData['Preamble'] || ''
      result.testBody = tableData['Test body'] || tableData['Test body []{.mark}'] || ''
      result.postamble = tableData['Postamble'] || ''
      result.comment = tableData['Comment'] || ''
      result.tableColumns = ['Field', 'Value']
    }

    return result
  }

  /**
   * Parser une ligne de tableau pour extraire les cellules
   */
  private parseTableRow(line: string): string[] {
    // Retirer les | au début et à la fin
    let content = line.trim()
    if (content.startsWith('|')) content = content.substring(1)
    if (content.endsWith('|')) content = content.substring(0, content.length - 1)
    
    // Séparer par |
    const cells = content.split('|').map(cell => cell.trim())
    return cells
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
    let currentSection: { number: string; title: string } | null = null

    // Extraire les listes ordonnées qui contiennent la structure des chapitres
    const lists = doc.querySelectorAll('ol')
    
    lists.forEach((list) => {
      const items = list.querySelectorAll('li')
      
      items.forEach((item) => {
        // Chercher les titres dans les éléments de liste
        const headers = item.querySelectorAll('h1, h2, h3, h4, h5, h6')
        
        headers.forEach((header) => {
          const text = header.textContent?.trim() || ''
          if (!text) return

          // Extraire le numéro automatique de la liste ordonnée si présent
          const listValue = item.getAttribute('value')
          
          // Détecter les chapitres principaux (h2)
          if (header.tagName === 'H2') {
            const chapterNum = listValue || this.extractNumber(text)
            if (chapterNum) {
              const chapterTitle = this.cleanTitle(text)
              currentChapter = { number: chapterNum, title: chapterTitle }
              currentSection = null
              
              results.push({
                _id: `chapter_${chapterNum}_${idCounter++}`,
                type: 'chapter',
                number: chapterNum,
                title: chapterTitle,
                timestamp: new Date().toISOString()
              })
            }
            return
          }

          // Détecter les sections (h3)
          if (header.tagName === 'H3') {
            const sectionNum = listValue || this.extractNumber(text)
            if (sectionNum) {
              const sectionTitle = this.cleanTitle(text)
              currentSection = { number: sectionNum, title: sectionTitle }
              
              results.push({
                _id: `section_${sectionNum.replace(/\./g, '_')}_${idCounter++}`,
                type: 'section',
                number: sectionNum,
                title: sectionTitle,
                parent: currentChapter ? currentChapter.number : null,
                chapter: currentChapter ? currentChapter.number : null,
                timestamp: new Date().toISOString()
              })
            }
            return
          }

          // Détecter les test cases (h4 avec format ACESM-XX-TCXX)
          if (header.tagName === 'H4') {
            const testMatch = text.match(/(ACESM-[A-Z0-9]+-TC\d+):?\s*(.+?)$/)
            if (testMatch) {
              const testId = testMatch[1]
              const testTitle = testMatch[2].trim()
              
              results.push({
                _id: `test_${testId}_${idCounter++}`,
                type: 'test-case',
                testId: testId,
                title: testTitle,
                chapter: currentChapter ? currentChapter.number : null,
                section: currentSection ? currentSection.number : null,
                timestamp: new Date().toISOString()
              })
            }
          }
        })
      })
    })

    // Extraire également les paragraphes qui ne sont pas dans des listes
    const paragraphs = doc.querySelectorAll('p, h1, h2, h3, h4, h5, h6')
    paragraphs.forEach((para) => {
      const text = para.textContent?.trim() || ''
      if (!text) return

      // Détecter les chapitres principaux (ex: "1 General procedures")
      const chapterMatch = text.match(/^(\d+)[\.\s]+(.+?)$/)
      if (chapterMatch && chapterMatch[1].length <= 2 && para.tagName === 'H2') {
        const chapterNum = chapterMatch[1]
        const chapterTitle = chapterMatch[2].trim()
        
        // Vérifier que ce chapitre n'existe pas déjà
        const exists = results.some(r => r.type === 'chapter' && r.number === chapterNum)
        if (!exists) {
          currentChapter = { number: chapterNum, title: chapterTitle }
          currentSection = null
          
          results.push({
            _id: `chapter_${chapterNum}_${idCounter++}`,
            type: 'chapter',
            number: chapterNum,
            title: chapterTitle,
            timestamp: new Date().toISOString()
          })
        }
        return
      }

      // Détecter les sections (ex: "2.1 Section title")
      const sectionMatch = text.match(/^(\d+\.\d+(?:\.\d+)?)\s+(.+?)$/)
      if (sectionMatch && (para.tagName === 'H3' || para.tagName === 'H2')) {
        const sectionNum = sectionMatch[1]
        const sectionTitle = sectionMatch[2].trim()
        
        // Vérifier que cette section n'existe pas déjà
        const exists = results.some(r => r.type === 'section' && r.number === sectionNum)
        if (!exists) {
          currentSection = { number: sectionNum, title: sectionTitle }
          
          results.push({
            _id: `section_${sectionNum.replace(/\./g, '_')}_${idCounter++}`,
            type: 'section',
            number: sectionNum,
            title: sectionTitle,
            parent: currentChapter ? currentChapter.number : null,
            chapter: currentChapter ? currentChapter.number : null,
            timestamp: new Date().toISOString()
          })
        }
      }
    })

    // Extraire les tableaux de procédures et test cases
    const tables = doc.querySelectorAll('table')
    tables.forEach((table) => {
      const rows = table.querySelectorAll('tr')
      if (rows.length < 2) return // Pas assez de lignes

      // Extraire toutes les cellules du tableau
      const allRows: string[][] = []
      rows.forEach(row => {
        const cells = row.querySelectorAll('th, td')
        const rowData = Array.from(cells).map(cell => cell.textContent?.trim() || '')
        allRows.push(rowData)
      })

      if (allRows.length === 0 || allRows[0].length !== 2) return

      const firstCell = allRows[0][0]
      
      // === PROCÉDURE : structure fixe à 4 lignes minimum ===
      if (firstCell === 'Procedure' && allRows.length >= 4) {
        const procedureName = allRows[0][1] || ''
        const references = allRows[1][1] || ''
        const testPurpose = allRows[2][1] || ''
        const procedureBody = allRows[3][1] || ''
        
        // Extraire le numéro de procédure si présent dans le nom
        const procNumMatch = procedureName.match(/^(\d+(?:\.\d+)*)\s/)
        const procNum = procNumMatch ? procNumMatch[1] : null
        
        results.push({
          _id: `procedure_${procedureName.replace(/[^a-zA-Z0-9]/g, '_')}_${idCounter++}`,
          type: 'procedure',
          number: procNum,
          title: procedureName,
          references: references,
          testPurpose: testPurpose,
          procedureBody: procedureBody,
          chapter: currentChapter ? currentChapter.number : null,
          section: currentSection ? currentSection.number : null,
          timestamp: new Date().toISOString(),
          tableColumns: ['Procedure', 'Data'],
          tableData: {
            'Procedure': procedureName,
            'References': references,
            'Test purpose': testPurpose,
            'Procedure body': procedureBody
          }
        })
        return
      }
      
      // === TEST CASE : structure variable ===
      if (firstCell === 'Test Case' && allRows.length >= 2) {
        const testCaseHeader = allRows[0][1]
        
        // Extraire le test ID et le titre
        let testId = ''
        let testTitle = ''
        
        if (testCaseHeader.includes('ACESM-') && testCaseHeader.includes('-TC')) {
          const colonIndex = testCaseHeader.indexOf(':')
          if (colonIndex > 0) {
            testId = testCaseHeader.substring(0, colonIndex).trim()
            testTitle = testCaseHeader.substring(colonIndex + 1).trim()
          } else {
            testId = testCaseHeader
            testTitle = testCaseHeader
          }
        } else {
          // Template ou autre
          testTitle = testCaseHeader
        }

        // Extraire toutes les données du tableau
        const tableData: Record<string, string> = {}
        const testBodySteps: string[] = []
        let inTestBody = false
        
        for (let i = 0; i < allRows.length; i++) {
          const key = allRows[i][0]
          const value = allRows[i][1] || ''
          
          if (!key && inTestBody && value) {
            // Ligne supplémentaire du Test body (pas de clé, juste du contenu)
            testBodySteps.push(value)
          } else if (key) {
            tableData[key] = value
            
            // Détecter le début du Test body
            if (key === 'Test body') {
              inTestBody = true
              if (value) {
                testBodySteps.push(value)
              }
            } else {
              inTestBody = false
            }
          }
        }

        // Vérifier si le test case n'existe pas déjà (éviter les doublons)
        const exists = results.some(r => r.type === 'test-case' && r.testId === testId)
        if (!exists) {
          // Créer l'entrée de test case
          results.push({
            _id: `test_${testId || 'unknown'}_${idCounter++}`,
            type: 'test-case',
            testId: testId,
            title: testTitle,
            chapter: currentChapter ? currentChapter.number : null,
            section: currentSection ? currentSection.number : null,
            timestamp: new Date().toISOString(),
            tableColumns: ['Field', 'Value'],
            tableData: tableData,
            // Champs détaillés
            useCase: tableData['Use Case'] || '',
            scenario: tableData['Scenario'] || '',
            step: tableData['Step'] || '',
            references: tableData['References'] || '',
            testPurpose: tableData['Test purpose'] || '',
            testStrategy: tableData['Test strategy'] || '',
            aaFilter: tableData['AA filter'] || tableData['AAs used'] || '',
            prerequisites: tableData['Prerequisites'] || '',
            expectedResult: tableData['Expected result'] || '',
            preamble: tableData['Preamble'] || '',
            testBody: testBodySteps.join('\n'),
            testBodySteps: testBodySteps,
            postamble: tableData['Postamble'] || '',
            comment: tableData['Comment'] || ''
          })
        }
      }
    })

    console.log(`Parsed ${results.length} elements from HTML`)
    return results
  }

  /**
   * Extraire le numéro d'un titre
   */
  private extractNumber(text: string): string | null {
    const match = text.match(/^(\d+(?:\.\d+)*)\s/)
    return match ? match[1] : null
  }

  /**
   * Nettoyer le titre en retirant le numéro
   */
  private cleanTitle(text: string): string {
    return text.replace(/^\d+(?:\.\d+)*\s+/, '').trim()
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

  /**
   * Parser un fichier Markdown contenant uniquement des procédures
   */
  private parseMarkdownProcedures(markdownContent: string): AtpTest[] {
    try {
      const lines = markdownContent.split('\n')
      const results: AtpTest[] = []
      let idCounter = 1
      let currentChapter: { number: string; title: string } | null = null
      let currentSection: { number: string; title: string } | null = null
      let lineIndex = 0

      while (lineIndex < lines.length) {
        const line = lines[lineIndex].trim()

        // Détecter les chapitres (##)
        if (line.startsWith('## ') && !line.startsWith('### ')) {
          const chapterTitle = line.substring(3).trim()
          const chapterNum = String(results.filter(r => r.type === 'chapter').length + 1)

          // Créer un ID unique basé sur le titre pour éviter les doublons
          const chapterId = `PROC_${chapterTitle.replace(/[^a-zA-Z0-9]/g, '_').substring(0, 50)}_${idCounter++}`

          currentChapter = { number: `PROC-${chapterNum}`, title: chapterTitle }
          currentSection = null

          results.push({
            _id: chapterId,
            type: 'chapter',
            number: `PROC-${chapterNum}`,
            title: chapterTitle,
            timestamp: new Date().toISOString()
          })

          lineIndex++
          continue
        }

        // Détecter les sections (###)
        if (line.startsWith('### ')) {
          const sectionTitle = line.substring(4).trim()
          const sectionNum = currentChapter ? `${currentChapter.number}.${results.filter(r => r.type === 'section' && r.chapter === currentChapter?.number).length + 1}` : String(idCounter)

          currentSection = { number: sectionNum, title: sectionTitle }

          // ID unique basé sur le titre de la section
          const sectionId = `PROC_SEC_${sectionTitle.replace(/[^a-zA-Z0-9]/g, '_').substring(0, 50)}_${idCounter++}`

          results.push({
            _id: sectionId,
            type: 'section',
            number: sectionNum,
            title: sectionTitle,
            parent: currentChapter ? currentChapter.number : null,
            chapter: currentChapter ? currentChapter.number : null,
            timestamp: new Date().toISOString()
          })

          lineIndex++
          continue
        }

        // Détecter les procédures (Table X -- Nom)
        if (line.match(/^Table\s+\d+\s+--\s+.+/)) {
          const tableNameMatch = line.match(/^Table\s+(\d+)\s+--\s+(.+)/)
          if (tableNameMatch) {
            const tableNumber = tableNameMatch[1]
            const procedureName = tableNameMatch[2].trim()
            
            // Parser le tableau de procédure
            const tableData = this.parseProcedureTable(lines, lineIndex + 1)
            
            if (tableData) {
              results.push({
                _id: `procedure_${tableNumber}_${idCounter++}`,
                type: 'procedure',
                number: tableNumber,
                title: procedureName,
                chapter: currentChapter ? currentChapter.number : null,
                section: currentSection ? currentSection.number : null,
                timestamp: new Date().toISOString(),
                tableColumns: ['Field', 'Value'],
                tableData: tableData,
                references: tableData['References'] || '',
                testPurpose: tableData['Test purpose'] || '',
                procedureBody: tableData['Procedure body'] || ''
              })
            }
          }
        }

        lineIndex++
      }

      console.log(`Parsed ${results.length} elements from Procedures Markdown`)
      return results
    } catch (error) {
      console.error('Error parsing Procedures Markdown:', error)
      throw new Error('Erreur lors du parsing des procédures Markdown')
    }
  }

  /**
   * Parser un fichier Markdown contenant uniquement des test cases
   */
  private parseMarkdownTestCases(markdownContent: string): AtpTest[] {
    try {
      const lines = markdownContent.split('\n')
      const results: AtpTest[] = []
      let idCounter = 1
      let currentChapter: { number: string; title: string } | null = null
      let currentSection: { number: string; title: string } | null = null
      let lineIndex = 0

      while (lineIndex < lines.length) {
        const line = lines[lineIndex].trim()

        // Détecter les chapitres (##)
        if (line.startsWith('## ') && !line.startsWith('### ')) {
          const chapterTitle = line.substring(3).trim()
          const chapterNum = String(results.filter(r => r.type === 'chapter').length + 1)

          // Créer un ID unique basé sur le titre pour éviter les doublons
          const chapterId = `TC_${chapterTitle.replace(/[^a-zA-Z0-9]/g, '_').substring(0, 50)}_${idCounter++}`

          currentChapter = { number: `TC-${chapterNum}`, title: chapterTitle }
          currentSection = null

          results.push({
            _id: chapterId,
            type: 'chapter',
            number: `TC-${chapterNum}`,
            title: chapterTitle,
            timestamp: new Date().toISOString()
          })

          lineIndex++
          continue
        }

        // Détecter les sections (### sans ACESM)
        if (line.startsWith('### ') && !line.includes('ACESM-')) {
          const sectionTitle = line.substring(4).trim()
          const sectionNum = currentChapter ? `${currentChapter.number}.${results.filter(r => r.type === 'section' && r.chapter === currentChapter?.number).length + 1}` : String(idCounter)

          currentSection = { number: sectionNum, title: sectionTitle }

          // ID unique basé sur le titre de la section
          const sectionId = `TC_SEC_${sectionTitle.replace(/[^a-zA-Z0-9]/g, '_').substring(0, 50)}_${idCounter++}`

          results.push({
            _id: sectionId,
            type: 'section',
            number: sectionNum,
            title: sectionTitle,
            parent: currentChapter ? currentChapter.number : null,
            chapter: currentChapter ? currentChapter.number : null,
            timestamp: new Date().toISOString()
          })

          lineIndex++
          continue
        }

        // Détecter les test cases (### ou #### avec ACESM-)
        if ((line.startsWith('### ') || line.startsWith('#### ')) && line.includes('ACESM-')) {
          const testMatch = line.match(/###[#]?\s+(ACESM-[A-Z0-9]+-TC\d+)\s*[,:]\s*(.+)/)
          if (testMatch) {
            const testId = testMatch[1]
            const testTitle = testMatch[2].trim()
            
            // Parser le tableau de test case
            const tableData = this.parseTestCaseTable(lines, lineIndex + 1)
            
            if (tableData) {
              results.push({
                _id: `test_${testId}_${idCounter++}`,
                type: 'test-case',
                testId: testId,
                title: testTitle,
                chapter: currentChapter ? currentChapter.number : null,
                section: currentSection ? currentSection.number : null,
                timestamp: new Date().toISOString(),
                tableColumns: ['Field', 'Value'],
                tableData: tableData,
                references: tableData['References'] || '',
                testPurpose: tableData['Test purpose'] || '',
                testStrategy: tableData['Test strategy'] || '',
                aaFilter: tableData['AA filter'] || tableData['AAs used'] || '',
                prerequisites: tableData['Prerequisites'] || '',
                expectedResult: tableData['Expected result'] || '',
                preamble: tableData['Preamble'] || '',
                testBody: tableData['Test body'] || '',
                testBodySteps: tableData['Test body'] ? [tableData['Test body']] : [],
                postamble: tableData['Postamble'] || '',
                comment: tableData['Comment'] || ''
              })
            }
          }
        }

        lineIndex++
      }

      console.log(`Parsed ${results.length} elements from Test Cases Markdown`)
      return results
    } catch (error) {
      console.error('Error parsing Test Cases Markdown:', error)
      throw new Error('Erreur lors du parsing des test cases Markdown')
    }
  }

  /**
   * Parser un tableau de procédure avec séparateurs +---+ et +===+
   */
  private parseProcedureTable(lines: string[], startIndex: number): Record<string, string> | null {
    const tableData: Record<string, string> = {}
    let currentKey = ''
    let currentValue = ''
    let inTable = false
    let foundProcedureHeader = false
    
    for (let i = startIndex; i < lines.length; i++) {
      const line = lines[i].trim()
      
      // Ligne de séparateur (début ou entre lignes)
      if (line.match(/^\+[-=]+\+/)) {
        // Si on était en train de traiter une cellule, la sauvegarder
        if (currentKey && currentValue) {
          tableData[currentKey] = currentValue.trim()
          currentKey = ''
          currentValue = ''
        }
        inTable = true
        continue
      }
      
      // Fin du tableau (ligne vide après avoir commencé)
      if (inTable && line === '') {
        // Sauvegarder la dernière cellule
        if (currentKey && currentValue) {
          tableData[currentKey] = currentValue.trim()
        }
        break
      }
      
      // Ligne de données
      if (line.startsWith('|') && line.endsWith('|')) {
        const cells = line.split('|').slice(1, -1).map(c => c.trim())
        
        if (cells.length >= 2) {
          let key = cells[0].replace(/\*\*/g, '').trim()
          let value = cells[1].replace(/\*\*/g, '').trim()
          
          // Détecter l'en-tête "Procedure" (peut être coupé)
          if (key.toLowerCase().includes('procedure') || 
              key.toLowerCase().includes('cedure') ||
              (currentKey && currentKey.toLowerCase().includes('pro') && key.toLowerCase().includes('cedure'))) {
            if (currentKey && currentKey.toLowerCase().includes('pro')) {
              // Continuation de "Pro" + "cedure"
              currentKey = 'Procedure'
              currentValue = value || tableData[currentKey] || ''
            }
            foundProcedureHeader = true
            tableData['Procedure'] = value || currentValue
            currentKey = ''
            currentValue = ''
            continue
          }
          
          // Si c'est une nouvelle clé (pas vide)
          if (key !== '') {
            // Vérifier si c'est une continuation de clé
            const isKeyContinuation = 
              key.length < 20 &&
              currentKey !== '' &&
              (value === '' || value.length < 10 || value.match(/^\[.*\]$/))
            
            if (isKeyContinuation) {
              // Combiner avec la clé en cours
              const knownAcronyms = ['AA', 'AAs', 'CTI']
              const incompletePrefixes = ['Pro', 'Re', 'Prer', 'P', 'R']
              
              let needSpace = false
              if (knownAcronyms.includes(currentKey)) {
                needSpace = true
              } else if (incompletePrefixes.includes(currentKey)) {
                needSpace = false
              } else if (currentKey.length >= 4) {
                needSpace = true
              }
              
              currentKey = needSpace ? currentKey + ' ' + key : currentKey + key
              
              if (value && value.length >= 10 && !value.match(/^\[.*\]$/)) {
                currentValue = value
              }
            } else {
              // Normaliser les clés connues
              if (key.toLowerCase().includes('reference')) key = 'References'
              else if (key.toLowerCase().includes('test') && key.toLowerCase().includes('purpose')) key = 'Test purpose'
              else if (key.toLowerCase().includes('purpose')) key = 'Test purpose'
              else if (key.toLowerCase().includes('procedure') && key.toLowerCase().includes('body')) key = 'Procedure body'
              else if (key.toLowerCase().includes('body')) key = 'Procedure body'
              
              // Sauvegarder la cellule précédente
              if (currentKey && currentValue) {
                tableData[currentKey] = currentValue.trim()
              }
              
              currentKey = key
              currentValue = value
            }
          } else {
            // Continuation de la valeur précédente
            if (currentValue && value) {
              currentValue += '\n' + value
            } else if (value) {
              currentValue = value
            }
          }
        }
      }
    }
    
    // Sauvegarder la dernière cellule
    if (currentKey && currentValue) {
      tableData[currentKey] = currentValue.trim()
    }
    
    return foundProcedureHeader ? tableData : null
  }

  /**
   * Parser un tableau de test case avec séparateurs +---+ et +===+
   */
  private parseTestCaseTable(lines: string[], startIndex: number): Record<string, string> | null {
    const tableData: Record<string, string> = {}
    let currentKey = ''
    let currentValue = ''
    let inTable = false
    let foundTestCaseHeader = false
    
    for (let i = startIndex; i < lines.length; i++) {
      const line = lines[i].trim()
      
      // Ligne de séparateur (début ou entre lignes)
      if (line.match(/^\+[-=]+\+/)) {
        // Si on était en train de traiter une cellule, la sauvegarder
        if (currentKey && currentValue) {
          tableData[currentKey] = currentValue.trim()
          currentKey = ''
          currentValue = ''
        }
        inTable = true
        continue
      }
      
      // Fin du tableau (ligne vide après avoir commencé)
      if (inTable && line === '') {
        // Sauvegarder la dernière cellule
        if (currentKey && currentValue) {
          tableData[currentKey] = currentValue.trim()
        }
        break
      }
      
      // Ligne de données
      if (line.startsWith('|') && line.endsWith('|')) {
        const cells = line.split('|').slice(1, -1).map(c => c.trim())
        
        if (cells.length >= 2) {
          let key = cells[0].replace(/\*\*/g, '').trim()
          let value = cells[1].replace(/\*\*/g, '').trim()
          
          // Détecter l'en-tête "Test Case" (peut être coupé)
          if ((key.toLowerCase().includes('test') && key.toLowerCase().includes('case')) ||
              key.toLowerCase() === 'case' ||
              (currentKey && currentKey.toLowerCase().includes('test') && key.toLowerCase() === 'case')) {
            if (currentKey && currentKey.toLowerCase().includes('test') && key.toLowerCase() === 'case') {
              // Continuation de "Test" + "Case"
              currentKey = 'Test Case'
              currentValue = value || tableData[currentKey] || ''
            }
            foundTestCaseHeader = true
            tableData['Test Case'] = value || currentValue
            currentKey = ''
            currentValue = ''
            continue
          }
          
          // Si c'est une nouvelle clé (pas vide)
          if (key !== '') {
            // Vérifier si c'est une continuation de clé
            const isKeyContinuation = 
              key.length < 20 &&
              currentKey !== '' &&
              (value === '' || value.length < 10 || value.match(/^\[.*\]$/))
            
            if (isKeyContinuation) {
              // Combiner avec la clé en cours
              const knownAcronyms = ['AA', 'AAs', 'CTI']
              const incompletePrefixes = ['Pro', 'Re', 'Prer', 'P', 'R', 'Conf']
              
              let needSpace = false
              if (knownAcronyms.includes(currentKey)) {
                needSpace = true
              } else if (incompletePrefixes.includes(currentKey)) {
                needSpace = false
              } else if (currentKey.length >= 4) {
                needSpace = true
              }
              
              currentKey = needSpace ? currentKey + ' ' + key : currentKey + key
              
              if (value && value.length >= 10 && !value.match(/^\[.*\]$/)) {
                currentValue = value
              }
            } else {
              // Normaliser les clés connues
              if (key.toLowerCase().includes('reference')) key = 'References'
              else if (key.toLowerCase().includes('test') && key.toLowerCase().includes('purpose')) key = 'Test purpose'
              else if (key.toLowerCase().includes('purpose')) key = 'Test purpose'
              else if (key.toLowerCase().includes('test') && key.toLowerCase().includes('strategy')) key = 'Test strategy'
              else if (key.toLowerCase().includes('strategy')) key = 'Test strategy'
              else if (key.toLowerCase().includes('aa') && key.toLowerCase().includes('filter')) key = 'AA filter'
              else if (key.toLowerCase().includes('aa') && key.toLowerCase().includes('used')) key = 'AAs used'
              else if (key.toLowerCase().includes('aa')) key = 'AAs used'
              else if (key.toLowerCase().includes('prerequisite')) key = 'Prerequisites'
              else if (key.toLowerCase().includes('equisite')) key = 'Prerequisites'
              else if (key.toLowerCase().includes('expected')) key = 'Expected result'
              else if (key.toLowerCase().includes('preamble')) key = 'Preamble'
              else if (key.toLowerCase().includes('test') && key.toLowerCase().includes('body')) key = 'Test body'
              else if (key.toLowerCase().includes('body')) key = 'Test body'
              else if (key.toLowerCase().includes('postamble')) key = 'Postamble'
              else if (key.toLowerCase().includes('comment')) key = 'Comment'
              
              // Sauvegarder la cellule précédente
              if (currentKey && currentValue) {
                tableData[currentKey] = currentValue.trim()
              }
              
              currentKey = key
              currentValue = value
            }
          } else {
            // Continuation de la valeur précédente
            if (currentValue && value) {
              currentValue += '\n' + value
            } else if (value) {
              currentValue = value
            }
          }
        }
      }
    }
    
    // Sauvegarder la dernière cellule
    if (currentKey && currentValue) {
      tableData[currentKey] = currentValue.trim()
    }
    
    return foundTestCaseHeader ? tableData : null
  }
}

export const atpParserService = new AtpParserService()
