/**
 * DLMS COSEM Class 3 - Register
 * 
 * @fileoverview Générateur pour la classe Register (class_id = 3, version = 0)
 * @version 1.0.0
 * @author DLMS COSEM Generator
 * 
 * Description: Cette IC permet de modéliser une valeur de processus ou d'état 
 * avec son facteur d'échelle et son unité associés.
 * Blue Book Ed. 16 Part 2, Section 4.3.2
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
// INTERFACES SPÉCIALISÉES REGISTER
// ===============================================

/** Structure scaler_unit selon Blue Book */
export interface ScalerUnit {
  /** Exposant (base 10) du facteur de multiplication */
  scaler: number  // integer (-128 à 127)
  /** Code unité selon Table 5 du Blue Book */
  unit: number    // enum (0-255)
}

/** Unités DLMS communes selon Blue Book Table 5 */
export enum DlmsUnit {
  // Unités de base
  UNDEFINED = 0,
  YEAR = 1,
  MONTH = 2,
  WEEK = 3,
  DAY = 4,
  HOUR = 5,
  MIN = 6,
  SECOND = 7,
  DEGREE = 8,
  DEGREE_CELSIUS = 9,
  
  // Unités électriques
  AMPERE = 33,
  VOLT = 35,
  WATT = 27,
  VAR = 28,
  WH = 30,         // Watt-heure
  VARH = 31,       // VAr-heure
  
  // Unités eau/gaz
  CUBIC_METRE = 14,        // m³
  CUBIC_METRE_PER_HOUR = 15, // m³/h
  LITRE = 25,
  LITRE_PER_HOUR = 26,
  
  // Unités pression/température
  PASCAL = 37,
  BAR = 40,
  KELVIN = 41,
  
  // Unités sans dimension
  COUNT = 255,     // Sans unité
  PERCENTAGE = 44
}

// ===============================================
// CLASS 3 - REGISTER
// ===============================================

export class RegisterClassGenerator extends BaseCosemClassGenerator {
  readonly classId: ClassId = 3
  readonly version: Version = 0
  readonly className: string = 'Register'
  readonly description: string = 'Valeur de processus/état avec facteur d\'échelle et unité'

  // ===============================================
  // DÉFINITION DES ATTRIBUTS
  // ===============================================

  protected attributes = new Map<AttributeIndex, AttributeInfo>([
    [1, this.createAttributeInfo(
      1, 'logical_name', 'octet-string', AccessMode.READ_ONLY, true, true,
      'Identifie l\'instance de l\'objet Register'
    )],
    [2, this.createAttributeInfo(
      2, 'value', 'CHOICE', AccessMode.READ_AND_WRITE, true, false,
      'Contient la valeur actuelle du processus ou de l\'état'
    )],
    [3, this.createAttributeInfo(
      3, 'scaler_unit', 'scal_unit_type', AccessMode.READ_ONLY, true, true,
      'Informations sur l\'unité et le facteur d\'échelle de la valeur'
    )]
  ])

  // ===============================================
  // DÉFINITION DES MÉTHODES
  // ===============================================

  protected methods = new Map<number, MethodInfo>([
    [1, this.createMethodInfo(
      1, 'reset', false, 'integer(0)',
      'Force une remise à zéro de l\'objet vers sa valeur par défaut'
    )]
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
      throw new Error(`Attribut ${attributeIndex} non supporté pour Register class`)
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
      
      case 2: // value
        return this.createValueAttribute(value)
      
      case 3: // scaler_unit
        return this.createScalerUnitAttribute(value)
      
      default:
        throw new Error(`Attribut ${attributeIndex} non supporté pour Register class`)
    }
  }

  // ===============================================
  // CRÉATEURS D'ATTRIBUTS SPÉCIALISÉS
  // ===============================================

  private createLogicalNameValue(value: string): DlmsValue {
    const hexValue = DlmsXmlGenerator.obisToHex(value)
    return {
      type: DlmsDataType.OCTET_STRING,
      value: hexValue,
      hexValue
    }
  }

  /**
   * Crée l'attribut value pour Register
   * Types autorisés selon Blue Book: types numériques principalement
   */
  private createValueAttribute(value: any): DlmsValue {
    if (value === null || value === undefined) {
      return { type: DlmsDataType.NULL_DATA, value: null }
    }

    // Nombre entier
    if (typeof value === 'number' && Number.isInteger(value)) {
      if (value >= 0) {
        // Valeurs positives - priorité aux types unsigned
        if (value <= 255) {
          return { type: DlmsDataType.UNSIGNED, value }
        }
        if (value <= 65535) {
          return { type: DlmsDataType.LONG_UNSIGNED, value }
        }
        if (value <= 4294967295) {
          return { type: DlmsDataType.DOUBLE_LONG_UNSIGNED, value }
        }
        return { type: DlmsDataType.LONG64_UNSIGNED, value }
      } else {
        // Valeurs négatives - types signés
        if (value >= -128 && value <= 127) {
          return { type: DlmsDataType.INTEGER, value }
        }
        if (value >= -32768 && value <= 32767) {
          return { type: DlmsDataType.LONG, value }
        }
        if (value >= -2147483648 && value <= 2147483647) {
          return { type: DlmsDataType.DOUBLE_LONG, value }
        }
        return { type: DlmsDataType.LONG64, value }
      }
    }

    // Nombre décimal - privilégier float32 pour les mesures
    if (typeof value === 'number') {
      return { type: DlmsDataType.FLOAT32, value }
    }

    // Bit string pour status registers
    if (typeof value === 'string' && /^[01]+$/.test(value)) {
      return { type: DlmsDataType.BIT_STRING, value }
    }

    // Octet string si format hex
    if (typeof value === 'string' && /^[0-9A-Fa-f]*$/.test(value) && value.length % 2 === 0) {
      return { 
        type: DlmsDataType.OCTET_STRING, 
        value: value.toUpperCase(),
        hexValue: value.toUpperCase()
      }
    }

    // Chaîne visible pour ID/serial numbers
    if (typeof value === 'string') {
      return { type: DlmsDataType.VISIBLE_STRING, value }
    }

    throw new Error(`Type de valeur non supporté pour Register: ${typeof value}`)
  }

