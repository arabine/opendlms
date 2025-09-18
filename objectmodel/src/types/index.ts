// DLMS/COSEM Types
export interface DlmsAttribute {
  number: number
  name: string
  type: string
  defaultValue?: string | null
  accessRights?: string | null
}

export interface DlmsObject {
  id: number
  name: string
  classId: number
  version: number
  obisCode: string
  attributes: DlmsAttribute[]
  expanded: boolean
  filteredAttributes?: DlmsAttribute[]
}

export interface FilteredObject extends DlmsObject {
  filteredAttributes: DlmsAttribute[]
}

export type XmlSchema = 'simple' | 'standard'
export type RequestType = 'get' | 'set'

// Excel parsing types
export type ExcelRow = (string | number | null | undefined)[]
export type ExcelData = ExcelRow[]

// Component props interfaces
export interface FileUploadProps {
  loading: boolean
}

export interface ErrorComponentProps {
  error: string
}

export interface SearchFilterProps {
  searchQuery: string
  filteredObjectsCount: number
  totalObjectsCount: number
  allExpanded: boolean
}

export interface ObjectItemProps {
  object: FilteredObject
  searchQuery: string
  selectedObject: DlmsObject | null
  selectedAttribute: DlmsAttribute | null
}

export interface AttributeItemProps {
  attribute: DlmsAttribute
  object: DlmsObject
  searchQuery: string
  isSelected: boolean
}

export interface SelectedAttributeInfoProps {
  selectedObject: DlmsObject
  selectedAttribute: DlmsAttribute
}

export interface SchemaSelectorProps {
  xmlSchema: XmlSchema
}

export interface RequestButtonsProps {
  canSet: boolean
}

export interface GeneratedXmlDisplayProps {
  generatedXml: string
  copySuccess: boolean
}

export interface XmlGenerationPanelProps {
  selectedObject: DlmsObject | null
  selectedAttribute: DlmsAttribute | null
}

export interface ObjectTreePanelProps {
  cosemObjects: DlmsObject[]
  searchQuery: string
  selectedObject: DlmsObject | null
  selectedAttribute: DlmsAttribute | null
}

export interface MainContentProps {
  cosemObjects: DlmsObject[]
  searchQuery: string
  selectedObject: DlmsObject | null
  selectedAttribute: DlmsAttribute | null
}

// Events
export interface SelectAttributeEvent {
  object: DlmsObject
  attribute: DlmsAttribute
}