/**
 * DLMS COSEM Class 22 - Single action schedule
 *
 * @fileoverview Générateur pour la classe Single action schedule (class_id = 22, version = 0)
 * @version 1.0.0
 * @author DLMS COSEM Generator
 *
 * Description: Cette IC permet de modéliser l'exécution d'actions périodiques dans un compteur.
 * Ces actions ne sont pas nécessairement liées à la tarification.
 * Blue Book Ed. 16 Part 2, Section 4.5.7
 */

import {
  ClassId,
  Version,
  ObisCode,
  AttributeIndex,
  DlmsValue,
  DlmsDataType,
  AttributeInfo,
  MethodInfo,
  AccessMode
} from '../types/common'
import { BaseCosemClassGenerator } from '../interfaces/ICosemClass'
import DlmsXmlGenerator from '../utils/xmlGenerator'

// ===============================================
// TYPES SPÉCIFIQUES CLASSE 22
// ===============================================

/** Script à exécuter */
export interface Script {
  /** Logical name de la Script table */
  script_logical_name: string
  /** Sélecteur de script */
  script_selector: number
}

/** Date et heure d'exécution */
export interface ExecutionTimeDate {
  /** Heure au format octet-string */
  time: string
  /** Date au format octet-string */
  date: string
}

/** Type d'exécution */
export enum ScheduleType {
  /** size = 1; wildcard in date allowed */
  SIZE_1_WILDCARD_ALLOWED = 1,
  /** size = n; all time values are the same, wildcards in date not allowed */
  SIZE_N_SAME_TIME_NO_WILDCARD = 2,
  /** size = n; all time values are the same, wildcards in date are allowed */
  SIZE_N_SAME_TIME_WILDCARD_ALLOWED = 3,
  /** size = n; time values may be different, wildcards in date not allowed */
  SIZE_N_DIFFERENT_TIME_NO_WILDCARD = 4,
  /** size = n; time values may be different, wildcards in date are allowed */
  SIZE_N_DIFFERENT_TIME_WILDCARD_ALLOWED = 5
}

// ===============================================
// CLASS 22 - SINGLE ACTION SCHEDULE
// ===============================================

export class SingleActionScheduleClassGenerator extends BaseCosemClassGenerator {
  readonly classId: ClassId = 22
  readonly version: Version = 0
  readonly className: string = 'Single action schedule'
  readonly description: string = 'Exécution d\'actions périodiques dans un compteur'

  // ===============================================
  // DÉFINITION DES ATTRIBUTS
  // ===============================================

  protected attributes = new Map<AttributeIndex, AttributeInfo>([
    [1, this.createAttributeInfo(
      1, 'logical_name', 'octet-string', AccessMode.READ_ONLY, true, true,
      'Identifie l\'instance de l\'objet Single action schedule'
    )],
    [2, this.createAttributeInfo(
      2, 'executed_script', 'script', AccessMode.READ_AND_WRITE, true, false,
      'Contient le logical name de la Script table et le script selector'
    )],
    [3, this.createAttributeInfo(
      3, 'type', 'enum', AccessMode.READ_AND_WRITE, true, false,
      'Spécifie le type d\'exécution (1-5)'
    )],
    [4, this.createAttributeInfo(
      4, 'execution_time', 'array', AccessMode.READ_AND_WRITE, true, false,
      'Spécifie le temps et la date d\'exécution du script'
    )]
  ])

  // ===============================================
  // DÉFINITION DES MÉTHODES
  // ===============================================

  protected methods = new Map<number, MethodInfo>([
    // La classe Single action schedule n'a pas de méthodes spécifiques
  ])

  // ===============================================
  // GÉNÉRATION XML
  // ===============================================

  generateSetRequest(
    obisCode: ObisCode,
    attributeIndex: AttributeIndex,
    value: any,
    invokeId: number = 1
  ): string {
    this.validateAttribute(attributeIndex, value)

    const descriptor = DlmsXmlGenerator.createDescriptor(this.classId, obisCode, attributeIndex)
    const dlmsValue = this.createAttributeValue(attributeIndex, value)

    return DlmsXmlGenerator.generateSetRequest(descriptor, dlmsValue, invokeId)
  }

