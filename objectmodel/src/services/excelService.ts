import * as XLSX from 'xlsx'
import type { ExcelData, ExcelRow, DlmsObject, DlmsAttribute } from '@/types'

class ExcelService {
  async readExcelFile(file: File): Promise<ExcelData> {
    return new Promise((resolve, reject) => {
      const reader = new FileReader()
      reader.onload = (e: ProgressEvent<FileReader>) => {
        try {
          if (!e.target?.result) {
            throw new Error('Impossible de lire le fichier')
          }

          const data = new Uint8Array(e.target.result as ArrayBuffer)
          const workbook = XLSX.read(data, { type: 'array' })
          
          const sheetName = "Object model"
          if (!workbook.Sheets[sheetName]) {
            throw new Error(`Feuille "${sheetName}" non trouvée dans le fichier Excel`)
          }

          const worksheet = workbook.Sheets[sheetName]
          const jsonData: ExcelData = XLSX.utils.sheet_to_json(worksheet, { header: 1 })
          resolve(jsonData)
        } catch (error) {
          reject(error)
        }
      }
      reader.onerror = () => reject(new Error('Erreur lors de la lecture du fichier'))
      reader.readAsArrayBuffer(file)
    })
  }

  parseObjectModel(data: ExcelData): DlmsObject[] {
    const objects: DlmsObject[] = []
    let currentObject: DlmsObject | null = null
    let objectCounter = 0

    for (let i = 2; i < data.length; i++) {
      const row = data[i]
      if (!row || row.length === 0) continue

      if (this.isObjectRow(row)) {
        if (currentObject) {
          objects.push(currentObject)
        }

        currentObject = {
          id: ++objectCounter,
          name: String(row[1] || 'Objet sans nom'),
          classId: parseInt(String(row[3] || '')),
          version: parseInt(String(row[4] || '')),
          obisCode: String(row[6] || ''),
          attributes: [],
          expanded: false
        }
      } else if (this.isAttributeRow(row) && currentObject) {
        const attribute: DlmsAttribute = {
          number: Number(row[0]),
          name: String(row[1] || 'Attribut sans nom'),
          type: String(row[2] || ''),
          defaultValue: row[6] ? String(row[6]) : null,
          accessRights: this.formatAccessRights(row[8], row[9], row[10])
        }
        currentObject.attributes.push(attribute)
      }
    }

    if (currentObject) {
      objects.push(currentObject)
    }

    return objects.filter(obj => obj.name && obj.name.trim())
  }

  private isObjectRow(row: ExcelRow): boolean {
    const obisCode = row[6]
    const classId = row[3]
    const hasNoNumber = !row[0] || isNaN(Number(row[0]))

    return hasNoNumber &&
           obisCode &&
           typeof obisCode === 'string' &&
           obisCode.includes('-') &&
           classId &&
           !isNaN(Number(classId))
  }

  private isAttributeRow(row: ExcelRow): boolean {
    return row[0] &&
           !isNaN(Number(row[0])) &&
           row[1] &&
           typeof row[1] === 'string'
  }

  private formatAccessRights(col8: any, col9: any, col10: any): string | null {
    const rights: string[] = []
    if (col8 && col8 !== '--') rights.push(`Public: ${col8}`)
    if (col9 && col9 !== '--') rights.push(`Pre: ${col9}`)
    if (col10 && col10 !== '--') rights.push(`Mgmt: ${col10}`)
    return rights.length > 0 ? rights.join(', ') : null
  }
}

export const excelService = new ExcelService()