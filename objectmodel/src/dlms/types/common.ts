/**
 * Types communs DLMS/COSEM - Factorisation des types utilisés partout
 * 
 * @fileoverview Types de base réutilisés dans toutes les classes COSEM
 * @version 1.0.0
 * @author DLMS COSEM Generator
 */

// ===============================================
// TYPES DE BASE COMMUNS - FACTORISATION
// ===============================================

/** Class ID DLMS/COSEM (0-65535) */
export type ClassId = number

/** Logical Name au format hexadécimal OBIS (ex: "0000010000FF") */
export type LogicalName = string

/** Index d'attribut (1-255) */
export type AttributeIndex = number

/** Code OBIS lisible (ex: "0-0:1.0.0.255") */
export type ObisCode = string

/** Version de classe d'interface (0-255) */
export type Version = number

// ===============================================
// DESCRIPTEUR D'OBJET COSEM STANDARD
// ===============================================

/** Descripteur complet d'un objet COSEM */
export interface CosemObjectDescriptor {
  /** Identifiant de classe COSEM */
  class_id: ClassId
  /** Nom logique au format hex */
  logical_name: LogicalName
  /** Index de l'attribut */
  attribute_index: AttributeIndex
  /** Version de la classe (optionnel) */
  version?: Version
}

// ===============================================
// TYPES DE DONNÉES DLMS/COSEM
// ===============================================

/** 
 * Types de données DLMS/COSEM selon Blue Book Ed. 16 Part 2, Table 3
 * Tags ASN.1 inclus pour référence
 */
export enum DlmsDataType {
  // Types simples
  NULL_DATA = 0,              // [0]
  BOOLEAN = 3,                // [3]
  BIT_STRING = 4,             // [4]
  DOUBLE_LONG = 5,            // [5] Integer32
  DOUBLE_LONG_UNSIGNED = 6,   // [6] Unsigned32
  OCTET_STRING = 9,           // [9]
  VISIBLE_STRING = 10,        // [10] ASCII
  UTF8_STRING = 12,           // [12]
  BCD = 13,                   // [13]
  INTEGER = 15,               // [15] Integer8
  LONG = 16,                  // [16] Integer16
  UNSIGNED = 17,              // [17] Unsigned8
  LONG_UNSIGNED = 18,         // [18] Unsigned16
  LONG64 = 20,                // [20] Integer64
  LONG64_UNSIGNED = 21,       // [21] Unsigned64
  ENUM = 22,                  // [22]
  FLOAT32 = 23,               // [23]
  FLOAT64 = 24,               // [24]
  DATE_TIME = 25,             // [25] 12 bytes
  DATE = 26,                  // [26] 5 bytes
  TIME = 27,                  // [27] 4 bytes
  
  // Types complexes
  ARRAY = 1,                  // [1]
  STRUCTURE = 2,              // [2]
  COMPACT_ARRAY = 19,         // [19]
  
  // Types delta (pour profils)
  DELTA_INTEGER = 28,         // [28]
  DELTA_LONG = 29,            // [29]
  DELTA_DOUBLE_LONG = 30,     // [30]
  DELTA_UNSIGNED = 31,        // [31]
  DELTA_LONG_UNSIGNED = 32,   // [32]
  DELTA_DOUBLE_LONG_UNSIGNED = 33  // [33]
}

/** Valeur de données DLMS avec métadonnées de type */
export interface DlmsValue {
  /** Type de données selon énumération DLMS */
  type: DlmsDataType
  /** Valeur effective (type natif JavaScript) */
  value: any
  /** Valeur hexadécimale pré-calculée (optionnel) */
  hexValue?: string
  /** Métadonnées additionnelles */
  metadata?: {
    /** Taille en octets */
    byteSize?: number
    /** Unité si applicable */
    unit?: number
    /** Scalaire si applicable */
    scaler?: number
  }
}

// ===============================================
// DROITS D'ACCÈS DLMS
// ===============================================

/** 
 * Modes d'accès aux attributs COSEM
 * Selon Blue Book Ed. 16 Part 2, Association classes
 */
export enum AccessMode {
  NO_ACCESS = 0,
  READ_ONLY = 1,
  WRITE_ONLY = 2,
  READ_AND_WRITE = 3,
  AUTHENTICATED_READ_ONLY = 4,
  AUTHENTICATED_WRITE_ONLY = 5,
  AUTHENTICATED_READ_AND_WRITE = 6
}

/** Droits d'accès pour un attribut */
export interface AttributeAccess {
  /** Index de l'attribut */
  attribute_id: number
  /** Mode d'accès */
  access_mode: AccessMode
  /** Sélecteurs d'accès autorisés */
  access_selectors?: number[]
}