  generateGetRequest(
    obisCode: ObisCode,
    attributeIndex: AttributeIndex,
    invokeId: number = 1
  ): string {
    if (!this.attributes.has(attributeIndex)) {
      throw new Error(`Attribut ${attributeIndex} non supporté pour Single action schedule class`)
    }

    const descriptor = DlmsXmlGenerator.createDescriptor(this.classId, obisCode, attributeIndex)
    return DlmsXmlGenerator.generateGetRequest(descriptor, invokeId)
  }

  // ===============================================
  // CRÉATION DE VALEURS TYPÉES
  // ===============================================

  createAttributeValue(attributeIndex: AttributeIndex, value: any): DlmsValue {
    switch (attributeIndex) {
      case 1: // logical_name
        return this.createLogicalNameValue(value)

      case 2: // executed_script
        return this.createExecutedScriptValue(value)

      case 3: // type
        return this.createTypeValue(value)

      case 4: // execution_time
        return this.createExecutionTimeValue(value)

      default:
        throw new Error(`Attribut ${attributeIndex} non supporté pour Single action schedule class`)
    }
  }

  // ===============================================
  // CRÉATEURS D'ATTRIBUTS SPÉCIALISÉS
  // ===============================================

  /**
   * Crée l'attribut logical_name (octet-string de 6 octets)
   */
  private createLogicalNameValue(value: string): DlmsValue {
    const hexValue = DlmsXmlGenerator.obisToHex(value)
    return {
      type: DlmsDataType.OCTET_STRING,
      value: hexValue,
      hexValue
    }
  }

  /**
   * Crée l'attribut executed_script (structure)
   * script ::= structure
   * {
   *   script_logical_name: octet-string,
   *   script_selector: long-unsigned
   * }
   */
  private createExecutedScriptValue(value: Script | any): DlmsValue {
    if (typeof value === 'object' && 'script_logical_name' in value && 'script_selector' in value) {
      const scriptLogicalName = DlmsXmlGenerator.obisToHex(value.script_logical_name)

      return {
        type: DlmsDataType.STRUCTURE,
        value: [
          {
            type: DlmsDataType.OCTET_STRING,
            value: scriptLogicalName,
            hexValue: scriptLogicalName
          },
          {
            type: DlmsDataType.LONG_UNSIGNED,
            value: value.script_selector
          }
        ]
      }
    }

    throw new Error('executed_script doit être un objet avec script_logical_name et script_selector')
  }

  /**
   * Crée l'attribut type (enum)
   * (1) size of execution_time = 1; wildcard in date allowed
   * (2) size of execution_time = n; all time values are the same, wildcards in date not allowed
   * (3) size of execution_time = n; all time values are the same, wildcards in date are allowed
   * (4) size of execution_time = n; time values may be different, wildcards in date not allowed
   * (5) size of execution_time = n; time values may be different, wildcards in date are allowed
   */
  private createTypeValue(value: number | ScheduleType): DlmsValue {
    const typeValue = typeof value === 'number' ? value : value

    if (typeValue < 1 || typeValue > 5) {
      throw new Error('type doit être entre 1 et 5')
    }

    return {
      type: DlmsDataType.ENUM,
      value: typeValue
    }
  }

  /**
   * Crée l'attribut execution_time (array de structures)
   * array execution_time_date
   * execution_time_date ::= structure
   * {
   *   time: octet-string,
   *   date: octet-string
   * }
   */
  private createExecutionTimeValue(value: ExecutionTimeDate[] | any): DlmsValue {
    if (!Array.isArray(value)) {
      throw new Error('execution_time doit être un tableau')
    }

    const executionTimeDates = value.map((item: ExecutionTimeDate) => {
      if (!item.time || !item.date) {
        throw new Error('Chaque élément execution_time doit avoir time et date')
      }

      return {
        type: DlmsDataType.STRUCTURE,
        value: [
          {
            type: DlmsDataType.OCTET_STRING,
            value: item.time,
            hexValue: item.time
          },
          {
            type: DlmsDataType.OCTET_STRING,
            value: item.date,
            hexValue: item.date
          }
        ]
      }
    })

    return {
      type: DlmsDataType.ARRAY,
      value: executionTimeDates
    }
  }

  // ===============================================
  // VALIDATION SPÉCIALISÉE
  // ===============================================

