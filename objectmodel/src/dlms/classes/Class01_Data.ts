/**
 * DLMS COSEM Class 1 - Data
 * 
 * @fileoverview Générateur pour la classe Data (class_id = 1, version = 0)
 * @version 1.0.0
 * @author DLMS COSEM Generator
 * 
 * Description: Cette IC permet de modéliser diverses données, 
 * telles que les données de configuration et les paramètres.
 * Blue Book Ed. 16 Part 2, Section 4.3.1
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
// CLASS 1 - DATA
// ===============================================

export class DataClassGenerator extends BaseCosemClassGenerator {
  readonly classId: ClassId = 1
  readonly version: Version = 0
  readonly className: string = 'Data'
  readonly description: string = 'Modélisation de données diverses (configuration, paramètres)'

  // ===============================================
  // DÉFINITION DES ATTRIBUTS
  // ===============================================

  protected attributes = new Map<AttributeIndex, AttributeInfo>([
    [1, this.createAttributeInfo(
      1, 'logical_name', 'octet-string', AccessMode.READ_ONLY, true, true,
      'Identifie l\'instance de l\'objet Data'
    )],
    [2, this.createAttributeInfo(
      2, 'value', 'CHOICE', AccessMode.READ_AND_WRITE, true, false,
      'Contient les données. Type déterminé par l\'implémentation selon logical_name'
    )]
  ])

  // ===============================================
  // DÉFINITION DES MÉTHODES
  // ===============================================

  protected methods = new Map<number, MethodInfo>([
    // Classe Data n'a pas de méthodes spécifiques
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
      throw new Error(`Attribut ${attributeIndex} non supporté pour Data class`)
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
      
      default:
        throw new Error(`Attribut ${attributeIndex} non supporté pour Data class`)
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
   * Crée l'attribut value avec auto-détection intelligente du type
   * Selon Blue Book: "Le type de données dépend de l'instanciation définie 
   * par logical_name et éventuellement du fabricant"
   */
  private createValueAttribute(value: any): DlmsValue {
    // Auto-détection du type selon la valeur et les conventions DLMS
    
    // null ou undefined
    if (value === null || value === undefined) {
      return { type: DlmsDataType.NULL_DATA, value: null }
    }
    
    // Booléen
    if (typeof value === 'boolean') {
      return { type: DlmsDataType.BOOLEAN, value }
    }
    
    // Nombre entier
    if (typeof value === 'number' && Number.isInteger(value)) {
      if (value >= 0 && value <= 255) {
        return { type: DlmsDataType.UNSIGNED, value }
      }
      if (value >= 0 && value <= 65535) {
        return { type: DlmsDataType.LONG_UNSIGNED, value }
      }
      if (value >= 0 && value <= 4294967295) {
        return { type: DlmsDataType.DOUBLE_LONG_UNSIGNED, value }
      }
      if (value >= -128 && value <= 127) {
        return { type: DlmsDataType.INTEGER, value }
      }
      if (value >= -32768 && value <= 32767) {
        return { type: DlmsDataType.LONG, value }
      }
      return { type: DlmsDataType.DOUBLE_LONG, value }
    }
    
    // Nombre décimal
    if (typeof value === 'number' && !Number.isInteger(value)) {
      return { type: DlmsDataType.FLOAT64, value }
    }
    
    // Chaîne de caractères
    if (typeof value === 'string') {
      // Détection format hex pour octet-string
      if (/^[0-9A-Fa-f]*$/.test(value) && value.length % 2 === 0 && value.length > 0) {
        return { 
          type: DlmsDataType.OCTET_STRING, 
          value: value.toUpperCase(),
          hexValue: value.toUpperCase()
        }
      }
      
      // Chaîne visible ASCII
      if (/^[\x20-\x7E]*$/.test(value)) {
        return { type: DlmsDataType.VISIBLE_STRING, value }
      }
      
      // UTF-8 pour caractères étendus
      return { type: DlmsDataType.UTF8_STRING, value }
    }
    
    // Date
    if (value instanceof Date) {
      return { type: DlmsDataType.DATE_TIME, value }
    }
    
    // Tableau -> Array DLMS
    if (Array.isArray(value)) {
      const elements = value.map(v => this.createValueAttribute(v))
      return { type: DlmsDataType.ARRAY, value: elements }
    }
    
    // Objet -> Structure DLMS
    if (typeof value === 'object') {
      const fields = Object.values(value).map(v => this.createValueAttribute(v))
      return { type: DlmsDataType.STRUCTURE, value: fields }
    }
    
    // Par défaut: conversion en string puis octet-string
    return { type: DlmsDataType.VISIBLE_STRING, value: value.toString() }
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
          throw new Error('logical_name doit être une chaîne OBIS (ex: "0-0:96.1.0.255")')
        }
        // Validation format OBIS
        if (!/^\d+-\d+:\d+\.\d+\.\d+\.\d+$/.test(value) && !/^[0-9A-Fa-f]{12}$/.test(value)) {
          throw new Error(`Format OBIS invalide: ${value}`)
        }
        return true
        
      case 2: // value
        // La classe Data accepte tous types de valeurs
        // La validation spécifique dépend du contexte d'utilisation
        return true
        
      default:
        return super.validateAttributeValue(attributeIndex, value, attrInfo)
    }
  }

  // ===============================================
  // MÉTHODES UTILITAIRES SPÉCIFIQUES
  // ===============================================

  /**
   * Crée une instance Data pour un Device ID
   * Usage typique: Device ID 1, manufacturing number (0-0:96.1.0.255)
   */
  static createDeviceId(obisCode: ObisCode, deviceId: string): string {
    const generator = new DataClassGenerator()
    return generator.generateSetRequest(obisCode, 2, deviceId)
  }

  /**
   * Crée une instance Data pour une configuration booléenne
   * Usage typique: Function control, Battery capacity, etc.
   */
  static createBooleanConfig(obisCode: ObisCode, enabled: boolean): string {
    const generator = new DataClassGenerator()
    return generator.generateSetRequest(obisCode, 2, enabled)
  }

  /**
   * Crée une instance Data pour un compteur/index numérique
   * Usage typique: Event Counter Objects
   */
  static createCounter(obisCode: ObisCode, count: number): string {
    const generator = new DataClassGenerator()
    return generator.generateSetRequest(obisCode, 2, count)
  }

  /**
   * Crée une instance Data pour un registre de status (bit field)
   * Usage typique: Alarm Register, Error Register
   */
  static createStatusRegister(obisCode: ObisCode, statusBits: number): string {
    const generator = new DataClassGenerator()
    // Convertir en hex pour représentation bit field
    const hexValue = statusBits.toString(16).toUpperCase().padStart(2, '0')
    return generator.generateSetRequest(obisCode, 2, hexValue)
  }

  /**
   * Crée une instance Data pour une chaîne de configuration
   * Usage typique: COSEM logical device name, Firmware identifiers
   */
  static createStringConfig(obisCode: ObisCode, configValue: string): string {
    const generator = new DataClassGenerator()
    return generator.generateSetRequest(obisCode, 2, configValue)
  }

  /**
   * Génère un exemple complet d'utilisation
   */
  static generateUsageExamples(): { [key: string]: string } {
    return {
      'Device ID Manufacturing Number': DataClassGenerator.createDeviceId(
        '0-0:96.1.0.255', 
        'WSM12345678'
      ),
      
      'Battery Capacity': DataClassGenerator.createBooleanConfig(
        '0-0:96.6.1.255', 
        true
      ),
      
      'Event Counter Global': DataClassGenerator.createCounter(
        '0-0:96.15.0.255', 
        42
      ),
      
      'Alarm Register': DataClassGenerator.createStatusRegister(
        '0-0:97.98.0.255', 
        0x1F // 5 bits activés
      ),
      
      'Active Firmware ID': DataClassGenerator.createStringConfig(
        '1-0:0.2.0.255', 
        'FW_v2.1.3'
      )
    }
  }
}

// ===============================================
// EXPORT
// ===============================================

export default DataClassGenerator