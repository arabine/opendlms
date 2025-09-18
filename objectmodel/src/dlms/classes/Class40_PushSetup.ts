/**
 * DLMS COSEM Class 40 - Push Setup
 * 
 * @fileoverview Générateur pour la classe Push Setup (class_id = 40, version = 2)
 * @version 1.0.0
 * @author DLMS COSEM Generator
 * 
 * Description: Contient une liste de références aux attributs d'objets COSEM à pousser.
 * Inclut aussi la destination push et la méthode ainsi que les fenêtres de communication
 * et la gestion des tentatives.
 * Blue Book Ed. 16 Part 2, Section 5.4.11 (version 2)
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
  AccessMode,
  DlmsDateTime
} from '../types/common'
import { BaseCosemClassGenerator } from '../interfaces/ICosemClass'
import DlmsXmlGenerator from '../utils/xmlGenerator'

// ===============================================
// INTERFACES SPÉCIALISÉES PUSH SETUP
// ===============================================

/** Structure push_object_definition selon Blue Book version 2 */
export interface PushObjectDefinition {
  /** Class ID de l'objet COSEM */
  class_id: number
  /** Nom logique (OBIS hex) */
  logical_name: string
  /** Index de l'attribut */
  attribute_index: number
  /** Index de données (sélection dans structure/array) */
  data_index: number
  /** Restriction d'accès aux données */
  restriction: RestrictionElement
  /** Sélection de colonnes (pour Profile Generic) */
  columns: CaptureObjectDefinition[]
}

/** Types de restriction */
export enum RestrictionType {
  NONE = 0,
  RESTRICTION_BY_DATE = 1,
  RESTRICTION_BY_ENTRY = 2
}

/** Élément de restriction */
export interface RestrictionElement {
  /** Type de restriction */
  restriction_type: RestrictionType
  /** Valeur de restriction selon le type */
  restriction_value?: RestrictionByDate | RestrictionByEntry | null
}

/** Restriction par date */
export interface RestrictionByDate {
  from_date: string  // octet-string format date
  to_date: string    // octet-string format date
}

/** Restriction par entrée */
export interface RestrictionByEntry {
  from_entry: number  // double-long-unsigned
  to_entry: number    // double-long-unsigned
}

/** Définition d'objet de capture */
export interface CaptureObjectDefinition {
  class_id: number
  logical_name: string
  attribute_index: number
  data_index: number
}

/** Destination et méthode d'envoi */
export interface SendDestinationAndMethod {
  /** Type de service de transport */
  transport_service: TransportServiceType
  /** Adresse de destination */
  destination: string
  /** Type de message */
  message: MessageType
}

/** Types de service de transport */
export enum TransportServiceType {
  TCP = 0,
  UDP = 1,
  FTP = 2,
  SMTP = 3,
  SMS = 4,
  HDLC = 5,
  M_BUS = 6,
  ZIGBEE = 7,
  DLMS_GATEWAY = 8,
  RELIABLE_COAP = 9,
  UNRELIABLE_COAP = 10
}

/** Types de message */
export enum MessageType {
  A_XDR_ENCODED_XDLMS_APDU = 0,
  XML_ENCODED_XDLMS_APDU = 1
}

/** Fenêtre de communication */
export interface CommunicationWindow {
  start_time: Date | DlmsDateTime
  end_time: Date | DlmsDateTime
}

/** Délai de répétition (version 2+) */
export interface RepetitionDelay {
  min: number         // Délai minimum
  exponent: number    // Exposant pour progression
  max: number         // Délai maximum
}

/** Paramètres de protection */
export interface ProtectionParameters {
  protected_object: string    // Référence objet protégé
  protection_type: number     // Type de protection
  protection_options: any[]   // Options spécifiques
}

/** Paramètres de confirmation */
export interface ConfirmationParameters {
  start_date: Date | DlmsDateTime
  interval: number  // Intervalle en secondes
}

// ===============================================
// CLASS 40 - PUSH SETUP
// ===============================================