/** Droits d'accès pour une méthode */
export interface MethodAccess {
  /** ID de la méthode */
  method_id: number
  /** Accès autorisé */
  access_mode: 0 | 1 | 2  // no_access | access | authenticated_access
}

// ===============================================
// TYPES POUR SELECTIVE ACCESS
// ===============================================

/** Paramètres d'accès sélectif */
export interface SelectiveAccessDescriptor {
  /** Sélecteur d'accès */
  access_selector: number
  /** Paramètres spécifiques au sélecteur */
  access_parameters: DlmsValue
}

// ===============================================
// TYPES TEMPORELS DLMS
// ===============================================

/** Date/Heure DLMS (12 octets) */
export interface DlmsDateTime {
  year: number        // 2 bytes
  month: number       // 1 byte (1-12)
  day: number         // 1 byte (1-31)
  dayOfWeek: number   // 1 byte (1-7, 1=lundi)
  hour: number        // 1 byte (0-23)
  minute: number      // 1 byte (0-59)
  second: number      // 1 byte (0-59)
  hundredths: number  // 1 byte (0-99)
  deviation?: number  // 2 bytes (minutes from GMT, optionnel)
  clockStatus?: number // 1 byte (status, optionnel)
}

/** Date DLMS (5 octets) */
export interface DlmsDate {
  year: number        // 2 bytes
  month: number       // 1 byte
  day: number         // 1 byte
  dayOfWeek: number   // 1 byte
}

/** Heure DLMS (4 octets) */
export interface DlmsTime {
  hour: number        // 1 byte
  minute: number      // 1 byte
  second: number      // 1 byte
  hundredths: number  // 1 byte
}

// ===============================================
// TYPES POUR STRUCTURE ET ARRAY
// ===============================================

/** Élément de structure DLMS */
export interface StructureField {
  /** Nom du champ (pour documentation) */
  name?: string
  /** Valeur du champ */
  value: DlmsValue
}

/** Array DLMS typé */
export interface DlmsArray {
  /** Nombre d'éléments */
  qty: number
  /** Éléments du tableau */
  elements: DlmsValue[]
}

// ===============================================
// MÉTADONNÉES DE CLASSE COSEM
// ===============================================

/** Informations sur une classe COSEM */
export interface CosemClassInfo {
  /** Identifiant de classe */
  classId: ClassId
  /** Version supportée */
  version: Version
  /** Nom de la classe */
  name: string
  /** Description courte */
  description: string
  /** Attributs disponibles */
  attributes: AttributeInfo[]
  /** Méthodes disponibles */
  methods: MethodInfo[]
}

/** Informations sur un attribut */
export interface AttributeInfo {
  /** Index de l'attribut */
  index: AttributeIndex
  /** Nom de l'attribut */
  name: string
  /** Type de données */
  dataType: string
  /** Accès en lecture/écriture */
  accessMode: AccessMode
  /** Obligatoire/Optionnel */
  mandatory: boolean
  /** Statique/Dynamique */
  static: boolean
  /** Description */
  description?: string
}

/** Informations sur une méthode */
export interface MethodInfo {
  /** ID de la méthode */
  id: number
  /** Nom de la méthode */
  name: string
  /** Obligatoire/Optionnel */
  mandatory: boolean
  /** Type de paramètre */
  parameterType?: string
  /** Description */
  description?: string
}

// ===============================================
// TYPES D'ERREUR DLMS
// ===============================================

/** Codes d'erreur DLMS standard */
export enum DlmsErrorCode {
  OK = 0,
  HARDWARE_FAULT = 1,
  TEMPORARY_FAILURE = 2,
  READ_WRITE_DENIED = 3,
  OBJECT_UNDEFINED = 4,
  OBJECT_CLASS_INCONSISTENT = 5,
  OBJECT_UNAVAILABLE = 6,
  TYPE_UNMATCHED = 7,
  SCOPE_OF_ACCESS_VIOLATED = 8,
  DATA_BLOCK_UNAVAILABLE = 9,
  LONG_GET_ABORTED = 10,
  NO_LONG_GET_IN_PROGRESS = 11,
  LONG_SET_ABORTED = 12,
  NO_LONG_SET_IN_PROGRESS = 13,
  DATA_BLOCK_NUMBER_INVALID = 14,
  OTHER_REASON = 250
}

/** Exception DLMS avec contexte */
export class DlmsError extends Error {
  constructor(
    message: string,
    public readonly errorCode: DlmsErrorCode,
    public readonly context?: {
      classId?: ClassId
      logicalName?: LogicalName
      attributeIndex?: AttributeIndex
    }
  ) {
    super(message)
    this.name = 'DlmsError'
  }
}

