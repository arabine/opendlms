/**
 * Générateur XML DLMS/COSEM - Utilitaires pour génération XML conforme au schéma simple
 * 
 * @fileoverview Générateur central pour toutes les requêtes DLMS/COSEM
 * @version 1.0.0
 * @author DLMS COSEM Generator
 */

import {
  ClassId,
  LogicalName,
  ObisCode,
  CosemObjectDescriptor,
  DlmsValue,
  DlmsDataType,
  DlmsDateTime,
  DlmsDate,
  DlmsTime,
  DLMS_TYPE_NAMES,
  DlmsError,
  DlmsErrorCode
} from '../types/common'

// ===============================================
// CLASSE PRINCIPALE - GÉNÉRATEUR XML
// ===============================================

/**
 * Générateur XML DLMS/COSEM conforme au schéma simple
 * Basé sur dlms_xml_schema_simple.xml du projet
 */
export class DlmsXmlGenerator {
  
  // ===============================================
  // MÉTHODES DE CONVERSION OBIS
  // ===============================================
  
  /**
   * Convertit un code OBIS au format hexadécimal
   * @param obisCode Format "A-B:C.D.E.F" (ex: "1-0:1.8.1.255")
   * @returns Format hex "AABBCCDDEEFF" (ex: "0100010801FF")
   */
  static obisToHex(obisCode: ObisCode): LogicalName {
    // Format standard OBIS: A-B:C.D.E.F
    const standardMatch = obisCode.match(/(\d+)-(\d+):(\d+)\.(\d+)\.(\d+)\.(\d+)/)
    if (standardMatch) {
      const [, a, b, c, d, e, f] = standardMatch
      return [a, b, c, d, e, f]
        .map(part => {
          const num = parseInt(part)
          if (num > 255) {
            throw new DlmsError(
              `Valeur OBIS trop grande: ${part} > 255`,
              DlmsErrorCode.TYPE_UNMATCHED
            )
          }
          return num.toString(16).padStart(2, '0').toUpperCase()
        })
        .join('')
    }
    
    // Format alternatif: A.B.C.D.E.F
    const dotMatch = obisCode.match(/(\d+)\.(\d+)\.(\d+)\.(\d+)\.(\d+)\.(\d+)/)
    if (dotMatch) {
      const [, a, b, c, d, e, f] = dotMatch
      return [a, b, c, d, e, f]
        .map(part => parseInt(part).toString(16).padStart(2, '0').toUpperCase())
        .join('')
    }
    
    // Si déjà en format hex
    if (/^[0-9A-Fa-f]{12}$/.test(obisCode)) {
      return obisCode.toUpperCase()
    }
    
    throw new DlmsError(
      `Format OBIS invalide: ${obisCode}. Attendu: "A-B:C.D.E.F" ou "AABBCCDDEEFF"`,
      DlmsErrorCode.TYPE_UNMATCHED
    )
  }

  /**
   * Convertit un nom logique hex vers code OBIS lisible
   * @param logicalName Format hex "AABBCCDDEEFF"
   * @returns Format "A-B:C.D.E.F"
   */
  static hexToObis(logicalName: LogicalName): ObisCode {
    if (!/^[0-9A-Fa-f]{12}$/.test(logicalName)) {
      throw new DlmsError(
        `Format LogicalName invalide: ${logicalName}. Attendu: 12 caractères hex`,
        DlmsErrorCode.TYPE_UNMATCHED
      )
    }
    
    const parts = []
    for (let i = 0; i < 12; i += 2) {
      parts.push(parseInt(logicalName.substr(i, 2), 16))
    }
    
    return `${parts[0]}-${parts[1]}:${parts[2]}.${parts[3]}.${parts[4]}.${parts[5]}`
  }

  // ===============================================
  // MÉTHODES DE CONVERSION NUMÉRIQUE
  // ===============================================

  /**
   * Convertit un nombre en hexadécimal avec padding
   * @param value Valeur numérique
   * @param bytes Nombre d'octets (détermine le padding)
   * @returns Chaîne hexadécimale uppercase
   */
  static toHex(value: string | number, bytes: number = 1): string {
    const num = typeof value === 'string' ? parseInt(value) : value
    
    if (isNaN(num)) {
      throw new DlmsError(
        `Valeur numérique invalide: ${value}`,
        DlmsErrorCode.TYPE_UNMATCHED
      )
    }
    
    const maxValue = Math.pow(256, bytes) - 1
    if (num < 0 || num > maxValue) {
      throw new DlmsError(
        `Valeur ${num} hors limites pour ${bytes} octets (0-${maxValue})`,
        DlmsErrorCode.TYPE_UNMATCHED
      )
    }
    
    return num.toString(16).padStart(bytes * 2, '0').toUpperCase()
  }

