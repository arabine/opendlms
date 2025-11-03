<template>
  <div v-if="isOpen" class="fixed inset-0 z-50 overflow-y-auto">
    <!-- Overlay -->
    <div 
      class="fixed inset-0 bg-black bg-opacity-50 transition-opacity"
      @click="close"
    ></div>

    <!-- Modal -->
    <div class="flex min-h-screen items-center justify-center p-4">
      <div 
        class="relative bg-white rounded-lg shadow-xl max-w-4xl w-full max-h-[90vh] overflow-y-auto"
        @click.stop
      >
        <!-- Header -->
        <div class="sticky top-0 bg-white border-b border-gray-200 px-6 py-4 flex items-center justify-between z-10">
          <h2 class="text-xl font-semibold text-gray-900">
            {{ isEditMode ? 'Éditer le test' : 'Ajouter un nouveau test' }}
          </h2>
          <button
            @click="close"
            class="text-gray-400 hover:text-gray-600 transition-colors"
          >
            <svg class="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M6 18L18 6M6 6l12 12" />
            </svg>
          </button>
        </div>

        <!-- Form -->
        <form @submit.prevent="save" class="p-6 space-y-6">
          <!-- Type -->
          <div>
            <label class="block text-sm font-semibold text-gray-700 mb-2">
              Type * <span class="text-xs font-normal text-gray-500">(obligatoire)</span>
            </label>
            <select
              v-model="formData.type"
              required
              class="w-full px-4 py-2 border border-gray-300 rounded-md focus:ring-2 focus:ring-blue-500 focus:border-blue-500"
            >
              <option value="chapter">Chapitre</option>
              <option value="section">Section</option>
              <option value="procedure">Procédure</option>
              <option value="test-case">Cas de test</option>
            </select>
          </div>

          <!-- Titre -->
          <div>
            <label class="block text-sm font-semibold text-gray-700 mb-2">
              Titre * <span class="text-xs font-normal text-gray-500">(obligatoire)</span>
            </label>
            <input
              v-model="formData.title"
              type="text"
              required
              placeholder="Entrez le titre du test"
              class="w-full px-4 py-2 border border-gray-300 rounded-md focus:ring-2 focus:ring-blue-500 focus:border-blue-500"
            />
          </div>

          <!-- Numéro ou Test ID selon le type -->
          <div>
            <div v-if="formData.type !== 'test-case'">
              <label class="block text-sm font-semibold text-gray-700 mb-2">Numéro</label>
              <input
                v-model="formData.number"
                type="text"
                placeholder="Ex: 7.1"
                class="w-full px-4 py-2 border border-gray-300 rounded-md focus:ring-2 focus:ring-blue-500 focus:border-blue-500"
              />
            </div>
            <div v-if="formData.type === 'test-case'">
              <label class="block text-sm font-semibold text-gray-700 mb-2">Test ID</label>
              <input
                v-model="formData.testId"
                type="text"
                placeholder="Ex: ACESM-UC06-TC01"
                class="w-full px-4 py-2 border border-gray-300 rounded-md focus:ring-2 focus:ring-blue-500 focus:border-blue-500"
              />
            </div>
          </div>

          <!-- Parent, Section et Chapitre -->
          <div class="grid grid-cols-3 gap-4">
            <div>
              <label class="block text-sm font-semibold text-gray-700 mb-2">Parent</label>
              <input
                v-model="formData.parent"
                type="text"
                placeholder="Ex: 7"
                class="w-full px-4 py-2 border border-gray-300 rounded-md focus:ring-2 focus:ring-blue-500 focus:border-blue-500"
              />
            </div>

            <div>
              <label class="block text-sm font-semibold text-gray-700 mb-2">Section</label>
              <input
                v-model="formData.section"
                type="text"
                placeholder="Ex: 7.1"
                class="w-full px-4 py-2 border border-gray-300 rounded-md focus:ring-2 focus:ring-blue-500 focus:border-blue-500"
              />
            </div>

            <div>
              <label class="block text-sm font-semibold text-gray-700 mb-2">Chapitre</label>
              <input
                v-model="formData.chapter"
                type="text"
                placeholder="Ex: 7"
                class="w-full px-4 py-2 border border-gray-300 rounded-md focus:ring-2 focus:ring-blue-500 focus:border-blue-500"
              />
            </div>
          </div>

          <!-- Champs détaillés pour procedure -->
          <div v-if="formData.type === 'procedure'" class="space-y-4 border-t border-gray-200 pt-6">
            <h3 class="text-lg font-semibold text-gray-800 mb-4">Détails de la procédure</h3>

            <div>
              <label class="block text-sm font-semibold text-gray-700 mb-2">References</label>
              <textarea
                v-model="formData.references"
                rows="2"
                placeholder="Références (ex: ACESM GCP Object definition File)"
                class="w-full px-4 py-2 border border-gray-300 rounded-md focus:ring-2 focus:ring-blue-500 focus:border-blue-500"
              ></textarea>
            </div>

            <div>
              <label class="block text-sm font-semibold text-gray-700 mb-2">Test Purpose</label>
              <textarea
                v-model="formData.testPurpose"
                rows="3"
                placeholder="Objectif de la procédure..."
                class="w-full px-4 py-2 border border-gray-300 rounded-md focus:ring-2 focus:ring-blue-500 focus:border-blue-500"
              ></textarea>
            </div>

            <div>
              <label class="block text-sm font-semibold text-gray-700 mb-2">Procedure Body</label>
              <textarea
                v-model="formData.procedureBody"
                rows="8"
                placeholder="Corps de la procédure..."
                class="w-full px-4 py-2 border border-gray-300 rounded-md focus:ring-2 focus:ring-blue-500 focus:border-blue-500 font-mono text-sm"
              ></textarea>
            </div>
          </div>

          <!-- Champs détaillés pour test-case -->
          <div v-if="formData.type === 'test-case'" class="space-y-4 border-t border-gray-200 pt-6">
            <h3 class="text-lg font-semibold text-gray-800 mb-4">Détails du test case</h3>

            <div class="grid grid-cols-2 gap-4">
              <div>
                <label class="block text-sm font-semibold text-gray-700 mb-2">Use Case</label>
                <input
                  v-model="formData.useCase"
                  type="text"
                  placeholder="Ex: UC06"
                  class="w-full px-4 py-2 border border-gray-300 rounded-md focus:ring-2 focus:ring-blue-500 focus:border-blue-500"
                />
              </div>

              <div>
                <label class="block text-sm font-semibold text-gray-700 mb-2">Scenario</label>
                <input
                  v-model="formData.scenario"
                  type="text"
                  placeholder="Ex: S01"
                  class="w-full px-4 py-2 border border-gray-300 rounded-md focus:ring-2 focus:ring-blue-500 focus:border-blue-500"
                />
              </div>
            </div>

            <div>
              <label class="block text-sm font-semibold text-gray-700 mb-2">Test Purpose</label>
              <textarea
                v-model="formData.testPurpose"
                rows="3"
                placeholder="Objectif du test..."
                class="w-full px-4 py-2 border border-gray-300 rounded-md focus:ring-2 focus:ring-blue-500 focus:border-blue-500"
              ></textarea>
            </div>

            <div>
              <label class="block text-sm font-semibold text-gray-700 mb-2">Test Strategy</label>
              <textarea
                v-model="formData.testStrategy"
                rows="3"
                placeholder="Stratégie de test..."
                class="w-full px-4 py-2 border border-gray-300 rounded-md focus:ring-2 focus:ring-blue-500 focus:border-blue-500"
              ></textarea>
            </div>

            <div>
              <label class="block text-sm font-semibold text-gray-700 mb-2">AA Filter</label>
              <input
                v-model="formData.aaFilter"
                type="text"
                placeholder="Ex: Management AA"
                class="w-full px-4 py-2 border border-gray-300 rounded-md focus:ring-2 focus:ring-blue-500 focus:border-blue-500"
              />
            </div>

            <div>
              <label class="block text-sm font-semibold text-gray-700 mb-2">Prerequisites</label>
              <textarea
                v-model="formData.prerequisites"
                rows="3"
                placeholder="Prérequis du test..."
                class="w-full px-4 py-2 border border-gray-300 rounded-md focus:ring-2 focus:ring-blue-500 focus:border-blue-500"
              ></textarea>
            </div>

            <div>
              <label class="block text-sm font-semibold text-gray-700 mb-2">Expected Result</label>
              <textarea
                v-model="formData.expectedResult"
                rows="3"
                placeholder="Résultat attendu..."
                class="w-full px-4 py-2 border border-gray-300 rounded-md focus:ring-2 focus:ring-blue-500 focus:border-blue-500"
              ></textarea>
            </div>

            <div>
              <label class="block text-sm font-semibold text-gray-700 mb-2">Preamble</label>
              <textarea
                v-model="formData.preamble"
                rows="4"
                placeholder="Préambule du test..."
                class="w-full px-4 py-2 border border-gray-300 rounded-md focus:ring-2 focus:ring-blue-500 focus:border-blue-500"
              ></textarea>
            </div>

            <div>
              <label class="block text-sm font-semibold text-gray-700 mb-2">Test Body</label>
              <textarea
                v-model="formData.testBody"
                rows="8"
                placeholder="Corps du test..."
                class="w-full px-4 py-2 border border-gray-300 rounded-md focus:ring-2 focus:ring-blue-500 focus:border-blue-500"
              ></textarea>
            </div>

            <div>
              <label class="block text-sm font-semibold text-gray-700 mb-2">Postamble</label>
              <textarea
                v-model="formData.postamble"
                rows="3"
                placeholder="Post-traitement..."
                class="w-full px-4 py-2 border border-gray-300 rounded-md focus:ring-2 focus:ring-blue-500 focus:border-blue-500"
              ></textarea>
            </div>

            <div>
              <label class="block text-sm font-semibold text-gray-700 mb-2">Comment</label>
              <textarea
                v-model="formData.comment"
                rows="3"
                placeholder="Commentaires..."
                class="w-full px-4 py-2 border border-gray-300 rounded-md focus:ring-2 focus:ring-blue-500 focus:border-blue-500"
              ></textarea>
            </div>
          </div>

          <!-- Footer avec boutons -->
          <div class="sticky bottom-0 bg-white border-t border-gray-200 pt-6 flex justify-end gap-3">
            <button
              type="button"
              @click="close"
              class="px-6 py-2 border border-gray-300 text-gray-700 rounded-md hover:bg-gray-50 transition-colors"
            >
              Annuler
            </button>
            <button
              type="submit"
              class="px-6 py-2 bg-blue-600 hover:bg-blue-700 text-white rounded-md transition-colors flex items-center gap-2"
            >
              <svg class="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M5 13l4 4L19 7" />
              </svg>
              {{ isEditMode ? 'Mettre à jour' : 'Créer' }}
            </button>
          </div>
        </form>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, watch } from 'vue'
