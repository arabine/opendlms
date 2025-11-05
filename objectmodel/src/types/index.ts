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

// ATP Test Types
export type TestType = 'chapter' | 'section' | 'procedure' | 'test-case'

export interface AtpTest {
  _id: string
  type: TestType
  number?: string
  testId?: string
  title: string
  line?: number  // Optionnel maintenant
  parent?: string | null
  chapter?: string | null
  section?: string | null  // Section parente (pour procédures et test cases)
  order?: number  // Ordre de l'élément dans sa liste (pour préserver l'ordre après drag & drop)
  validated?: boolean  // Statut de validation du test
  timestamp: string
  _rev?: string
  // Données de tableau (si applicable)
  tableColumns?: string[]  // En-têtes du tableau
  tableData?: Record<string, string>  // Données clé-valeur du tableau (pour les tableaux 2 colonnes)
  tableRows?: string[][]  // Données brutes du tableau (pour les tableaux multi-colonnes)
  
  // === Champs de PROCEDURE ===
  references?: string          // References (pour procédure)
  procedureBody?: string       // Procedure body (pour procédure)
  procedureSteps?: string[]    // Procedure body décomposé en étapes
  
  // === Champs de TEST CASE ===
  useCase?: string            // Use Case
  scenario?: string           // Scenario
  step?: string               // Step (optionnel)
  testPurpose?: string        // Test purpose
  testStrategy?: string       // Test strategy
  aaFilter?: string           // AA filter / AAs used
  prerequisites?: string      // Prerequisites
  expectedResult?: string     // Expected result (optionnel)
  preamble?: string           // Preamble
  testBody?: string           // Test body (peut contenir plusieurs subtests)
  testBodySteps?: string[]    // Test body décomposé en étapes
  postamble?: string          // Postamble
  comment?: string            // Comment
}

export interface AtpTestStats {
  total: number
  chapters: number
  sections: number
  procedures: number
  tests: number
  validatedTests: number
  validatedProcedures: number
}

export interface FileUploadAtpProps {
  loading: boolean
}

export interface AtpTestListProps {
  tests: AtpTest[]
  filterType: TestType | 'all'
  searchQuery: string
}

// Tree structure types
export interface AtpTreeNode {
  id: string
  test: AtpTest
  children: AtpTreeNode[]
  expanded: boolean
}

export interface AtpTreeViewProps {
  chapters: AtpTreeNode[]
  procedures: AtpTest[]
  selectedTest: AtpTest | null
  searchQuery: string
}

export interface AtpDetailViewProps {
  test: AtpTest | null
}