  // ===============================================
  // GÉNÉRATION SETREQUEST PRINCIPAL
  // ===============================================

  /**
   * Génère un SetRequest XML complet conforme au schéma simple
   * @param descriptor Descripteur de l'objet COSEM
   * @param value Valeur à écrire
   * @param invokeId ID d'invocation (défaut: 1)
   * @returns XML SetRequest complet
   */
  static generateSetRequest(
    descriptor: CosemObjectDescriptor,
    value: DlmsValue,
    invokeId: number = 1
  ): string {
    const classIdHex = this.toHex(descriptor.class_id, 2)
    const instanceIdHex = descriptor.logical_name
    const attributeIdHex = this.toHex(descriptor.attribute_index, 1)
    const invokeIdHex = this.toHex(invokeId, 1)
    
    const valueXml = this.generateValueXml(value)
    
    return `<SetRequest>
  <SetRequestNormal>
    <InvokeIdAndPriority Value="${invokeIdHex}"/>
    <AttributeDescriptor>
      <ClassId Value="${classIdHex}"/>
      <InstanceId Value="${instanceIdHex}"/>
      <AttributeId Value="${attributeIdHex}"/>
    </AttributeDescriptor>
    <Value>
      ${valueXml}
    </Value>
  </SetRequestNormal>
</SetRequest>`
  }

  /**
   * Génère un GetRequest XML complet
   * @param descriptor Descripteur de l'objet COSEM
   * @param invokeId ID d'invocation (défaut: 1)
   * @returns XML GetRequest complet
   */
  static generateGetRequest(
    descriptor: CosemObjectDescriptor,
    invokeId: number = 1
  ): string {
    const classIdHex = this.toHex(descriptor.class_id, 2)
    const instanceIdHex = descriptor.logical_name
    const attributeIdHex = this.toHex(descriptor.attribute_index, 1)
    const invokeIdHex = this.toHex(invokeId, 1)
    
    return `<GetRequest>
  <GetRequestNormal>
    <InvokeIdAndPriority Value="${invokeIdHex}"/>
    <AttributeDescriptor>
      <ClassId Value="${classIdHex}"/>
      <InstanceId Value="${instanceIdHex}"/>
      <AttributeId Value="${attributeIdHex}"/>
    </AttributeDescriptor>
  </GetRequestNormal>
</GetRequest>`
  }

  // ===============================================
  // GÉNÉRATION VALEURS XML
  // ===============================================