import type { AtpTest, TestType } from '@/types'

const props = defineProps<{
  isOpen: boolean
  test?: AtpTest | null
}>()

const emit = defineEmits<{
  close: []
  save: [test: AtpTest]
}>()

const isEditMode = ref(false)

const formData = ref<Partial<AtpTest>>({
  type: 'test-case',
  title: '',
  number: '',
  testId: '',
  parent: null,
  chapter: null,
  section: null,
  references: '',
  testPurpose: '',
  procedureBody: '',
  useCase: '',
  scenario: '',
  testStrategy: '',
  aaFilter: '',
  prerequisites: '',
  expectedResult: '',
  preamble: '',
  testBody: '',
  postamble: '',
  comment: ''
})

// Définir resetForm AVANT le watch
const resetForm = () => {
  formData.value = {
    type: 'test-case',
    title: '',
    number: '',
    testId: '',
    parent: null,
    chapter: null,
    section: null,
    references: '',
    testPurpose: '',
    procedureBody: '',
    useCase: '',
    scenario: '',
    testStrategy: '',
    aaFilter: '',
    prerequisites: '',
    expectedResult: '',
    preamble: '',
    testBody: '',
    postamble: '',
    comment: ''
  }
}

// Watch APRÈS la définition de resetForm
watch(() => props.test, (newTest) => {
  if (newTest) {
    isEditMode.value = true
    formData.value = { ...newTest }
  } else {
    isEditMode.value = false
    resetForm()
  }
}, { immediate: true })

const close = () => {
  emit('close')
  resetForm()
}

const save = () => {
  // Générer un ID si nécessaire (pour les nouveaux tests)
  if (!formData.value._id) {
    const type = formData.value.type as string
    const identifier = formData.value.testId || formData.value.number || 'new'
    formData.value._id = `${type}_${identifier}_${Date.now()}`
  }

  // Ajouter le timestamp si nécessaire
  if (!formData.value.timestamp) {
    formData.value.timestamp = new Date().toISOString()
  }

  // Créer les tableData si c'est un test-case
  if (formData.value.type === 'test-case') {
    formData.value.tableData = {
      'Use Case': formData.value.useCase || '',
      'Scenario': formData.value.scenario || '',
      'Test purpose': formData.value.testPurpose || '',
      'Test strategy': formData.value.testStrategy || '',
      'AA filter': formData.value.aaFilter || '',
      'Prerequisites': formData.value.prerequisites || '',
      'Preamble': formData.value.preamble || '',
      'Test body': formData.value.testBody || '',
      'Postamble': formData.value.postamble || '',
      'Comment': formData.value.comment || ''
    }
  }

  emit('save', formData.value as AtpTest)
  close()
}
</script>
