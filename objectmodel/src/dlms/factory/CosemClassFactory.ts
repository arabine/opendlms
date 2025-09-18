/**
 * Factory DLMS/COSEM - Gestionnaire central de toutes les classes
 * 
 * @fileoverview Factory pattern pour créer et gérer tous les générateurs de classes COSEM
 * @version 1.0.0
 * @author DLMS COSEM Generator
 */

import {
  ClassId,
  Version,
  ObisCode,
  AttributeIndex,
  CosemClassInfo
} from '../types/common'
import { ICosemClassGenerator } from '../interfaces/ICosemClass'

// Import de toutes les classes implémentées
import DataClassGenerator from '../classes/Class01_Data'
import RegisterClassGenerator from '../classes/Class03_Register'
import PushSetupGenerator from '../classes/Class40_PushSetup'

// ===============================================
// REGISTRY DES CLASSES SUPPORTÉES
// ===============================================

/** 
 * Registre de toutes les classes COSEM supportées
 * Basé sur l'analyse du fichier Excel WSM GCP Object Model
 */
const COSEM_CLASS_REGISTRY = new Map<ClassId, () => ICosemClassGenerator>([
  // Classes fondamentales
  [1, () => new DataClassGenerator()],           // Data
  [3, () => new RegisterClassGenerator()],       // Register
  
  // Classes de communication et push
  [40, () => new PushSetupGenerator()],          // Push Setup v2
  
  // TODO: Ajouter les autres classes selon analyse Excel
  // [6, () => new RegisterActivationGenerator()],    // Register activation
  // [7, () => new ProfileGenericGenerator()],         // Profile generic v1
  // [8, () => new ClockGenerator()],                  // Clock
  // [9, () => new ScriptTableGenerator()],            // Script table
  // [11, () => new SpecialDaysTableGenerator()],      // Special days table
  // [15, () => new AssociationLNGenerator()],         // Association LN v3
  // [17, () => new SapAssignmentGenerator()],         // SAP Assignment
  // [18, () => new ImageTransferGenerator()],         // Image transfer
  // [19, () => new IecLocalPortSetupGenerator()],     // IEC local port setup v1
  // [20, () => new ActivityCalendarGenerator()],      // Activity Calendar
  // [21, () => new RegisterMonitorGenerator()],       // Register monitor
  // [22, () => new SingleActionScheduleGenerator()],  // Single action schedule
  // [23, () => new IecHdlcSetupGenerator()],          // IEC HDLC setup v1
  // [25, () => new MBusSlavePortSetupGenerator()],    // M-Bus slave port setup
  // [27, () => new ModemConfigurationGenerator()],    // Modem configuration v1
  // [28, () => new AutoAnswerGenerator()],            // Auto answer v2
  // [29, () => new AutoConnectGenerator()],           // Auto connect v2
  // [41, () => new TcpUdpSetupGenerator()],           // TCP-UDP setup
  // [42, () => new IPv4SetupGenerator()],             // IPv4 setup
  // [43, () => new MacAddressSetupGenerator()],       // MAC address setup
  // [44, () => new PppSetupGenerator()],              // PPP setup
  // [45, () => new GprsModemSetupGenerator()],        // GPRS modem setup
  // [47, () => new GsmDiagnosticGenerator()],         // GSM Diagnostic v2
  // [48, () => new IPv6SetupGenerator()],             // IPv6 setup
  // [62, () => new CompactDataGenerator()],           // Compact Data v1
  // [64, () => new SecuritySetupGenerator()],         // Security setup v1
  // [68, () => new ArbitratorGenerator()],            // Arbitrator
  // [70, () => new DisconnectControlGenerator()],     // Disconnect control
  // [71, () => new LimiterGenerator()],               // Limiter
  // [73, () => new WirelessModeQChannelGenerator()],  // Wireless Mode Q channel v1
  // [76, () => new DlmsServerMBusPortSetupGenerator()], // DLMS server M-Bus port setup
  // [77, () => new MBusDiagnosticGenerator()],        // M-Bus diagnostic
  // [95, () => new WiSunSetupGenerator()],            // Wi-SUN setup
  // [96, () => new WiSunDiagnosticGenerator()],       // Wi-SUN Diagnostic
  // [97, () => new RplDiagnosticGenerator()],         // RPL Diagnostic
  // [98, () => new MplDiagnosticGenerator()],         // MPL diagnostic
  // [100, () => new NtpSetupGenerator()],             // NTP setup
  // [111, () => new AccountGenerator()],              // Account (Prepayment)
  // [112, () => new CreditGenerator()],               // Credit
  // [113, () => new ChargeGenerator()],               // Charge
  // [115, () => new TokenGatewayGenerator()],         // Token gateway
  // [122, () => new FunctionControlGenerator()],      // Function control
  // [126, () => new LpwanSetupGenerator()],           // LPWAN Setup
  // [127, () => new LpwanDiagnosticGenerator()],      // LPWAN Diagnostic
  // [128, () => new LorawanSetupGenerator()],         // LoRaWAN Setup
  // [129, () => new LorawanDiagnosticGenerator()],    // LoRaWAN Diagnostic
])