  /**
   * Génère l'élément Value XML selon le type DLMS
   * @param value Valeur DLMS avec type et métadonnées
   * @returns Fragment XML pour la valeur
   */
  static generateValueXml(value: DlmsValue): string {
    // Si une valeur hex pré-calculée existe, l'utiliser
    if (value.hexValue) {
      return this.generateTypedValueXml(value.type, value.hexValue)
    }

    // Génération selon le type
    switch (value.type) {
      case DlmsDataType.NULL_DATA:
        return '<OctetString Value="00"/>'
        
      case DlmsDataType.BOOLEAN:
        const boolHex = value.value ? '01' : '00'
        return `<Boolean Value="${boolHex}"/>`
        
      case DlmsDataType.UNSIGNED:
        return `<Unsigned Value="${this.toHex(value.value, 1)}"/>`
        
      case DlmsDataType.LONG_UNSIGNED:
        return `<LongUnsigned Value="${this.toHex(value.value, 2)}"/>`
        
      case DlmsDataType.DOUBLE_LONG_UNSIGNED:
        return `<DoubleLongUnsigned Value="${this.toHex(value.value, 4)}"/>`
        
      case DlmsDataType.INTEGER:
        return `<Integer Value="${this.toSignedHex(value.value, 1)}"/>`
        
      case DlmsDataType.LONG:
        return `<Long Value="${this.toSignedHex(value.value, 2)}"/>`
        
      case DlmsDataType.DOUBLE_LONG:
        return `<DoubleLong Value="${this.toSignedHex(value.value, 4)}"/>`
        
      case DlmsDataType.OCTET_STRING:
        return `<OctetString Value="${this.valueToHex(value.value)}"/>`
        
      case DlmsDataType.VISIBLE_STRING:
        const visibleHex = Buffer.from(value.value.toString(), 'ascii').toString('hex').toUpperCase()
        return `<VisibleString Value="${visibleHex}"/>`
        
      case DlmsDataType.UTF8_STRING:
        const utf8Hex = Buffer.from(value.value.toString(), 'utf8').toString('hex').toUpperCase()
        return `<Utf8String Value="${utf8Hex}"/>`
        
      case DlmsDataType.ENUM:
        return `<Enum Value="${this.toHex(value.value, 1)}"/>`
        
      case DlmsDataType.BIT_STRING:
        return `<BitString Value="${this.bitStringToHex(value.value)}"/>`
        
      case DlmsDataType.DATE_TIME:
        return `<DateTime Value="${this.dateTimeToHex(value.value)}"/>`
        
      case DlmsDataType.DATE:
        return `<Date Value="${this.dateToHex(value.value)}"/>`
        
      case DlmsDataType.TIME:
        return `<Time Value="${this.timeToHex(value.value)}"/>`
        
      case DlmsDataType.FLOAT32:
        return `<Float32 Value="${this.float32ToHex(value.value)}"/>`
        
      case DlmsDataType.FLOAT64:
        return `<Float64 Value="${this.float64ToHex(value.value)}"/>`
        
      case DlmsDataType.ARRAY:
        return this.generateArrayXml(value.value)
        
      case DlmsDataType.STRUCTURE:
        return this.generateStructureXml(value.value)
        
      default:
        return `<OctetString Value="00"/>
        <!-- Type non supporté: ${value.type} -->`
    }
  }

  /**
   * Génère XML pour un type avec valeur hex pré-calculée
   */
  private static generateTypedValueXml(type: DlmsDataType, hexValue: string): string {
    const typeName = DLMS_TYPE_NAMES[type] || 'OctetString'
    return `<${typeName} Value="${hexValue}"/>`
  }

  // ===============================================
  // GÉNÉRATION TYPES COMPLEXES
  // ===============================================

  /**
   * Génère XML pour un Array DLMS
   */
  private static generateArrayXml(items: DlmsValue[]): string {
    const qtyHex = this.toHex(items.length, 1)
    const elementsXml = items
      .map(item => this.generateValueXml(item))
      .join('\n      ')
    
    return `<Array Qty="${qtyHex}">
      ${elementsXml}
    </Array>`
  }

  /**
   * Génère XML pour une Structure DLMS
   */
  private static generateStructureXml(fields: DlmsValue[]): string {
    const qtyHex = this.toHex(fields.length, 1)
    const fieldsXml = fields
      .map(field => this.generateValueXml(field))
      .join('\n      ')
    
    return `<Structure Qty="${qtyHex}">
      ${fieldsXml}
    </Structure>`
  }

  // ===============================================
  // CONVERSIONS SPÉCIALISÉES
  // ===============================================

  /**
   * Convertit un nombre signé en hex (complément à 2)
   */
  private static toSignedHex(value: number, bytes: number): string {
    const maxPositive = Math.pow(2, bytes * 8 - 1) - 1
    const minNegative = -Math.pow(2, bytes * 8 - 1)
    
    if (value < minNegative || value > maxPositive) {
      throw new DlmsError(
        `Valeur signée ${value} hors limites pour ${bytes} octets`,
        DlmsErrorCode.TYPE_UNMATCHED
      )
    }
    
    // Complément à 2 pour valeurs négatives
    const unsigned = value < 0 ? Math.pow(256, bytes) + value : value
    return unsigned.toString(16).padStart(bytes * 2, '0').toUpperCase()
  }

  /**
   * Convertit une valeur en hex selon son type
   */
  private static valueToHex(value: any): string {
    if (typeof value === 'string') {
      // Si déjà en hex
      if (/^[0-9A-Fa-f]*$/.test(value)) {
        return value.toUpperCase()
      }
      // Sinon convertir depuis UTF-8
      return Buffer.from(value, 'utf8').toString('hex').toUpperCase()
    }
    
    if (typeof value === 'number') {
      return this.toHex(value, Math.ceil(Math.log2(value + 1) / 8) || 1)
    }
    
    if (Buffer.isBuffer(value)) {
      return value.toString('hex').toUpperCase()
    }
    
    if (Array.isArray(value)) {
      return value.map(v => this.toHex(v, 1)).join('')
    }
    
    return '00'
  }

