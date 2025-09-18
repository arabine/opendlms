/**
 * Service DLMS Simplifié
 * 
 * @fileoverview Version simplifiée du service DLMS pour intégration directe
 * sans complexité supplémentaire - juste l'essentiel pour XmlGenerationPanel
 * @version 1.0.0
 */

import { 
  CosemClassFactory, 
  getSupportedClasses, 
  isClassSupported 
} from '@/dlms'

import type { DlmsObject, DlmsAttribute } from '@/types'

// ===============================================
// TYPES SIMPLES
// ===============================================

export interface GenerationResult {
  success: boolean
  xml?: string
  error?: string
  usedFactory: boolean
}

// ===============================================
// SERVICE SIMPLIFIÉ
// ===============================================

class DlmsService {
  private supportedClasses: number[] = []

  constructor() {
    this.loadSupportedClasses()
  }

  private loadSupportedClasses(): void {
    try {
      this.supportedClasses = getSupportedClasses()
    } catch (error) {
      console.warn('[DlmsService] Impossible de charger les classes supportées:', error)
      this.supportedClasses = []
    }
  }

  // ===============================================
  // MÉTHODES PRINCIPALES
  // ===============================================

  /**
   * Vérifie si une classe est supportée par la factory
   */
  isClassSupported(classId: number): boolean {
    return isClassSupported(classId)
  }

  /**
   * Obtient les informations d'une classe supportée
   */
  getClassInfo(classId: number) {
    if (!this.isClassSupported(classId)) {
      return null
    }

    try {
      const generator = CosemClassFactory.getGenerator(classId)
      return {
        classId,
        name: generator.className,
        description: generator.description,
        version: generator.version
      }
    } catch (error) {
      return null
    }
  }

  /**
   * Génère un GetRequest - utilise la factory si possible
   */
  generateGetRequest(
    dlmsObject: DlmsObject,
    dlmsAttribute: DlmsAttribute
  ): GenerationResult {
    try {
      if (this.isClassSupported(dlmsObject.classId)) {
        const xml = CosemClassFactory.generateGetRequest(
          dlmsObject.classId,
          dlmsObject.obisCode,
          dlmsAttribute.number
        )

        return {
          success: true,
          xml,
          usedFactory: true
        }
      } else {
        return {
          success: false,
          error: `Classe ${dlmsObject.classId} non supportée par CosemClassFactory`,
          usedFactory: false
        }
      }
    } catch (error) {
      return {
        success: false,
        error: `Erreur génération GET: ${error}`,
        usedFactory: this.isClassSupported(dlmsObject.classId)
      }
    }
  }

  /**
   * Génère un SetRequest - utilise la factory si possible
   */
  generateSetRequest(
    dlmsObject: DlmsObject,
    dlmsAttribute: DlmsAttribute,
    value: any
  ): GenerationResult {
    try {
      if (this.isClassSupported(dlmsObject.classId)) {
        const xml = CosemClassFactory.generateSetRequest(
          dlmsObject.classId,
          dlmsObject.obisCode,
          dlmsAttribute.number,
          value
        )

        return {
          success: true,
          xml,
          usedFactory: true
        }
      } else {
        return {
          success: false,
          error: `Classe ${dlmsObject.classId} non supportée par CosemClassFactory`,
          usedFactory: false
        }
      }
    } catch (error) {
      return {
        success: false,
        error: `Erreur génération SET: ${error}`,
        usedFactory: this.isClassSupported(dlmsObject.classId)
      }
    }
  }

  /**
   * Valide une configuration
   */
  validateConfiguration(
    dlmsObject: DlmsObject,
    dlmsAttribute: DlmsAttribute,
    value: any
  ): { valid: boolean; error?: string } {
    try {
      if (!this.isClassSupported(dlmsObject.classId)) {
        return {
          valid: false,
          error: `Classe ${dlmsObject.classId} non supportée`
        }
      }

      const isValid = CosemClassFactory.validateConfiguration(
        dlmsObject.classId,
        dlmsObject.obisCode,
        dlmsAttribute.number,
        value
      )

      return { valid: isValid }

    } catch (error) {
      return {
        valid: false,
        error: `Erreur validation: ${error}`
      }
    }
  }