// ===============================================
// FACTORY PRINCIPALE
// ===============================================

/**
 * Factory centrale pour créer et gérer les générateurs de classes COSEM
 */
export class CosemClassFactory {
  
  /** Cache des instances créées pour éviter les re-créations */
  private static instanceCache = new Map<ClassId, ICosemClassGenerator>()

  // ===============================================
  // MÉTHODES PRINCIPALES
  // ===============================================

  /**
   * Obtient un générateur pour une classe donnée
   * @param classId Identifiant de classe COSEM
   * @returns Générateur pour la classe ou erreur si non supporté
   */
  static getGenerator(classId: ClassId): ICosemClassGenerator {
    // Vérifier le cache d'abord
    if (this.instanceCache.has(classId)) {
      return this.instanceCache.get(classId)!
    }

    // Créer nouvelle instance
    const factory = COSEM_CLASS_REGISTRY.get(classId)
    if (!factory) {
      throw new Error(
        `Classe COSEM ${classId} non supportée. ` +
        `Classes disponibles: ${this.getSupportedClassIds().join(', ')}`
      )
    }

    const generator = factory()
    this.instanceCache.set(classId, generator)
    return generator
  }

  /**
   * Génère directement un SetRequest XML
   * @param classId Identifiant de classe
   * @param obisCode Code OBIS de l'objet
   * @param attributeIndex Index de l'attribut
   * @param value Valeur à écrire
   * @param invokeId ID d'invocation (optionnel)
   * @returns XML SetRequest complet
   */
  static generateSetRequest(
    classId: ClassId,
    obisCode: ObisCode,
    attributeIndex: AttributeIndex,
    value: any,
    invokeId?: number
  ): string {
    const generator = this.getGenerator(classId)
    return generator.generateSetRequest(obisCode, attributeIndex, value, invokeId)
  }

  /**
   * Génère directement un GetRequest XML
   * @param classId Identifiant de classe
   * @param obisCode Code OBIS de l'objet
   * @param attributeIndex Index de l'attribut
   * @param invokeId ID d'invocation (optionnel)
   * @returns XML GetRequest complet
   */
  static generateGetRequest(
    classId: ClassId,
    obisCode: ObisCode,
    attributeIndex: AttributeIndex,
    invokeId?: number
  ): string {
    const generator = this.getGenerator(classId)
    return generator.generateGetRequest(obisCode, attributeIndex, invokeId)
  }

  // ===============================================
  // MÉTHODES D'INFORMATION
  // ===============================================

  /**
   * Liste tous les class_id supportés
   * @returns Array des identifiants de classe supportés
   */
  static getSupportedClassIds(): ClassId[] {
    return Array.from(COSEM_CLASS_REGISTRY.keys()).sort((a, b) => a - b)
  }

  /**
   * Vérifie si une classe est supportée
   * @param classId Identifiant de classe à vérifier
   * @returns true si la classe est supportée
   */
  static isClassSupported(classId: ClassId): boolean {
    return COSEM_CLASS_REGISTRY.has(classId)
  }

  /**
   * Obtient les informations complètes sur une classe
   * @param classId Identifiant de classe
   * @returns Informations détaillées sur la classe
   */
  static getClassInfo(classId: ClassId): CosemClassInfo {
    const generator = this.getGenerator(classId)
    return generator.getClassInfo()
  }

