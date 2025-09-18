/**
 * Interface de base pour toutes les classes COSEM
 * 
 * @fileoverview Définit le contrat standard pour tous les générateurs de classes COSEM
 * @version 1.0.0
 * @author DLMS COSEM Generator
 */

import {
  ClassId,
  Version,
  ObisCode,
  AttributeIndex,
  DlmsValue,
  CosemClassInfo,
  AttributeInfo,
  MethodInfo
} from '../types/common'

// ===============================================
// INTERFACE PRINCIPALE COSEM
// ===============================================

/**
 * Interface contractuelle pour tous les générateurs de classes COSEM
 * Chaque class_id doit implémenter cette interface
 */
export interface ICosemClassGenerator {
  /** Identifiant de classe COSEM */
  readonly classId: ClassId
  
  /** Version de la classe supportée */
  readonly version: Version
  
  /** Nom de la classe pour documentation */
  readonly className: string
  
  /** Description courte de la classe */
  readonly description: string

  /**
   * Génère un SetRequest XML pour un attribut donné
   * @param obisCode Code OBIS de l'objet
   * @param attributeIndex Index de l'attribut à modifier
   * @param value Valeur à écrire
   * @param invokeId ID d'invocation (optionnel, défaut: 1)
   * @returns XML SetRequest complet
   */
  generateSetRequest(
    obisCode: ObisCode, 
    attributeIndex: AttributeIndex, 
    value: any,
    invokeId?: number
  ): string

  /**
   * Génère un GetRequest XML pour un attribut donné
   * @param obisCode Code OBIS de l'objet
   * @param attributeIndex Index de l'attribut à lire
   * @param invokeId ID d'invocation (optionnel, défaut: 1)
   * @returns XML GetRequest complet
   */
  generateGetRequest(
    obisCode: ObisCode, 
    attributeIndex: AttributeIndex,
    invokeId?: number
  ): string

  /**
   * Valide une valeur pour un attribut donné
   * @param attributeIndex Index de l'attribut
   * @param value Valeur à valider
   * @returns true si valide, sinon lève une exception
   */
  validateAttribute(attributeIndex: AttributeIndex, value: any): boolean

  /**
   * Crée une valeur DLMS typée pour un attribut
   * @param attributeIndex Index de l'attribut
   * @param value Valeur native JavaScript
   * @returns Valeur DLMS typée
   */
  createAttributeValue(attributeIndex: AttributeIndex, value: any): DlmsValue

  /**
   * Obtient les informations sur la classe
   * @returns Métadonnées complètes de la classe
   */
  getClassInfo(): CosemClassInfo

  /**
   * Obtient les informations sur un attribut spécifique
   * @param attributeIndex Index de l'attribut
   * @returns Métadonnées de l'attribut ou null si inexistant
   */
  getAttributeInfo(attributeIndex: AttributeIndex): AttributeInfo | null

  /**
   * Liste tous les attributs supportés
   * @returns Array des informations d'attributs
   */
  getSupportedAttributes(): AttributeInfo[]

  /**
   * Vérifie si un attribut est en écriture
   * @param attributeIndex Index de l'attribut
   * @returns true si l'attribut peut être écrit
   */
  isWritableAttribute(attributeIndex: AttributeIndex): boolean

  /**
   * Vérifie si un attribut est obligatoire
   * @param attributeIndex Index de l'attribut
   * @returns true si l'attribut est obligatoire
   */
  isMandatoryAttribute(attributeIndex: AttributeIndex): boolean
}

// ===============================================
// CLASSE DE BASE ABSTRAITE
// ===============================================

/**
 * Classe de base abstraite fournissant l'implémentation commune
 * pour tous les générateurs COSEM
 */
export abstract class BaseCosemClassGenerator implements ICosemClassGenerator {
  
  abstract readonly classId: ClassId
  abstract readonly version: Version  
  abstract readonly className: string
  abstract readonly description: string

  /** Définition des attributs - à surcharger dans chaque classe */
  protected abstract attributes: Map<AttributeIndex, AttributeInfo>
  
  /** Définition des méthodes - à surcharger dans chaque classe */
  protected abstract methods: Map<number, MethodInfo>

  // ===============================================
  // IMPLÉMENTATION ABSTRAITE
  // ===============================================

  abstract generateSetRequest(
    obisCode: ObisCode, 
    attributeIndex: AttributeIndex, 
    value: any,
    invokeId?: number
  ): string

  abstract generateGetRequest(
    obisCode: ObisCode, 
    attributeIndex: AttributeIndex,
    invokeId?: number
  ): string