  // ===============================================
  // MÉTHODES D'INFORMATION
  // ===============================================

  /**
   * Obtient la liste des classes supportées
   */
  getSupportedClasses(): number[] {
    return [...this.supportedClasses]
  }

  /**
   * Obtient les informations de toutes les classes supportées
   */
  getAllClassesInfo() {
    return this.supportedClasses
      .map(classId => this.getClassInfo(classId))
      .filter(info => info !== null)
  }

  /**
   * Génère des exemples de valeurs pour un attribut
   */
  getExampleValues(dlmsObject: DlmsObject, dlmsAttribute: DlmsAttribute) {
    const examples = []
    const classId = dlmsObject.classId
    const attributeIndex = dlmsAttribute.number
    const type = dlmsAttribute.type?.toLowerCase() || ''

    // Exemples génériques
    if (type.includes('unsigned') || type.includes('integer')) {
      examples.push(
        { label: '0', value: 0 },
        { label: '123456', value: 123456 }
      )
    } else if (type.includes('string')) {
      examples.push(
        { label: 'DEVICE_ID', value: 'DEVICE_ID' },
        { label: 'TEST_VALUE', value: 'TEST_VALUE' }
      )
    } else if (type.includes('boolean')) {
      examples.push(
        { label: 'true', value: true },
        { label: 'false', value: false }
      )
    }

    // Exemples spécifiques par classe
    if (classId === 1 && attributeIndex === 2) {
      examples.push({ label: 'WSM_DEVICE', value: 'WSM_DEVICE_001' })
    } else if (classId === 3 && attributeIndex === 2) {
      examples.push(
        { label: 'Volume', value: 12345 },
        { label: 'Débit', value: 150 }
      )
    } else if (classId === 40 && attributeIndex === 2) {
      examples.push({
        label: 'Push Objects',
        value: [{
          class_id: 8,
          logical_name: "0000010000FF",
          attribute_index: 2,
          data_index: 0
        }]
      })
    }

    return examples
  }

  /**
   * Détecte si un type est complexe (nécessite JSON)
   */
  isComplexType(dlmsAttribute: DlmsAttribute): boolean {
    const type = dlmsAttribute.type?.toLowerCase() || ''
    return type.includes('structure') || 
           type.includes('array') || 
           type.includes('push_object') ||
           type.includes('capture_object')
  }

  /**
   * Obtient un placeholder pour un champ de saisie
   */
  getValuePlaceholder(dlmsAttribute: DlmsAttribute): string {
    const type = dlmsAttribute.type?.toLowerCase() || ''
    
    if (type.includes('unsigned') || type.includes('integer')) {
      return 'Ex: 123456'
    } else if (type.includes('string')) {
      return 'Ex: DEVICE_ID'
    } else if (type.includes('boolean')) {
      return 'true ou false'
    } else if (type.includes('octet_string')) {
      return 'Ex: 0123456789ABCDEF'
    } else if (type.includes('structure') || type.includes('array')) {
      return '{"example": "json_config"}'
    }
    
    return 'Entrez une valeur...'
  }

  // ===============================================
  // MÉTHODES DE DEBUG
  // ===============================================

  /**
   * Obtient le statut du service
   */
  getStatus() {
    return {
      initialized: this.supportedClasses.length > 0,
      supportedClassesCount: this.supportedClasses.length,
      supportedClasses: this.supportedClasses
    }
  }

  /**
   * Affiche les capacités (pour debug)
   */
  demonstrateCapabilities(): void {
    console.group('🔧 DlmsService - Capacités')
    console.log('Classes supportées:', this.supportedClasses)
    console.log('Nombre total:', this.supportedClasses.length)
    
    // Exemple de génération pour chaque classe
    this.supportedClasses.forEach(classId => {
      try {
        const info = this.getClassInfo(classId)
        console.log(`Classe ${classId}: ${info?.name || 'N/A'}`)
      } catch (error) {
        console.log(`Classe ${classId}: Erreur - ${error}`)
      }
    })
    
    console.groupEnd()
  }
}

// ===============================================
// INSTANCE SINGLETON
// ===============================================

export const dlmsService = new DlmsService()

// ===============================================
// EXPORTS
// ===============================================

export default DlmsService
