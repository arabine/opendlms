import type { DlmsObject, DlmsAttribute } from '@/types'

class XmlService {
  /**
   * Convertit un code OBIS au format hexadécimal
   * Ex: "1-0:1.8.1.255" → "0100010801FF"
   */
  private obisToHex(obisCode: string): string {
    // Format standard OBIS: A-B:C.D.E.F
    const parts = obisCode.match(/(\d+)-(\d+):(\d+)\.(\d+)\.(\d+)\.(\d+)/)
    if (parts) {
      const [, a, b, c, d, e, f] = parts
      return [a, b, c, d, e, f]
        .map(part => parseInt(part).toString(16).padStart(2, '0').toUpperCase())
        .join('')
    }
    // Fallback: essayer de parser comme "A.B.C.D.E.F"
    const dotParts = obisCode.split(/[.-:]/).filter(p => p)
    if (dotParts.length >= 6) {
      return dotParts.slice(0, 6)
        .map(part => parseInt(part).toString(16).padStart(2, '0').toUpperCase())
        .join('')
    }
    // Si le parsing échoue, retourner une valeur par défaut
    return '0000000000FF'
  }

  /**
   * Convertit un nombre en hexadécimal avec padding
   */
  private toHex(value: string | number, bytes: number = 1): string {
    const num = typeof value === 'string' ? parseInt(value) : value
    return num.toString(16).padStart(bytes * 2, '0').toUpperCase()
  }

  generateSimpleGetXml(selectedObject: DlmsObject, selectedAttribute: DlmsAttribute): string {
    const obisHex = this.obisToHex(selectedObject.obisCode)
    const classIdHex = this.toHex(selectedObject.classId, 2)
    const attributeIdHex = this.toHex(selectedAttribute.number, 1)
    
    return `<GetRequest>
  <GetRequestNormal>
    <InvokeIdAndPriority Value="01"/>
    <AttributeDescriptor>
      <ClassId Value="${classIdHex}"/>
      <InstanceId Value="${obisHex}"/>
      <AttributeId Value="${attributeIdHex}"/>
    </AttributeDescriptor>
  </GetRequestNormal>
</GetRequest>`
  }

  generateStandardGetXml(selectedObject: DlmsObject, selectedAttribute: DlmsAttribute): string {
    return `<?xml version="1.0" encoding="UTF-8"?>
<get-request xmlns="http://www.dlms.com/COSEMpdu">
  <get-request-normal>
    <invoke-id-and-priority>01</invoke-id-and-priority>
    <cosem-attribute-descriptor>
      <class-id>${selectedObject.classId}</class-id>
      <instance-id>${selectedObject.obisCode}</instance-id>
      <attribute-id>${selectedAttribute.number}</attribute-id>
    </cosem-attribute-descriptor>
  </get-request-normal>
</get-request>`
  }

  generateSimpleSetXml(selectedObject: DlmsObject, selectedAttribute: DlmsAttribute): string {
    const obisHex = this.obisToHex(selectedObject.obisCode)
    const classIdHex = this.toHex(selectedObject.classId, 2)
    const attributeIdHex = this.toHex(selectedAttribute.number, 1)
    
    // Génération de la valeur selon le type d'attribut
    const valueXml = this.generateValueXml(selectedAttribute)
    
    return `<SetRequest>
  <SetRequestNormal>
    <InvokeIdAndPriority Value="01"/>
    <AttributeDescriptor>
      <ClassId Value="${classIdHex}"/>
      <InstanceId Value="${obisHex}"/>
      <AttributeId Value="${attributeIdHex}"/>
    </AttributeDescriptor>
    <Value>
      ${valueXml}
    </Value>
  </SetRequestNormal>
</SetRequest>`
  }

  generateStandardSetXml(selectedObject: DlmsObject, selectedAttribute: DlmsAttribute): string {
    return `<?xml version="1.0" encoding="UTF-8"?>
<set-request xmlns="http://www.dlms.com/COSEMpdu">
  <set-request-normal>
    <invoke-id-and-priority>01</invoke-id-and-priority>
    <cosem-attribute-descriptor>
      <class-id>${selectedObject.classId}</class-id>
      <instance-id>${selectedObject.obisCode}</instance-id>
      <attribute-id>${selectedAttribute.number}</attribute-id>
    </cosem-attribute-descriptor>
    <value>
      ${this.generateStandardValueXml(selectedAttribute)}
    </value>
  </set-request-normal>
</set-request>`
  }