  abstract createAttributeValue(attributeIndex: AttributeIndex, value: any): DlmsValue

  // ===============================================
  // IMPLÉMENTATION COMMUNE
  // ===============================================

  validateAttribute(attributeIndex: AttributeIndex, value: any): boolean {
    const attrInfo = this.getAttributeInfo(attributeIndex)
    if (!attrInfo) {
      throw new Error(`Attribut ${attributeIndex} non supporté pour class_id ${this.classId}`)
    }

    // Validation basique selon le type
    if (value === null || value === undefined) {
      if (attrInfo.mandatory) {
        throw new Error(`Attribut ${attributeIndex} (${attrInfo.name}) est obligatoire`)
      }
      return true
    }

    // Validation spécifique selon l'attribut - à surcharger si nécessaire
    return this.validateAttributeValue(attributeIndex, value, attrInfo)
  }

  /**
   * Validation spécifique par attribut - peut être surchargée
   */
  protected validateAttributeValue(
    attributeIndex: AttributeIndex, 
    value: any, 
    attrInfo: AttributeInfo
  ): boolean {
    // Validation par défaut - les classes spécialisées peuvent surcharger
    return true
  }

  getClassInfo(): CosemClassInfo {
    return {
      classId: this.classId,
      version: this.version,
      name: this.className,
      description: this.description,
      attributes: Array.from(this.attributes.values()),
      methods: Array.from(this.methods.values())
    }
  }

  getAttributeInfo(attributeIndex: AttributeIndex): AttributeInfo | null {
    return this.attributes.get(attributeIndex) || null
  }

  getSupportedAttributes(): AttributeInfo[] {
    return Array.from(this.attributes.values()).sort((a, b) => a.index - b.index)
  }

  isWritableAttribute(attributeIndex: AttributeIndex): boolean {
    const attr = this.getAttributeInfo(attributeIndex)
    if (!attr) return false
    
    // AccessMode: READ_AND_WRITE (3) ou WRITE_ONLY (2) ou AUTHENTICATED_*_WRITE
    return [2, 3, 5, 6].includes(attr.accessMode)
  }

  isMandatoryAttribute(attributeIndex: AttributeIndex): boolean {
    const attr = this.getAttributeInfo(attributeIndex)
    return attr?.mandatory || false
  }

  // ===============================================
  // HELPERS PROTECTED POUR CLASSES DÉRIVÉES
  // ===============================================

  /**
   * Helper pour créer les informations d'attribut standard
   */
  protected createAttributeInfo(
    index: AttributeIndex,
    name: string,
    dataType: string,
    accessMode: number = 1, // READ_ONLY par défaut
    mandatory: boolean = true,
    isStatic: boolean = false,
    description?: string
  ): AttributeInfo {
    return {
      index,
      name,
      dataType,
      accessMode,
      mandatory,
      static: isStatic,
      description
    }
  }

  /**
   * Helper pour créer les informations de méthode standard
   */
  protected createMethodInfo(
    id: number,
    name: string,
    mandatory: boolean = false,
    parameterType?: string,
    description?: string
  ): MethodInfo {
    return {
      id,
      name,
      mandatory,
      parameterType,
      description
    }
  }

  /**
   * Initialise les attributs communs (logical_name est toujours présent)
   */
  protected initializeCommonAttributes(): Map<AttributeIndex, AttributeInfo> {
    const attrs = new Map<AttributeIndex, AttributeInfo>()
    
    // Attribut 1: logical_name (toujours présent dans toutes les classes)
    attrs.set(1, this.createAttributeInfo(
      1,
      'logical_name',
      'octet-string',
      1, // READ_ONLY
      true,
      true, // Static
      'Identifie l\'instance de l\'objet COSEM selon OBIS'
    ))
    
    return attrs
  }
}

// ===============================================
// TYPES HELPERS POUR CRÉATION DE CLASSES
// ===============================================

/** Configuration pour créer rapidement un attribut */
export interface AttributeConfig {
  index: AttributeIndex
  name: string
  dataType: string
  accessMode?: number
  mandatory?: boolean
  static?: boolean
  description?: string
}

/** Configuration pour créer rapidement une méthode */
export interface MethodConfig {
  id: number
  name: string
  mandatory?: boolean
  parameterType?: string
  description?: string
}

/** Configuration complète pour une classe COSEM */
export interface CosemClassConfig {
  classId: ClassId
  version: Version
  className: string
  description: string
  attributes: AttributeConfig[]
  methods?: MethodConfig[]
}

// ===============================================
// EXPORT
// ===============================================

export default ICosemClassGenerator