  /**
   * Liste toutes les classes supportées avec leurs informations
   * @returns Map des informations de toutes les classes
   */
  static getAllClassInfo(): Map<ClassId, CosemClassInfo> {
    const classInfoMap = new Map<ClassId, CosemClassInfo>()
    
    this.getSupportedClassIds().forEach(classId => {
      try {
        const info = this.getClassInfo(classId)
        classInfoMap.set(classId, info)
      } catch (error) {
        console.warn(`Erreur lors de la récupération des infos pour class_id ${classId}:`, error)
      }
    })
    
    return classInfoMap
  }

  /**
   * Recherche des classes par nom ou description
   * @param searchTerm Terme de recherche
   * @returns Array des classes correspondantes
   */
  static searchClasses(searchTerm: string): CosemClassInfo[] {
    const searchLower = searchTerm.toLowerCase()
    const results: CosemClassInfo[] = []
    
    this.getSupportedClassIds().forEach(classId => {
      try {
        const info = this.getClassInfo(classId)
        if (
          info.name.toLowerCase().includes(searchLower) ||
          info.description.toLowerCase().includes(searchLower)
        ) {
          results.push(info)
        }
      } catch (error) {
        // Ignorer les erreurs de recherche
      }
    })
    
    return results.sort((a, b) => a.classId - b.classId)
  }

  // ===============================================
  // GESTION AVANCÉE
  // ===============================================

  /**
   * Ajoute dynamiquement un nouveau générateur de classe
   * @param classId Identifiant de classe
   * @param generatorFactory Function qui crée le générateur
   */
  static addClassGenerator(
    classId: ClassId, 
    generatorFactory: () => ICosemClassGenerator
  ): void {
    if (COSEM_CLASS_REGISTRY.has(classId)) {
      console.warn(`Class ID ${classId} déjà enregistré. Remplacement...`)
    }
    
    COSEM_CLASS_REGISTRY.set(classId, generatorFactory)
    
    // Invalider le cache pour cette classe
    if (this.instanceCache.has(classId)) {
      this.instanceCache.delete(classId)
    }
  }

  /**
   * Supprime un générateur de classe (pour tests/développement)
   * @param classId Identifiant de classe à supprimer
   */
  static removeClassGenerator(classId: ClassId): boolean {
    const removed = COSEM_CLASS_REGISTRY.delete(classId)
    this.instanceCache.delete(classId)
    return removed
  }

  /**
   * Vide le cache des instances (utile pour tests)
   */
  static clearCache(): void {
    this.instanceCache.clear()
  }

  /**
   * Valide qu'un objet peut être configuré avec une classe donnée
   * @param classId Identifiant de classe
   * @param obisCode Code OBIS
   * @param attributeIndex Index attribut
   * @param value Valeur à valider
   * @returns true si valide, sinon lève une exception
   */
  static validateConfiguration(
    classId: ClassId,
    obisCode: ObisCode,
    attributeIndex: AttributeIndex,
    value: any
  ): boolean {
    const generator = this.getGenerator(classId)
    return generator.validateAttribute(attributeIndex, value)
  }

  // ===============================================
  // MÉTHODES DE BATCH/GROUPE
  // ===============================================

  /**
   * Génère plusieurs SetRequest en une seule opération
   * @param configurations Array de configurations à appliquer
   * @returns Array des XML générés
   */
  static generateBatchSetRequests(configurations: Array<{
    classId: ClassId
    obisCode: ObisCode
    attributeIndex: AttributeIndex
    value: any
    invokeId?: number
  }>): string[] {
    return configurations.map((config, index) => {
      try {
        return this.generateSetRequest(
          config.classId,
          config.obisCode,
          config.attributeIndex,
          config.value,
          config.invokeId || index + 1
        )
      } catch (error) {
        throw new Error(
          `Erreur configuration ${index + 1} ` +
          `(class_id=${config.classId}, obis=${config.obisCode}): ${error}`
        )
      }
    })
  }

