/**
 * Test de la classe 22 - Single action schedule
 */

import {
  CosemClassFactory,
  SingleActionScheduleClassGenerator,
  ScheduleType,
  WsmGcpConfigurations
} from './src/dlms/index.ts'

console.log('='.repeat(80))
console.log('TEST - DLMS COSEM Class 22 - Single Action Schedule')
console.log('='.repeat(80))

// Vérifier que la classe 22 est supportée
console.log('\n1. Vérification du support de la classe 22:')
console.log('   Classes supportées:', CosemClassFactory.getSupportedClassIds())
console.log('   Classe 22 supportée:', CosemClassFactory.isClassSupported(22) ? 'OUI ✅' : 'NON ❌')

// Obtenir les informations de la classe
console.log('\n2. Informations sur la classe 22:')
try {
  const classInfo = CosemClassFactory.getClassInfo(22)
  console.log('   Nom:', classInfo.name)
  console.log('   Description:', classInfo.description)
  console.log('   Version:', classInfo.version)
  console.log('   Nombre d\'attributs:', classInfo.attributes.length)
  console.log('   Attributs:')
  classInfo.attributes.forEach(attr => {
    console.log(`     - ${attr.index}. ${attr.name} (${attr.dataType})`)
  })
} catch (error) {
  console.error('   Erreur:', error.message)
}

// Test 1: Génération GetRequest
console.log('\n3. Test GetRequest pour l\'attribut logical_name:')
try {
  const xml = CosemClassFactory.generateGetRequest(22, '0-0:15.0.1.0', 1)
  console.log('   Succès ✅')
  console.log('   XML généré:')
  console.log(xml.split('\n').map(line => '   ' + line).join('\n'))
} catch (error) {
  console.error('   Erreur ❌:', error.message)
}

// Test 2: Génération SetRequest pour executed_script
console.log('\n4. Test SetRequest pour executed_script (attribut 2):')
try {
  const xml = CosemClassFactory.generateSetRequest(22, '0-0:15.0.1.0', 2, {
    script_logical_name: '0-0:10.0.1.0',
    script_selector: 1
  })
  console.log('   Succès ✅')
  console.log('   XML généré:')
  console.log(xml.split('\n').map(line => '   ' + line).join('\n'))
} catch (error) {
  console.error('   Erreur ❌:', error.message)
}

// Test 3: Génération SetRequest pour type
console.log('\n5. Test SetRequest pour type (attribut 3):')
try {
  const xml = CosemClassFactory.generateSetRequest(22, '0-0:15.0.1.0', 3, ScheduleType.SIZE_1_WILDCARD_ALLOWED)
  console.log('   Succès ✅')
  console.log('   Type:', ScheduleType.SIZE_1_WILDCARD_ALLOWED)
  console.log('   XML généré:')
  console.log(xml.split('\n').map(line => '   ' + line).join('\n'))
} catch (error) {
  console.error('   Erreur ❌:', error.message)
}

// Test 4: Génération SetRequest pour execution_time
console.log('\n6. Test SetRequest pour execution_time (attribut 4):')
try {
  const xml = CosemClassFactory.generateSetRequest(22, '0-0:15.0.1.0', 4, [
    {
      time: '00000000FF', // 00:00:00
      date: '07FFFFFFFFFF0000FF' // Tous les jours
    }
  ])
  console.log('   Succès ✅')
  console.log('   XML généré:')
  console.log(xml.split('\n').map(line => '   ' + line).join('\n'))
} catch (error) {
  console.error('   Erreur ❌:', error.message)
}

// Test 5: Configuration complète avec méthode simplifiée
console.log('\n7. Test configuration complète - Planning quotidien à 00:00:')
try {
  const xml = SingleActionScheduleClassGenerator.createSimpleSchedule(
    '0-0:15.0.1.0',
    '0-0:10.0.1.0',
    1,
    '00000000FF',
    '07FFFFFFFFFF0000FF'
  )
  console.log('   Succès ✅')
  console.log('   XML généré:')
  console.log(xml.split('\n').map(line => '   ' + line).join('\n'))
} catch (error) {
  console.error('   Erreur ❌:', error.message)
}

// Test 6: Configuration avec WSM GCP helper
console.log('\n8. Test configuration WSM GCP - Planning quotidien à 06:00:')
try {
  const xml = WsmGcpConfigurations.createDailySchedule(
    '0-0:15.0.1.0',
    '0-0:10.0.1.0',
    1,
    6, // 06:00
    0
  )
  console.log('   Succès ✅')
  console.log('   XML généré:')
  console.log(xml.split('\n').map(line => '   ' + line).join('\n'))
} catch (error) {
  console.error('   Erreur ❌:', error.message)
}

// Test 7: Planning multiple (3 fois par jour)
console.log('\n9. Test planning multiple - 3 exécutions par jour:')
try {
  const xml = SingleActionScheduleClassGenerator.createMultipleSchedule(
    '0-0:15.0.1.1',
    '0-0:10.0.1.0',
    2,
    [
      { time: '06000000FF', date: '07FFFFFFFFFF0000FF' }, // 06:00
      { time: '0C000000FF', date: '07FFFFFFFFFF0000FF' }, // 12:00
      { time: '12000000FF', date: '07FFFFFFFFFF0000FF' }  // 18:00
    ]
  )
  console.log('   Succès ✅')
  console.log('   XML généré (extrait):')
  const lines = xml.split('\n')
  console.log(lines.slice(0, 20).map(line => '   ' + line).join('\n'))
  console.log('   ... (', lines.length, 'lignes au total)')
} catch (error) {
  console.error('   Erreur ❌:', error.message)
}

console.log('\n' + '='.repeat(80))
console.log('FIN DES TESTS')
console.log('='.repeat(80))
