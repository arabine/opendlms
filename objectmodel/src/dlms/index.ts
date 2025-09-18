/**
 * DLMS/COSEM TypeScript Generator - Point d'entrée principal
 * 
 * @fileoverview Export centralisé de toutes les fonctionnalités DLMS/COSEM
 * @version 1.0.0
 * @author DLMS COSEM Generator
 * 
 * @example
 * ```typescript
 * import { CosemClassFactory, DlmsDataType } from './dlms'
 * 
 * // Génération simple
 * const xml = CosemClassFactory.generateSetRequest(1, '0-0:96.1.0.255', 2, 'DEVICE_123')
 * 
 * // Configuration complexe avec builder
 * const builder = new CosemConfigurationBuilder()
 * const xmls = builder
 *   .addAttribute(1, '0-0:96.1.0.255', 2, 'DEVICE_ID')
 *   .addAttribute(3, '1-0:1.8.0.255', 2, 123456)
 *   .build()
 * ```
 */

// ===============================================
// EXPORTS PRINCIPAUX
// ===============================================

// Types communs
export * from './types/common'

// Utilitaires XML
export { default as DlmsXmlGenerator } from './utils/xmlGenerator'

// Interface de base
export { type ICosemClassGenerator, BaseCosemClassGenerator } from './interfaces/ICosemClass'
export type { AttributeConfig, MethodConfig, CosemClassConfig } from './interfaces/ICosemClass'

// Classes COSEM implémentées
export { default as DataClassGenerator } from './classes/Class01_Data'
export { default as RegisterClassGenerator, DlmsUnit, type ScalerUnit } from './classes/Class03_Register'
export { 
  default as PushSetupGenerator,
  type PushObjectDefinition,
  RestrictionType,
  TransportServiceType,
  MessageType,
  type SendDestinationAndMethod,
  type CommunicationWindow,
  type RepetitionDelay
} from './classes/Class40_PushSetup'

// Factory principale
export { default as CosemClassFactory, CosemConfigurationBuilder } from './factory/CosemClassFactory'

// ===============================================
// API SIMPLIFIÉE POUR UTILISATION RAPIDE
// ===============================================

import CosemClassFactory from './factory/CosemClassFactory'
import type { ClassId, ObisCode, AttributeIndex } from './types/common'

/**
 * API simplifiée pour génération rapide de SetRequest
 * @param classId Identifiant de classe COSEM
 * @param obisCode Code OBIS (format lisible ou hex)
 * @param attributeIndex Index de l'attribut
 * @param value Valeur à configurer
 * @returns XML SetRequest prêt à utiliser
 */
export const generateSetRequest = (
  classId: ClassId,
  obisCode: ObisCode,
  attributeIndex: AttributeIndex,
  value: any
): string => {
  return CosemClassFactory.generateSetRequest(classId, obisCode, attributeIndex, value)
}

/**
 * API simplifiée pour génération rapide de GetRequest
 * @param classId Identifiant de classe COSEM
 * @param obisCode Code OBIS (format lisible ou hex)
 * @param attributeIndex Index de l'attribut
 * @returns XML GetRequest prêt à utiliser
 */
export const generateGetRequest = (
  classId: ClassId,
  obisCode: ObisCode,
  attributeIndex: AttributeIndex
): string => {
  return CosemClassFactory.generateGetRequest(classId, obisCode, attributeIndex)
}

/**
 * Obtient la liste des classes COSEM supportées
 * @returns Array des class_id supportés
 */
export const getSupportedClasses = (): ClassId[] => {
  return CosemClassFactory.getSupportedClassIds()
}

/**
 * Vérifie si une classe est supportée
 * @param classId Identifiant à vérifier
 * @returns true si supporté
 */
export const isClassSupported = (classId: ClassId): boolean => {
  return CosemClassFactory.isClassSupported(classId)
}

// ===============================================
// CONFIGURATIONS PRÉDÉFINIES WSM GCP
// ===============================================

/**
 * Configurations prédéfinies pour WSM GCP (Water Smart Meter Profile)
 */