export class PushSetupGenerator extends BaseCosemClassGenerator {
  readonly classId: ClassId = 40
  readonly version: Version = 2
  readonly className: string = 'Push Setup'
  readonly description: string = 'Configuration push des données COSEM avec gestion avancée'

  // ===============================================
  // DÉFINITION DES ATTRIBUTS
  // ===============================================

  protected attributes = new Map<AttributeIndex, AttributeInfo>([
    [1, this.createAttributeInfo(
      1, 'logical_name', 'octet-string', AccessMode.READ_ONLY, true, true,
      'Identifie l\'instance de l\'objet Push Setup'
    )],
    [2, this.createAttributeInfo(
      2, 'push_object_list', 'array push_object_definition', AccessMode.READ_AND_WRITE, true, true,
      'Liste des attributs à pousser avec restrictions et sélections'
    )],
    [3, this.createAttributeInfo(
      3, 'send_destination_and_method', 'structure', AccessMode.READ_AND_WRITE, true, true,
      'Destination et méthode d\'envoi des données'
    )],
    [4, this.createAttributeInfo(
      4, 'communication_window', 'array', AccessMode.READ_AND_WRITE, true, true,
      'Fenêtres de communication autorisées'
    )],
    [5, this.createAttributeInfo(
      5, 'randomisation_start_interval', 'long-unsigned', AccessMode.READ_AND_WRITE, true, true,
      'Intervalle de randomisation avant démarrage (secondes)'
    )],
    [6, this.createAttributeInfo(
      6, 'number_of_retries', 'unsigned', AccessMode.READ_AND_WRITE, true, true,
      'Nombre maximum de tentatives en cas d\'échec'
    )],
    [7, this.createAttributeInfo(
      7, 'repetition_delay', 'structure', AccessMode.READ_AND_WRITE, true, true,
      'Délai avant nouvelle tentative (formule exponentielle v2+)'
    )],
    [8, this.createAttributeInfo(
      8, 'port_reference', 'octet-string', AccessMode.READ_AND_WRITE, true, true,
      'Référence du port de communication'
    )],
    [9, this.createAttributeInfo(
      9, 'push_client_SAP', 'integer', AccessMode.READ_AND_WRITE, true, true,
      'SAP du client push'
    )],
    [10, this.createAttributeInfo(
      10, 'push_protection_parameters', 'array', AccessMode.READ_AND_WRITE, true, true,
      'Paramètres de protection des données push'
    )],
    [11, this.createAttributeInfo(
      11, 'push_operation_method', 'enum', AccessMode.READ_AND_WRITE, true, true,
      'Méthode d\'opération push (confirmé/non-confirmé)'
    )],
    [12, this.createAttributeInfo(
      12, 'confirmation_parameters', 'structure', AccessMode.READ_AND_WRITE, true, true,
      'Paramètres de limitation des confirmations'
    )],
    [13, this.createAttributeInfo(
      13, 'last_confirmation_date_time', 'date-time', AccessMode.READ_ONLY, false, false,
      'Date/heure de la dernière confirmation'
    )]
  ])

  // ===============================================
  // DÉFINITION DES MÉTHODES
  // ===============================================