  /**
   * Convertit un bit string en hex
   */
  private static bitStringToHex(bitString: string | boolean[]): string {
    if (typeof bitString === 'string') {
      // Format "10110010" -> hex
      const padding = (8 - (bitString.length % 8)) % 8
      const paddedBits = bitString + '0'.repeat(padding)
      
      let hex = ''
      for (let i = 0; i < paddedBits.length; i += 8) {
        const byte = paddedBits.substr(i, 8)
        hex += parseInt(byte, 2).toString(16).padStart(2, '0').toUpperCase()
      }
      return hex
    }
    
    if (Array.isArray(bitString)) {
      const bits = bitString.map(b => b ? '1' : '0').join('')
      return this.bitStringToHex(bits)
    }
    
    return '00'
  }

  /**
   * Convertit DateTime DLMS en hex (12 octets)
   */
  private static dateTimeToHex(dateTime: Date | DlmsDateTime): string {
    if (dateTime instanceof Date) {
      return this.formatJsDateToHex(dateTime)
    }
    
    const dt = dateTime as DlmsDateTime
    const parts = [
      this.toHex(dt.year, 2),
      this.toHex(dt.month, 1),
      this.toHex(dt.day, 1),
      this.toHex(dt.dayOfWeek || this.getDayOfWeek(dt.year, dt.month, dt.day), 1),
      this.toHex(dt.hour, 1),
      this.toHex(dt.minute, 1),
      this.toHex(dt.second, 1),
      this.toHex(dt.hundredths || 0, 1),
      this.toSignedHex(dt.deviation || 0, 2),
      this.toHex(dt.clockStatus || 0xFF, 1)
    ]
    
    return parts.join('')
  }

  /**
   * Convertit Date DLMS en hex (5 octets)
   */
  private static dateToHex(date: Date | DlmsDate): string {
    if (date instanceof Date) {
      const parts = [
        this.toHex(date.getFullYear(), 2),
        this.toHex(date.getMonth() + 1, 1),
        this.toHex(date.getDate(), 1),
        this.toHex(this.getDayOfWeek(date.getFullYear(), date.getMonth() + 1, date.getDate()), 1)
      ]
      return parts.join('')
    }
    
    const d = date as DlmsDate
    return [
      this.toHex(d.year, 2),
      this.toHex(d.month, 1),
      this.toHex(d.day, 1),
      this.toHex(d.dayOfWeek, 1)
    ].join('')
  }

  /**
   * Convertit Time DLMS en hex (4 octets)
   */
  private static timeToHex(time: Date | DlmsTime): string {
    if (time instanceof Date) {
      return [
        this.toHex(time.getHours(), 1),
        this.toHex(time.getMinutes(), 1),
        this.toHex(time.getSeconds(), 1),
        this.toHex(Math.floor(time.getMilliseconds() / 10), 1)
      ].join('')
    }
    
    const t = time as DlmsTime
    return [
      this.toHex(t.hour, 1),
      this.toHex(t.minute, 1),
      this.toHex(t.second, 1),
      this.toHex(t.hundredths, 1)
    ].join('')
  }

  /**
   * Convertit Float32 en hex (IEEE 754)
   */
  private static float32ToHex(value: number): string {
    const buffer = Buffer.allocUnsafe(4)
    buffer.writeFloatBE(value, 0)
    return buffer.toString('hex').toUpperCase()
  }

  /**
   * Convertit Float64 en hex (IEEE 754)
   */
  private static float64ToHex(value: number): string {
    const buffer = Buffer.allocUnsafe(8)
    buffer.writeDoubleBE(value, 0)
    return buffer.toString('hex').toUpperCase()
  }

  // ===============================================
  // UTILITAIRES DATES
  // ===============================================

  /**
   * Convertit Date JavaScript vers format DLMS hex
   */
  private static formatJsDateToHex(date: Date): string {
    const year = date.getFullYear()
    const month = date.getMonth() + 1
    const day = date.getDate()
    const dayOfWeek = this.getDayOfWeek(year, month, day)
    const hour = date.getHours()
    const minute = date.getMinutes()
    const second = date.getSeconds()
    const hundredths = Math.floor(date.getMilliseconds() / 10)
    
    return [
      this.toHex(year, 2),
      this.toHex(month, 1),
      this.toHex(day, 1),
      this.toHex(dayOfWeek, 1),
      this.toHex(hour, 1),
      this.toHex(minute, 1),
      this.toHex(second, 1),
      this.toHex(hundredths, 1),
      'FFFF', // Deviation non spécifiée
      'FF'    // Clock status non spécifié
    ].join('')
  }