  protected validateAttributeValue(
    attributeIndex: AttributeIndex,
    value: any,
    attrInfo: AttributeInfo
  ): boolean {
    switch (attributeIndex) {
      case 1: // logical_name
        if (typeof value !== 'string') {
          throw new Error('logical_name doit être une chaîne OBIS (ex: "0-0:15.0.1.0")')
        }
        if (!/^\d+-\d+:\d+\.\d+\.\d+\.\d+$/.test(value) && !/^[0-9A-Fa-f]{12}$/.test(value)) {
          throw new Error(`Format OBIS invalide: ${value}`)
        }
        return true

      case 2: // executed_script
        if (typeof value !== 'object' || !value.script_logical_name || !value.script_selector) {
          throw new Error('executed_script doit avoir script_logical_name et script_selector')
        }
        if (typeof value.script_selector !== 'number' || value.script_selector < 0 || value.script_selector > 65535) {
          throw new Error('script_selector doit être un nombre entre 0 et 65535')
        }
        return true

      case 3: // type
        if (typeof value !== 'number' || value < 1 || value > 5) {
          throw new Error('type doit être un nombre entre 1 et 5')
        }
        return true

      case 4: // execution_time
        if (!Array.isArray(value)) {
          throw new Error('execution_time doit être un tableau')
        }
        for (const item of value) {
          if (!item.time || !item.date) {
            throw new Error('Chaque élément execution_time doit avoir time et date')
          }
        }
        return true

      default:
        return super.validateAttributeValue(attributeIndex, value, attrInfo)
    }
  }

  // ===============================================
  // MÉTHODES UTILITAIRES SPÉCIFIQUES
  // ===============================================

  /**
   * Crée un planning d'exécution simple (type 1)
   * Usage: Exécution unique ou récurrente avec wildcards
   */
  static createSimpleSchedule(
    obisCode: ObisCode,
    scriptLogicalName: string,
    scriptSelector: number,
    time: string,
    date: string
  ): string {
    const generator = new SingleActionScheduleClassGenerator()

    // Configurer le script
    const scriptXml = generator.generateSetRequest(obisCode, 2, {
      script_logical_name: scriptLogicalName,
      script_selector: scriptSelector
    })

    // Configurer le type
    const typeXml = generator.generateSetRequest(obisCode, 3, ScheduleType.SIZE_1_WILDCARD_ALLOWED)

    // Configurer l'heure d'exécution
    const executionTimeXml = generator.generateSetRequest(obisCode, 4, [
      { time, date }
    ])

    return `${scriptXml}\n\n${typeXml}\n\n${executionTimeXml}`
  }

  /**
   * Crée un planning d'exécution multiple (type 5)
   * Usage: Plusieurs exécutions à des heures différentes avec wildcards
   */
  static createMultipleSchedule(
    obisCode: ObisCode,
    scriptLogicalName: string,
    scriptSelector: number,
    executionTimes: ExecutionTimeDate[]
  ): string {
    const generator = new SingleActionScheduleClassGenerator()

    // Configurer le script
    const scriptXml = generator.generateSetRequest(obisCode, 2, {
      script_logical_name: scriptLogicalName,
      script_selector: scriptSelector
    })

    // Configurer le type
    const typeXml = generator.generateSetRequest(obisCode, 3, ScheduleType.SIZE_N_DIFFERENT_TIME_WILDCARD_ALLOWED)

    // Configurer les heures d'exécution
    const executionTimeXml = generator.generateSetRequest(obisCode, 4, executionTimes)

    return `${scriptXml}\n\n${typeXml}\n\n${executionTimeXml}`
  }

  /**
   * Génère un exemple complet d'utilisation
   */
  static generateUsageExamples(): { [key: string]: string } {
    return {
      'Simple Schedule - Daily at 00:00': SingleActionScheduleClassGenerator.createSimpleSchedule(
        '0-0:15.0.1.0',
        '0-0:10.0.1.0', // Script table logical name
        1, // Script selector
        '00000000FF', // Time: 00:00:00 (hundredths=FF wildcard)
        '07FFFFFFFFFF0000FF' // Date: wildcards pour tous les jours
      ),

      'Multiple Schedule - Three times a day': SingleActionScheduleClassGenerator.createMultipleSchedule(
        '0-0:15.0.1.1',
        '0-0:10.0.1.0',
        2,
        [
          { time: '06000000FF', date: '07FFFFFFFFFF0000FF' }, // 06:00:00
          { time: '0C000000FF', date: '07FFFFFFFFFF0000FF' }, // 12:00:00
          { time: '12000000FF', date: '07FFFFFFFFFF0000FF' }  // 18:00:00
        ]
      )
    }
  }
}

// ===============================================
// EXPORT
// ===============================================

export default SingleActionScheduleClassGenerator