export const WsmGcpConfigurations = {
  
  /**
   * Configuration Device ID principal
   */
  createDeviceId: (serialNumber: string): string => {
    return generateSetRequest(1, '0-0:96.1.0.255', 2, serialNumber)
  },

  /**
   * Configuration totalisateur eau
   */
  createWaterTotal: (volume: number, scaler: number = -3): string => {
    const registerXml = generateSetRequest(3, '1-0:1.8.0.255', 2, volume)
    const scalerXml = generateSetRequest(3, '1-0:1.8.0.255', 3, { scaler, unit: 14 }) // m³
    return `${registerXml}\n\n${scalerXml}`
  },

  /**
   * Configuration horloge avec fuseau horaire
   */
  createClockSetup: (dateTime: Date, timezone: number = 0): string => {
    // Note: Nécessite implémentation Class08_Clock
    // return generateSetRequest(8, '0-0:1.0.0.255', 2, dateTime)
    throw new Error('Clock class (8) pas encore implémentée')
  },

  /**
   * Configuration Push "On Connectivity" complète
   */
  createConnectivityPush: (destination: string): string => {
    const pushObjects = [
      {
        class_id: 8,
        logical_name: "0000010000FF", // Clock
        attribute_index: 2,
        data_index: 0,
        restriction: { restriction_type: 0 },
        columns: []
      },
      {
        class_id: 1,
        logical_name: "0000600100FF", // Device ID
        attribute_index: 2,
        data_index: 0,
        restriction: { restriction_type: 0 },
        columns: []
      }
    ]

    const objectListXml = generateSetRequest(40, '0-0:25.9.0.255', 2, pushObjects)
    const destinationXml = generateSetRequest(40, '0-0:25.9.0.255', 3, {
      transport_service: 0, // TCP
      destination: destination,
      message: 0 // A-XDR
    })

    return `${objectListXml}\n\n${destinationXml}`
  },

  /**
   * Configuration complète d'un compteur d'eau WSM GCP
   */
  createCompleteWaterMeter: (config: {
    deviceId: string
    currentVolume: number
    serverDestination: string
    timezone?: number
  }): string[] => {
    const configurations = [
      WsmGcpConfigurations.createDeviceId(config.deviceId),
      WsmGcpConfigurations.createWaterTotal(config.currentVolume),
      WsmGcpConfigurations.createConnectivityPush(config.serverDestination)
    ]

    return configurations
  }
}

// ===============================================
// UTILITAIRES DE DIAGNOSTIC
// ===============================================

/**
 * Utilitaires de diagnostic et debugging
 */
export const DlmsDebugUtils = {
  
  /**
   * Affiche un rapport complet des classes supportées
   */
  printSupportReport: (): void => {
    console.log(CosemClassFactory.generateSupportReport())
  },

  /**
   * Démonstration des capacités
   */
  demonstrate: (): void => {
    CosemClassFactory.demonstrateCapabilities()
  },

  /**
   * Validation d'une configuration
   */
  validateConfig: (
    classId: ClassId,
    obisCode: ObisCode,
    attributeIndex: AttributeIndex,
    value: any
  ): boolean => {
    try {
      return CosemClassFactory.validateConfiguration(classId, obisCode, attributeIndex, value)
    } catch (error) {
      console.error('Validation échouée:', error)
      return false
    }
  },

  /**
   * Test de génération pour toutes les classes
   */
  testAllClasses: (): void => {
    const supported = getSupportedClasses()
    console.log(`\n🧪 Test de ${supported.length} classes supportées:\n`)
    
    supported.forEach(classId => {
      try {
        const info = CosemClassFactory.getClassInfo(classId)
        console.log(`✅ Class ${classId} (${info.name}): ${info.attributes.length} attr, ${info.methods.length} methods`)
      } catch (error) {
        console.log(`❌ Class ${classId}: ${error}`)
      }
    })
  }
}

// ===============================================
// VERSION ET METADATA
// ===============================================

/**
 * Informations sur la version du générateur
 */
export const DLMS_GENERATOR_INFO = {
  version: '1.0.0',
  buildDate: new Date().toISOString(),
  supportedClasses: getSupportedClasses(),
  totalClasses: getSupportedClasses().length,
  baseStandard: 'DLMS UA Blue Book Ed. 16 Part 2',
  profile: 'WSM GCP (Water Smart Meter Profile)',
  
  /**
   * Affiche les informations de version
   */
  printInfo: (): void => {
    console.log(`
╔════════════════════════════════════════════════════════════════╗
║                 DLMS/COSEM TypeScript Generator                ║
╠════════════════════════════════════════════════════════════════╣
║ Version: ${DLMS_GENERATOR_INFO.version.padEnd(53)} ║
║ Build: ${DLMS_GENERATOR_INFO.buildDate.substring(0, 19).padEnd(55)} ║
║ Standard: ${DLMS_GENERATOR_INFO.baseStandard.padEnd(52)} ║
║ Profile: ${DLMS_GENERATOR_INFO.profile.padEnd(53)} ║
║ Classes supportées: ${DLMS_GENERATOR_INFO.totalClasses.toString().padEnd(42)} ║
╚════════════════════════════════════════════════════════════════╝
    `)
  }
}

// ===============================================
// EXPORT PAR DÉFAUT
// ===============================================

/**
 * Export par défaut - Factory principale
 */
export default CosemClassFactory