  /**
   * Calcule le jour de la semaine (1=lundi, 7=dimanche)
   */
  private static getDayOfWeek(year: number, month: number, day: number): number {
    const date = new Date(year, month - 1, day)
    const jsDay = date.getDay() // 0=dimanche, 1=lundi...
    return jsDay === 0 ? 7 : jsDay // Conversion vers format DLMS
  }

  // ===============================================
  // VALIDATION ET HELPERS
  // ===============================================

  /**
   * Valide un descripteur COSEM
   */
  static validateDescriptor(descriptor: CosemObjectDescriptor): void {
    if (!descriptor.class_id || descriptor.class_id < 0 || descriptor.class_id > 65535) {
      throw new DlmsError(
        `Class ID invalide: ${descriptor.class_id}`,
        DlmsErrorCode.OBJECT_CLASS_INCONSISTENT
      )
    }
    
    if (!descriptor.logical_name || !/^[0-9A-Fa-f]{12}$/.test(descriptor.logical_name)) {
      throw new DlmsError(
        `Logical name invalide: ${descriptor.logical_name}`,
        DlmsErrorCode.OBJECT_UNDEFINED
      )
    }
    
    if (!descriptor.attribute_index || descriptor.attribute_index < 1 || descriptor.attribute_index > 255) {
      throw new DlmsError(
        `Attribute index invalide: ${descriptor.attribute_index}`,
        DlmsErrorCode.TYPE_UNMATCHED
      )
    }
  }

  /**
   * Crée un descripteur COSEM à partir d'un code OBIS
   */
  static createDescriptor(
    classId: ClassId,
    obisCode: ObisCode,
    attributeIndex: number
  ): CosemObjectDescriptor {
    const descriptor: CosemObjectDescriptor = {
      class_id: classId,
      logical_name: this.obisToHex(obisCode),
      attribute_index: attributeIndex
    }
    
    this.validateDescriptor(descriptor)
    return descriptor
  }

  /**
   * Crée une valeur DLMS simple avec auto-détection de type
   */
  static createSimpleValue(value: any): DlmsValue {
    if (value === null || value === undefined) {
      return { type: DlmsDataType.NULL_DATA, value: null }
    }
    
    if (typeof value === 'boolean') {
      return { type: DlmsDataType.BOOLEAN, value }
    }
    
    if (typeof value === 'number') {
      if (Number.isInteger(value)) {
        if (value >= 0 && value <= 255) {
          return { type: DlmsDataType.UNSIGNED, value }
        }
        if (value >= 0 && value <= 65535) {
          return { type: DlmsDataType.LONG_UNSIGNED, value }
        }
        if (value >= 0 && value <= 4294967295) {
          return { type: DlmsDataType.DOUBLE_LONG_UNSIGNED, value }
        }
        // Valeurs signées
        if (value >= -128 && value <= 127) {
          return { type: DlmsDataType.INTEGER, value }
        }
        if (value >= -32768 && value <= 32767) {
          return { type: DlmsDataType.LONG, value }
        }
        return { type: DlmsDataType.DOUBLE_LONG, value }
      } else {
        return { type: DlmsDataType.FLOAT64, value }
      }
    }
    
    if (typeof value === 'string') {
      // Tenter de détecter format hex
      if (/^[0-9A-Fa-f]*$/.test(value) && value.length % 2 === 0) {
        return { type: DlmsDataType.OCTET_STRING, value }
      }
      return { type: DlmsDataType.VISIBLE_STRING, value }
    }
    
    if (value instanceof Date) {
      return { type: DlmsDataType.DATE_TIME, value }
    }
    
    if (Array.isArray(value)) {
      const elements = value.map(v => this.createSimpleValue(v))
      return { type: DlmsDataType.ARRAY, value: elements }
    }
    
    // Objet -> Structure
    if (typeof value === 'object') {
      const fields = Object.values(value).map(v => this.createSimpleValue(v))
      return { type: DlmsDataType.STRUCTURE, value: fields }
    }
    
    return { type: DlmsDataType.OCTET_STRING, value: value.toString() }
  }
}

// ===============================================
// EXPORT PAR DÉFAUT
// ===============================================

export default DlmsXmlGenerator