// ===============================================
// CONSTANTES UTILES
// ===============================================

/** Tailles des types de données en octets */
export const DLMS_TYPE_SIZES: Record<DlmsDataType, number> = {
  [DlmsDataType.NULL_DATA]: 0,
  [DlmsDataType.BOOLEAN]: 1,
  [DlmsDataType.BIT_STRING]: 0, // Variable
  [DlmsDataType.INTEGER]: 1,
  [DlmsDataType.UNSIGNED]: 1,
  [DlmsDataType.LONG]: 2,
  [DlmsDataType.LONG_UNSIGNED]: 2,
  [DlmsDataType.DOUBLE_LONG]: 4,
  [DlmsDataType.DOUBLE_LONG_UNSIGNED]: 4,
  [DlmsDataType.LONG64]: 8,
  [DlmsDataType.LONG64_UNSIGNED]: 8,
  [DlmsDataType.OCTET_STRING]: 0, // Variable
  [DlmsDataType.VISIBLE_STRING]: 0, // Variable
  [DlmsDataType.UTF8_STRING]: 0, // Variable
  [DlmsDataType.BCD]: 0, // Variable
  [DlmsDataType.ENUM]: 1,
  [DlmsDataType.FLOAT32]: 4,
  [DlmsDataType.FLOAT64]: 8,
  [DlmsDataType.DATE_TIME]: 12,
  [DlmsDataType.DATE]: 5,
  [DlmsDataType.TIME]: 4,
  [DlmsDataType.ARRAY]: 0, // Variable
  [DlmsDataType.STRUCTURE]: 0, // Variable
  [DlmsDataType.COMPACT_ARRAY]: 0, // Variable
  [DlmsDataType.DELTA_INTEGER]: 1,
  [DlmsDataType.DELTA_LONG]: 2,
  [DlmsDataType.DELTA_DOUBLE_LONG]: 4,
  [DlmsDataType.DELTA_UNSIGNED]: 1,
  [DlmsDataType.DELTA_LONG_UNSIGNED]: 2,
  [DlmsDataType.DELTA_DOUBLE_LONG_UNSIGNED]: 4
}

/** Noms des types DLMS pour génération XML */
export const DLMS_TYPE_NAMES: Record<DlmsDataType, string> = {
  [DlmsDataType.NULL_DATA]: 'NullData',
  [DlmsDataType.BOOLEAN]: 'Boolean',
  [DlmsDataType.BIT_STRING]: 'BitString',
  [DlmsDataType.INTEGER]: 'Integer',
  [DlmsDataType.UNSIGNED]: 'Unsigned',
  [DlmsDataType.LONG]: 'Long',
  [DlmsDataType.LONG_UNSIGNED]: 'LongUnsigned',
  [DlmsDataType.DOUBLE_LONG]: 'DoubleLong',
  [DlmsDataType.DOUBLE_LONG_UNSIGNED]: 'DoubleLongUnsigned',
  [DlmsDataType.LONG64]: 'Long64',
  [DlmsDataType.LONG64_UNSIGNED]: 'Long64Unsigned',
  [DlmsDataType.OCTET_STRING]: 'OctetString',
  [DlmsDataType.VISIBLE_STRING]: 'VisibleString',
  [DlmsDataType.UTF8_STRING]: 'Utf8String',
  [DlmsDataType.BCD]: 'Bcd',
  [DlmsDataType.ENUM]: 'Enum',
  [DlmsDataType.FLOAT32]: 'Float32',
  [DlmsDataType.FLOAT64]: 'Float64',
  [DlmsDataType.DATE_TIME]: 'DateTime',
  [DlmsDataType.DATE]: 'Date',
  [DlmsDataType.TIME]: 'Time',
  [DlmsDataType.ARRAY]: 'Array',
  [DlmsDataType.STRUCTURE]: 'Structure',
  [DlmsDataType.COMPACT_ARRAY]: 'CompactArray',
  [DlmsDataType.DELTA_INTEGER]: 'Integer',
  [DlmsDataType.DELTA_LONG]: 'Long',
  [DlmsDataType.DELTA_DOUBLE_LONG]: 'DoubleLong',
  [DlmsDataType.DELTA_UNSIGNED]: 'Unsigned',
  [DlmsDataType.DELTA_LONG_UNSIGNED]: 'LongUnsigned',
  [DlmsDataType.DELTA_DOUBLE_LONG_UNSIGNED]: 'DoubleLongUnsigned'
}

// ===============================================
// EXPORT PAR DÉFAUT
// ===============================================

export default {
  DlmsDataType,
  AccessMode,
  DlmsErrorCode,
  DLMS_TYPE_SIZES,
  DLMS_TYPE_NAMES
}