  /**
   * Génère une configuration complète pour un type d'objet
   * @param template Template de configuration
   * @returns XML complet pour l'objet
   */
  static generateObjectConfiguration(template: {
    classId: ClassId
    obisCode: ObisCode
    attributes: Array<{ index: AttributeIndex, value: any }>
  }): string {
    const xmlParts: string[] = []
    
    template.attributes.forEach((attr, index) => {
      const xml = this.generateSetRequest(
        template.classId,
        template.obisCode,
        attr.index,
        attr.value,
        index + 1
      )
      xmlParts.push(xml)
    })
    
    return xmlParts.join('\n\n')
  }

  // ===============================================
  // REPORTING ET DIAGNOSTICS
  // ===============================================

  /**
   * Génère un rapport complet des classes supportées
   * @returns Rapport détaillé au format markdown
   */
  static generateSupportReport(): string {
    const supportedIds = this.getSupportedClassIds()
    const totalClasses = supportedIds.length
    
    let report = `# Rapport Classes COSEM Supportées\n\n`
    report += `**Total classes supportées:** ${totalClasses}\n\n`
    report += `## Classes implémentées\n\n`
    
    supportedIds.forEach(classId => {
      try {
        const info = this.getClassInfo(classId)
        report += `### Class ID ${classId} - ${info.name} (v${info.version})\n\n`
        report += `${info.description}\n\n`
        report += `**Attributs:** ${info.attributes.length}\n`
        report += `**Méthodes:** ${info.methods.length}\n\n`
        
        // Liste des attributs
        if (info.attributes.length > 0) {
          report += `#### Attributs:\n`
          info.attributes.forEach(attr => {
            const access = attr.accessMode === 1 ? 'RO' : 
                          attr.accessMode === 2 ? 'WO' : 
                          attr.accessMode === 3 ? 'RW' : 'AUTH'
            const mandatory = attr.mandatory ? 'M' : 'O'
            const static_flag = attr.static ? 'S' : 'D'
            report += `- ${attr.index}. **${attr.name}** (${attr.dataType}) - ${access}/${mandatory}/${static_flag}\n`
          })
          report += '\n'
        }
        
        // Liste des méthodes
        if (info.methods.length > 0) {
          report += `#### Méthodes:\n`
          info.methods.forEach(method => {
            const mandatory = method.mandatory ? 'M' : 'O'
            report += `- ${method.id}. **${method.name}** - ${mandatory}\n`
          })
          report += '\n'
        }
        
        report += `---\n\n`
      } catch (error) {
        report += `### Class ID ${classId} - ERREUR\n\n`
        report += `Impossible de récupérer les informations: ${error}\n\n`
        report += `---\n\n`
      }
    })
    
    return report
  }

  /**
   * Statistiques d'utilisation de la factory
   * @returns Objet avec les statistiques
   */
  static getUsageStatistics(): {
    supportedClasses: number
    cachedInstances: number
    supportedClassIds: ClassId[]
    cacheHitRatio?: number
  } {
    return {
      supportedClasses: COSEM_CLASS_REGISTRY.size,
      cachedInstances: this.instanceCache.size,
      supportedClassIds: this.getSupportedClassIds(),
      // TODO: Ajouter tracking des hits/miss pour calculer le ratio
    }
  }

  // ===============================================
  // MÉTHODES D'EXEMPLE ET DÉMONSTRATION
  // ===============================================

  /**
   * Génère des exemples d'utilisation pour toutes les classes
   * @returns Map des exemples par classe
   */
  static generateAllExamples(): Map<ClassId, { [key: string]: string }> {
    const examplesMap = new Map<ClassId, { [key: string]: string }>()
    
    this.getSupportedClassIds().forEach(classId => {
      try {
        const examples: { [key: string]: string } = {}
        
        // Exemples spécialisés selon la classe
        switch (classId) {
          case 1: // Data
            examples['Device ID'] = this.generateSetRequest(1, '0-0:96.1.0.255', 2, 'WSM123456')
            examples['Battery Status'] = this.generateSetRequest(1, '0-0:96.6.1.255', 2, true)
            examples['Event Counter'] = this.generateSetRequest(1, '0-0:96.15.0.255', 2, 42)
            break
            
          case 3: // Register
            examples['Water Total'] = this.generateSetRequest(3, '1-0:1.8.0.255', 2, 123456)
            examples['Flow Rate'] = this.generateSetRequest(3, '1-0:2.27.0.255', 2, 150)
            break
            
          case 40: // Push Setup
            examples['Push Object List'] = this.generateSetRequest(40, '0-0:25.9.0.255', 2, [
              {
                class_id: 8,
                logical_name: "0000010000FF",
                attribute_index: 2,
                data_index: 0,
                restriction: { restriction_type: 0 },
                columns: []
              }
            ])
            break
            
          default:
            // Exemple générique - logical_name
            examples['Logical Name'] = this.generateGetRequest(classId, '0-0:1.0.0.255', 1)
        }
        
        examplesMap.set(classId, examples)
      } catch (error) {
        examplesMap.set(classId, { 'Erreur': `Impossible de générer l'exemple: ${error}` })
      }
    })
    
    return examplesMap
  }