  /**
   * Crée l'attribut scaler_unit (structure)
   */
  private createScalerUnitAttribute(scalerUnit: ScalerUnit): DlmsValue {
    if (!scalerUnit || typeof scalerUnit !== 'object') {
      throw new Error('scaler_unit doit être un objet {scaler: number, unit: number}')
    }

    const { scaler, unit } = scalerUnit

    // Validation des valeurs
    if (typeof scaler !== 'number' || scaler < -128 || scaler > 127) {
      throw new Error('scaler doit être un entier entre -128 et 127')
    }

    if (typeof unit !== 'number' || unit < 0 || unit > 255) {
      throw new Error('unit doit être un entier entre 0 et 255')
    }

    const fields: DlmsValue[] = [
      { type: DlmsDataType.INTEGER, value: scaler },
      { type: DlmsDataType.ENUM, value: unit }
    ]

    return { type: DlmsDataType.STRUCTURE, value: fields }
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
      case 2: // value
        // Les Register acceptent principalement des valeurs numériques
        if (typeof value !== 'number' && typeof value !== 'string') {
          throw new Error('La valeur Register doit être numérique ou string (hex/bit)')
        }
        return true

      case 3: // scaler_unit
        if (!value || typeof value !== 'object') {
          throw new Error('scaler_unit doit être un objet {scaler, unit}')
        }
        if (!('scaler' in value) || !('unit' in value)) {
          throw new Error('scaler_unit doit contenir les propriétés scaler et unit')
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
   * Crée un Register pour une mesure d'énergie (kWh)
   */
  static createEnergyRegister(obisCode: ObisCode, value: number, scaler: number = -3): string {
    const generator = new RegisterClassGenerator()
    const examples = [
      generator.generateSetRequest(obisCode, 2, value),
      generator.generateSetRequest(obisCode, 3, { scaler, unit: DlmsUnit.WH })
    ]
    return examples.join('\n\n')
  }

  /**
   * Crée un Register pour une mesure de volume d'eau (m³)
   */
  static createWaterVolumeRegister(obisCode: ObisCode, volume: number, scaler: number = -3): string {
    const generator = new RegisterClassGenerator()
    const examples = [
      generator.generateSetRequest(obisCode, 2, volume),
      generator.generateSetRequest(obisCode, 3, { scaler, unit: DlmsUnit.CUBIC_METRE })
    ]
    return examples.join('\n\n')
  }

  /**
   * Crée un Register pour un débit (L/h)
   */
  static createFlowRateRegister(obisCode: ObisCode, flowRate: number): string {
    const generator = new RegisterClassGenerator()
    const examples = [
      generator.generateSetRequest(obisCode, 2, flowRate),
      generator.generateSetRequest(obisCode, 3, { scaler: 0, unit: DlmsUnit.LITRE_PER_HOUR })
    ]
    return examples.join('\n\n')
  }

  /**
   * Crée un Register pour une température (°C)
   */
  static createTemperatureRegister(obisCode: ObisCode, temperature: number, scaler: number = -1): string {
    const generator = new RegisterClassGenerator()
    const examples = [
      generator.generateSetRequest(obisCode, 2, temperature),
      generator.generateSetRequest(obisCode, 3, { scaler, unit: DlmsUnit.DEGREE_CELSIUS })
    ]
    return examples.join('\n\n')
  }

  /**
   * Génère des exemples d'utilisation typiques pour WSM GCP
   */
  static generateWsmGcpExamples(): { [key: string]: string } {
    return {
      'Water Total Consumption': RegisterClassGenerator.createWaterVolumeRegister(
        '1-0:1.8.0.255', 
        123456, // 123.456 m³
        -3
      ),
      
      'Instantaneous Flow Rate': RegisterClassGenerator.createFlowRateRegister(
        '1-0:2.27.0.255',
        150 // 150 L/h
      ),
      
      'Clock Time Shift Limit': (() => {
        const generator = new RegisterClassGenerator()
        return [
          generator.generateSetRequest('0-0:1.0.9.255', 2, 300), // 5 minutes
          generator.generateSetRequest('0-0:1.0.9.255', 3, { scaler: 0, unit: DlmsUnit.SECOND })
        ].join('\n\n')
      })(),
      
      'Water Freezing Index': RegisterClassGenerator.createWaterVolumeRegister(
        '1-0:1.8.2.255',
        87654, // 87.654 m³
        -3
      ),
      
      'Average Flow Rate': RegisterClassGenerator.createFlowRateRegister(
        '1-0:2.27.1.255',
        75 // 75 L/h moyens
      )
    }
  }
}

// ===============================================
// EXPORT
// ===============================================

export default RegisterClassGenerator