  /**
   * Génère l'élément Value selon le schéma simple DLMS
   */
  private generateValueXml(attribute: DlmsAttribute): string {
    const type = attribute.type.toLowerCase()
    const defaultValue = attribute.defaultValue || ''
    
    // Mappage des types DLMS vers les éléments XML du schéma simple
    if (type.includes('octet_string') || type.includes('octetstring')) {
      const hexValue = defaultValue.replace(/[^0-9A-Fa-f]/g, '') || '00'
      return `<OctetString Value="${hexValue}"/>`
    }
    
    if (type.includes('unsigned') && !type.includes('long')) {
      const hexValue = this.toHex(defaultValue || '0', 1)
      return `<Unsigned Value="${hexValue}"/>`
    }
    
    if (type.includes('long_unsigned') || type.includes('longunsigned')) {
      const hexValue = this.toHex(defaultValue || '0', 2)
      return `<LongUnsigned Value="${hexValue}"/>`
    }
    
    if (type.includes('double_long_unsigned') || type.includes('doublelongunsigned')) {
      const hexValue = this.toHex(defaultValue || '0', 4)
      return `<DoubleLongUnsigned Value="${hexValue}"/>`
    }
    
    if (type.includes('integer') && !type.includes('long')) {
      const hexValue = this.toHex(defaultValue || '0', 1)
      return `<Integer Value="${hexValue}"/>`
    }
    
    if (type.includes('long') && type.includes('integer')) {
      const hexValue = this.toHex(defaultValue || '0', 2)
      return `<Long Value="${hexValue}"/>`
    }
    
    if (type.includes('boolean')) {
      const hexValue = defaultValue.toLowerCase() === 'true' ? '01' : '00'
      return `<Boolean Value="${hexValue}"/>`
    }
    
    if (type.includes('enum')) {
      const hexValue = this.toHex(defaultValue || '0', 1)
      return `<Enum Value="${hexValue}"/>`
    }
    
    if (type.includes('visible_string') || type.includes('visiblestring')) {
      const hexValue = Buffer.from(defaultValue || 'VALUE_TO_SET').toString('hex').toUpperCase()
      return `<VisibleString Value="${hexValue}"/>`
    }
    
    if (type.includes('bit_string') || type.includes('bitstring')) {
      return `<BitString Value="${defaultValue || '00'}"/>`
    }
    
    // Type par défaut : OctetString
    return `<OctetString Value="00"/>
    <!-- Type non reconnu: ${attribute.type} - Valeur par défaut: ${defaultValue} -->`
  }

  /**
   * Génère l'élément value selon le schéma standard DLMS
   */
  private generateStandardValueXml(attribute: DlmsAttribute): string {
    const type = attribute.type.toLowerCase()
    const defaultValue = attribute.defaultValue || ''
    
    if (type.includes('octet_string') || type.includes('octetstring')) {
      return `<octet-string>${defaultValue || '00'}</octet-string>`
    }
    
    if (type.includes('unsigned') && !type.includes('long')) {
      return `<unsigned>${parseInt(defaultValue || '0')}</unsigned>`
    }
    
    if (type.includes('long_unsigned') || type.includes('longunsigned')) {
      return `<long-unsigned>${parseInt(defaultValue || '0')}</long-unsigned>`
    }
    
    if (type.includes('double_long_unsigned') || type.includes('doublelongunsigned')) {
      return `<double-long-unsigned>${parseInt(defaultValue || '0')}</double-long-unsigned>`
    }
    
    if (type.includes('integer') && !type.includes('long')) {
      return `<integer>${parseInt(defaultValue || '0')}</integer>`
    }
    
    if (type.includes('long') && type.includes('integer')) {
      return `<long>${parseInt(defaultValue || '0')}</long>`
    }
    
    if (type.includes('boolean')) {
      return `<boolean>${defaultValue.toLowerCase() === 'true'}</boolean>`
    }
    
    if (type.includes('enum')) {
      return `<enum>${parseInt(defaultValue || '0')}</enum>`
    }
    
    if (type.includes('visible_string') || type.includes('visiblestring')) {
      return `<visible-string>${defaultValue || 'VALUE_TO_SET'}</visible-string>`
    }
    
    // Type par défaut
    return `<octet-string>00</octet-string>
    <!-- Type non reconnu: ${attribute.type} -->`
  }

  canSet(selectedAttribute: DlmsAttribute | null): boolean {
    if (!selectedAttribute || !selectedAttribute.accessRights) return false
    return selectedAttribute.accessRights.toLowerCase().includes('set')
  }
}

export const xmlService = new XmlService()