  /**
   * Démonstration complète des capacités de la factory
   */
  static demonstrateCapabilities(): void {
    console.log('='.repeat(80))
    console.log('DLMS COSEM Class Factory - Démonstration des capacités')
    console.log('='.repeat(80))
    
    const stats = this.getUsageStatistics()
    console.log(`\n📊 Statistiques:`)
    console.log(`   - Classes supportées: ${stats.supportedClasses}`)
    console.log(`   - Instances en cache: ${stats.cachedInstances}`)
    console.log(`   - Classes disponibles: ${stats.supportedClassIds.join(', ')}`)
    
    console.log(`\n🔍 Recherche de classes (mot-clé "push"):`)
    const pushClasses = this.searchClasses('push')
    pushClasses.forEach(cls => {
      console.log(`   - Class ${cls.classId}: ${cls.name} - ${cls.description}`)
    })
    
    console.log(`\n⚙️ Configuration d'exemple:`)
    try {
      const xml = this.generateSetRequest(1, '0-0:96.1.0.255', 2, 'DEMO_DEVICE_123')
      console.log('   Device ID configuré avec succès')
    } catch (error) {
      console.log(`   Erreur: ${error}`)
    }
    
    console.log(`\n📝 Validation d'exemple:`)
    try {
      const valid = this.validateConfiguration(3, '1-0:1.8.0.255', 2, 123.456)
      console.log(`   Validation Register: ${valid ? 'OK' : 'ÉCHEC'}`)
    } catch (error) {
      console.log(`   Erreur validation: ${error}`)
    }
    
    console.log('\n' + '='.repeat(80))
  }
}

// ===============================================
// CLASSE D'AIDE POUR CONFIGURATIONS COMPLEXES
// ===============================================

/**
 * Builder pour construire des configurations complexes d'objets COSEM
 */
export class CosemConfigurationBuilder {
  private configurations: Array<{
    classId: ClassId
    obisCode: ObisCode
    attributeIndex: AttributeIndex
    value: any
    invokeId?: number
  }> = []

  /**
   * Ajoute une configuration d'attribut
   */
  addAttribute(
    classId: ClassId,
    obisCode: ObisCode,
    attributeIndex: AttributeIndex,
    value: any
  ): this {
    this.configurations.push({ classId, obisCode, attributeIndex, value })
    return this
  }

  /**
   * Ajoute une configuration complète d'objet
   */
  addObject(
    classId: ClassId,
    obisCode: ObisCode,
    attributes: Array<{ index: AttributeIndex, value: any }>
  ): this {
    attributes.forEach(attr => {
      this.addAttribute(classId, obisCode, attr.index, attr.value)
    })
    return this
  }

  /**
   * Génère tous les XML SetRequest
   */
  build(): string[] {
    return CosemClassFactory.generateBatchSetRequests(this.configurations)
  }

  /**
   * Génère un seul XML concaténé
   */
  buildConcatenated(): string {
    return this.build().join('\n\n')
  }

  /**
   * Remet à zéro le builder
   */
  clear(): this {
    this.configurations = []
    return this
  }

  /**
   * Obtient le nombre de configurations
   */
  count(): number {
    return this.configurations.length
  }
}

// ===============================================
// EXPORT PAR DÉFAUT
// ===============================================

export default CosemClassFactory