  protected methods = new Map<number, MethodInfo>([
    [1, this.createMethodInfo(
      1, 'push', true, 'integer(0)',
      'Active le processus push selon la configuration'
    )],
    [2, this.createMethodInfo(
      2, 'reset', false, 'integer(0)',
      'Remet à zéro les compteurs et état du push'
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
      throw new Error(`Attribut ${attributeIndex} non supporté pour Push Setup class`)
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
      case 2: // push_object_list
        return this.createPushObjectList(value)
      case 3: // send_destination_and_method
        return this.createSendDestination(value)
      case 4: // communication_window
        return this.createCommunicationWindow(value)
      case 5: // randomisation_start_interval
        return { type: DlmsDataType.LONG_UNSIGNED, value: Number(value) }
      case 6: // number_of_retries
        return { type: DlmsDataType.UNSIGNED, value: Number(value) }
      case 7: // repetition_delay
        return this.createRepetitionDelay(value)
      case 8: // port_reference
        return { type: DlmsDataType.OCTET_STRING, value: String(value) }
      case 9: // push_client_SAP
        return { type: DlmsDataType.INTEGER, value: Number(value) }
      case 10: // push_protection_parameters
        return this.createProtectionParameters(value)
      case 11: // push_operation_method
        return { type: DlmsDataType.ENUM, value: Number(value) }
      case 12: // confirmation_parameters
        return this.createConfirmationParameters(value)
      case 13: // last_confirmation_date_time
        return { type: DlmsDataType.DATE_TIME, value: value }
      default:
        throw new Error(`Attribut ${attributeIndex} non supporté pour Push Setup class`)
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
   * Crée push_object_list (array de push_object_definition)
   */
  private createPushObjectList(objects: PushObjectDefinition[]): DlmsValue {
    if (!Array.isArray(objects)) {
      throw new Error('push_object_list doit être un array de PushObjectDefinition')
    }

    const objectElements: DlmsValue[] = objects.map(obj => {
      const fields: DlmsValue[] = [
        { type: DlmsDataType.LONG_UNSIGNED, value: obj.class_id },
        { type: DlmsDataType.OCTET_STRING, value: obj.logical_name },
        { type: DlmsDataType.INTEGER, value: obj.attribute_index },
        { type: DlmsDataType.LONG_UNSIGNED, value: obj.data_index },
        this.createRestriction(obj.restriction),
        this.createColumns(obj.columns || [])
      ]
      return { type: DlmsDataType.STRUCTURE, value: fields }
    })

    return { type: DlmsDataType.ARRAY, value: objectElements }
  }

  /**
   * Crée send_destination_and_method
   */
  private createSendDestination(destination: SendDestinationAndMethod): DlmsValue {
    const fields: DlmsValue[] = [
      { type: DlmsDataType.ENUM, value: destination.transport_service },
      { type: DlmsDataType.OCTET_STRING, value: destination.destination },
      { type: DlmsDataType.ENUM, value: destination.message }
    ]
    return { type: DlmsDataType.STRUCTURE, value: fields }
  }

  /**
   * Crée communication_window (array de fenêtres)
   */
  private createCommunicationWindow(windows: CommunicationWindow[]): DlmsValue {
    if (!Array.isArray(windows)) {
      throw new Error('communication_window doit être un array de CommunicationWindow')
    }

    const windowElements: DlmsValue[] = windows.map(window => {
      const fields: DlmsValue[] = [
        { type: DlmsDataType.DATE_TIME, value: window.start_time },
        { type: DlmsDataType.DATE_TIME, value: window.end_time }
      ]
      return { type: DlmsDataType.STRUCTURE, value: fields }
    })

    return { type: DlmsDataType.ARRAY, value: windowElements }
  }

  /**
   * Crée repetition_delay (structure version 2+)
   */
  private createRepetitionDelay(delay: RepetitionDelay | number): DlmsValue {
    if (typeof delay === 'number') {
      // Version 0/1: simple long-unsigned
      return { type: DlmsDataType.LONG_UNSIGNED, value: delay }
    }
    
    // Version 2+: structure with exponential formula
    const fields: DlmsValue[] = [
      { type: DlmsDataType.LONG_UNSIGNED, value: delay.min },
      { type: DlmsDataType.LONG_UNSIGNED, value: delay.exponent },
      { type: DlmsDataType.LONG_UNSIGNED, value: delay.max }
    ]
    return { type: DlmsDataType.STRUCTURE, value: fields }
  }

  /**
   * Crée push_protection_parameters
   */
  private createProtectionParameters(params: ProtectionParameters[]): DlmsValue {
    if (!Array.isArray(params)) {
      return { type: DlmsDataType.ARRAY, value: [] } // Array vide
    }

    const paramElements: DlmsValue[] = params.map(param => {
      const fields: DlmsValue[] = [
        { type: DlmsDataType.OCTET_STRING, value: param.protected_object },
        { type: DlmsDataType.ENUM, value: param.protection_type },
        { type: DlmsDataType.ARRAY, value: param.protection_options?.map(opt => 
          DlmsXmlGenerator.createSimpleValue(opt)) || [] }
      ]
      return { type: DlmsDataType.STRUCTURE, value: fields }
    })

    return { type: DlmsDataType.ARRAY, value: paramElements }
  }

  /**
   * Crée confirmation_parameters
   */
  private createConfirmationParameters(params: ConfirmationParameters): DlmsValue {
    const fields: DlmsValue[] = [
      { type: DlmsDataType.DATE_TIME, value: params.start_date },
      { type: DlmsDataType.LONG_UNSIGNED, value: params.interval }
    ]
    return { type: DlmsDataType.STRUCTURE, value: fields }
  }

  /**
   * Crée restriction_element
   */
  private createRestriction(restriction: RestrictionElement): DlmsValue {
    const fields: DlmsValue[] = [
      { type: DlmsDataType.ENUM, value: restriction.restriction_type }
    ]

    if (restriction.restriction_type === RestrictionType.NONE || !restriction.restriction_value) {
      fields.push({ type: DlmsDataType.NULL_DATA, value: null })
    } else if (restriction.restriction_type === RestrictionType.RESTRICTION_BY_DATE) {
      const dateRestriction = restriction.restriction_value as RestrictionByDate
      const dateFields: DlmsValue[] = [
        { type: DlmsDataType.OCTET_STRING, value: dateRestriction.from_date },
        { type: DlmsDataType.OCTET_STRING, value: dateRestriction.to_date }
      ]
      fields.push({ type: DlmsDataType.STRUCTURE, value: dateFields })
    } else if (restriction.restriction_type === RestrictionType.RESTRICTION_BY_ENTRY) {
      const entryRestriction = restriction.restriction_value as RestrictionByEntry
      const entryFields: DlmsValue[] = [
        { type: DlmsDataType.DOUBLE_LONG_UNSIGNED, value: entryRestriction.from_entry },
        { type: DlmsDataType.DOUBLE_LONG_UNSIGNED, value: entryRestriction.to_entry }
      ]
      fields.push({ type: DlmsDataType.STRUCTURE, value: entryFields })
    }

    return { type: DlmsDataType.STRUCTURE, value: fields }
  }

  /**
   * Crée columns (array de capture_object_definition)
   */
  private createColumns(columns: CaptureObjectDefinition[]): DlmsValue {
    if (columns.length === 0) {
      return { type: DlmsDataType.ARRAY, value: [] }
    }
    
    const columnElements: DlmsValue[] = columns.map(col => {
      const fields: DlmsValue[] = [
        { type: DlmsDataType.LONG_UNSIGNED, value: col.class_id },
        { type: DlmsDataType.OCTET_STRING, value: col.logical_name },
        { type: DlmsDataType.INTEGER, value: col.attribute_index },
        { type: DlmsDataType.LONG_UNSIGNED, value: col.data_index }
      ]
      return { type: DlmsDataType.STRUCTURE, value: fields }
    })

    return { type: DlmsDataType.ARRAY, value: columnElements }
  }

  // ===============================================
  // MÉTHODES UTILITAIRES SPÉCIFIQUES
  // ===============================================

  /**
   * Crée une configuration Push "On Connectivity" typique WSM GCP
   */
  static createOnConnectivityPush(obisCode: ObisCode, destination: string): string {
    const generator = new PushSetupGenerator()
    
    const pushObjects: PushObjectDefinition[] = [
      {
        class_id: 8, // Clock
        logical_name: "0000010000FF", // 0-0:1.0.0.255
        attribute_index: 2, // time
        data_index: 0,
        restriction: { restriction_type: RestrictionType.NONE },
        columns: []
      },
      {
        class_id: 1, // Data - Device ID
        logical_name: "0000600100FF", // 0-0:96.1.0.255
        attribute_index: 2, // value
        data_index: 0,
        restriction: { restriction_type: RestrictionType.NONE },
        columns: []
      },
      {
        class_id: 3, // Register - Water total
        logical_name: "0100010800FF", // 1-0:1.8.0.255
        attribute_index: 2, // value
        data_index: 0,
        restriction: { restriction_type: RestrictionType.NONE },
        columns: []
      }
    ]

    const sendDestination: SendDestinationAndMethod = {
      transport_service: TransportServiceType.TCP,
      destination: destination,
      message: MessageType.A_XDR_ENCODED_XDLMS_APDU
    }

    return generator.generateSetRequest(obisCode, 2, pushObjects) + '\n\n' +
           generator.generateSetRequest(obisCode, 3, sendDestination) + '\n\n' +
           generator.generateSetRequest(obisCode, 5, 60) + '\n\n' + // 1 minute random
           generator.generateSetRequest(obisCode, 6, 3) // 3 retries
  }

  /**
   * Crée une configuration Push "On Alarm" avec restriction par entrée
   */
  static createOnAlarmPush(obisCode: ObisCode, destination: string): string {
    const generator = new PushSetupGenerator()
    
    const pushObjects: PushObjectDefinition[] = [
      {
        class_id: 7, // Profile Generic - Standard Event Log
        logical_name: "0000630100FF", // 0-0:99.1.0.255
        attribute_index: 2, // buffer
        data_index: 0x1001, // Dernières 10 entrées
        restriction: { 
          restriction_type: RestrictionType.RESTRICTION_BY_ENTRY,
          restriction_value: { from_entry: 0, to_entry: 10 }
        },
        columns: [
          {
            class_id: 8, // Clock pour timestamp
            logical_name: "0000010000FF",
            attribute_index: 2,
            data_index: 0
          }
        ]
      }
    ]

    const sendDestination: SendDestinationAndMethod = {
      transport_service: TransportServiceType.UDP,
      destination: destination,
      message: MessageType.A_XDR_ENCODED_XDLMS_APDU
    }

    return generator.generateSetRequest(obisCode, 2, pushObjects) + '\n\n' +
           generator.generateSetRequest(obisCode, 3, sendDestination) + '\n\n' +
           generator.generateSetRequest(obisCode, 5, 10) + '\n\n' + // 10s random
           generator.generateSetRequest(obisCode, 6, 5) // 5 retries
  }

  /**
   * Génère des exemples complets pour tous les types de Push Setup WSM GCP
   */
  static generateWsmGcpExamples(): { [key: string]: string } {
    return {
      'Push Setup - On Connectivity': PushSetupGenerator.createOnConnectivityPush(
        '0-0:25.9.0.255',
        '192.168.1.100:4059'
      ),
      
      'Push Setup - On Alarm': PushSetupGenerator.createOnAlarmPush(
        '0-0:25.9.1.255',
        '192.168.1.100:4060'
      ),
      
      'Push Setup - Interval (Daily)': (() => {
        const generator = new PushSetupGenerator()
        const windows: CommunicationWindow[] = [
          {
            start_time: new Date('2024-01-01T02:00:00'),
            end_time: new Date('2024-01-01T03:00:00')
          }
        ]
        
        return generator.generateSetRequest('0-0:25.9.2.255', 4, windows) + '\n\n' +
               generator.generateSetRequest('0-0:25.9.2.255', 7, {
                 min: 300,    // 5 minutes
                 exponent: 2, // x4 à chaque retry
                 max: 3600    // Max 1 heure
               })
      })(),
      
      'Push Setup - LoRaWAN Configuration': (() => {
        const generator = new PushSetupGenerator()
        const destination: SendDestinationAndMethod = {
          transport_service: TransportServiceType.UNRELIABLE_COAP,
          destination: 'coap://lorawan.server.com:5683/push',
          message: MessageType.A_XDR_ENCODED_XDLMS_APDU
        }
        
        return generator.generateSetRequest('0-0:25.9.3.255', 3, destination) + '\n\n' +
               generator.generateSetRequest('0-0:25.9.3.255', 5, 300) + '\n\n' + // 5 min random
               generator.generateSetRequest('0-0:25.9.3.255', 6, 2) // 2 retries max
      })()
    }
  }
}

// ===============================================
// EXPORT
// ===============================================

export default PushSetupGenerator
