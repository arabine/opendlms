<template>
  <div v-if="test" class="h-full overflow-y-auto bg-white">
    <!-- En-tête -->
    <div :class="[
      'p-6 border-b-4',
      getHeaderClasses(test.type)
    ]">
      <div class="flex items-start justify-between mb-4">
        <div class="flex items-center gap-3">
          <span :class="getIconClasses(test.type)">
            {{ getIcon(test.type) }}
          </span>
          <div>
            <div class="text-2xl font-bold text-gray-900">
              {{ test.number || test.testId || 'N/A' }}
            </div>
            <div :class="[
              'mt-1 px-3 py-1 rounded-full text-xs font-semibold uppercase inline-block',
              getBadgeClasses(test.type)
            ]">
              {{ getTypeLabel(test.type) }}
            </div>
          </div>
        </div>

        <div class="flex gap-2">
          <!-- Bouton Valider (toujours visible pour procédures et test-cases) -->
          <button
            v-if="test.type === 'test-case' || test.type === 'procedure'"
            @click="toggleValidation"
            :class="[
              'px-3 py-2 text-sm rounded-md transition-colors flex items-center gap-2',
              test.validated
                ? 'bg-green-600 hover:bg-green-700 text-white'
                : 'bg-gray-400 hover:bg-gray-500 text-white'
            ]"
          >
            <svg class="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 12l2 2 4-4m6 2a9 9 0 11-18 0 9 9 0 0118 0z" />
            </svg>
            {{ test.validated ? 'Validé' : 'Valider' }}
          </button>

          <button
            v-if="!isEditing"
            @click="startEditing"
            class="px-3 py-2 text-sm bg-blue-600 hover:bg-blue-700 text-white rounded-md transition-colors flex items-center gap-2"
          >
            <svg class="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M11 5H6a2 2 0 00-2 2v11a2 2 0 002 2h11a2 2 0 002-2v-5m-1.414-9.414a2 2 0 112.828 2.828L11.828 15H9v-2.828l8.586-8.586z" />
            </svg>
            Éditer
          </button>
          <button
            v-if="isEditing"
            @click="saveChanges"
            class="px-3 py-2 text-sm bg-green-600 hover:bg-green-700 text-white rounded-md transition-colors flex items-center gap-2"
          >
            <svg class="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M5 13l4 4L19 7" />
            </svg>
            Sauvegarder
          </button>
          <button
            v-if="isEditing"
            @click="cancelEditing"
            class="px-3 py-2 text-sm bg-gray-500 hover:bg-gray-600 text-white rounded-md transition-colors flex items-center gap-2"
          >
            <svg class="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M6 18L18 6M6 6l12 12" />
            </svg>
            Annuler
          </button>
        </div>
      </div>

      <!-- Titre éditable -->
      <div v-if="!isEditing">
        <h2 class="text-xl font-semibold text-gray-800 leading-tight">
          {{ test.title }}
        </h2>
      </div>
      <div v-else>
        <input
          v-model="editableTest.title"
          type="text"
          class="w-full text-xl font-semibold text-gray-800 leading-tight border-2 border-blue-300 rounded px-3 py-2 focus:outline-none focus:border-blue-500"
          placeholder="Titre..."
        />
      </div>
    </div>

    <!-- Contenu -->
    <div class="p-6 space-y-6">
      <!-- Champs détaillés du CHAPITRE (si applicable) -->
      <div v-if="test.type === 'chapter'" class="space-y-4">
        <h3 class="text-lg font-semibold text-gray-800 mb-3">Détails du chapitre</h3>

        <!-- Number -->
        <div class="bg-green-50 rounded-lg p-4">
          <div class="text-xs font-semibold text-green-600 uppercase mb-2">Number</div>
          <input
            v-if="isEditing"
            v-model="editableTest.number"
            type="text"
            class="w-full border border-green-300 rounded px-3 py-2 text-sm focus:outline-none focus:border-green-500"
            placeholder="Numéro du chapitre (ex: 1, 2, 3...)"
          />
          <div v-else class="text-sm text-gray-900">{{ test.number || 'N/A' }}</div>
        </div>
      </div>

      <!-- Champs détaillés de la SECTION (si applicable) -->
      <div v-if="test.type === 'section'" class="space-y-4">
        <h3 class="text-lg font-semibold text-gray-800 mb-3">Détails de la section</h3>

        <!-- Number -->
        <div class="bg-yellow-50 rounded-lg p-4">
          <div class="text-xs font-semibold text-yellow-600 uppercase mb-2">Number</div>
          <input
            v-if="isEditing"
            v-model="editableTest.number"
            type="text"
            class="w-full border border-yellow-300 rounded px-3 py-2 text-sm focus:outline-none focus:border-yellow-500"
            placeholder="Numéro de section (ex: 1.1, 1.2...)"
          />
          <div v-else class="text-sm text-gray-900">{{ test.number || 'N/A' }}</div>
        </div>

        <!-- Chapter -->
        <div class="bg-yellow-50 rounded-lg p-4">
          <div class="text-xs font-semibold text-yellow-600 uppercase mb-2">Chapter</div>
          <input
            v-if="isEditing"
            v-model="editableTest.chapter"
            type="text"
            class="w-full border border-yellow-300 rounded px-3 py-2 text-sm focus:outline-none focus:border-yellow-500"
            placeholder="Numéro du chapitre parent"
          />
          <div v-else class="text-sm text-gray-900">{{ test.chapter || 'N/A' }}</div>
        </div>
      </div>

      <!-- Champs détaillés de la PROCÉDURE (si applicable) -->
      <div v-if="test.type === 'procedure'" class="space-y-4">
        <h3 class="text-lg font-semibold text-gray-800 mb-3">Détails de la procédure</h3>

        <!-- References -->
        <div class="bg-blue-50 rounded-lg p-4">
          <div class="text-xs font-semibold text-blue-600 uppercase mb-2">References</div>
          <textarea
            v-if="isEditing"
            v-model="editableTest.references"
            rows="2"
            class="w-full border border-blue-300 rounded px-3 py-2 text-sm focus:outline-none focus:border-blue-500"
            placeholder="Références..."
          ></textarea>
          <div v-else class="text-sm text-gray-900">{{ test.references || 'N/A' }}</div>
        </div>

        <!-- Test Purpose -->
        <div class="bg-blue-50 rounded-lg p-4">
          <div class="text-xs font-semibold text-blue-600 uppercase mb-2">Test Purpose</div>
          <textarea
            v-if="isEditing"
            v-model="editableTest.testPurpose"
            rows="3"
            class="w-full border border-blue-300 rounded px-3 py-2 text-sm focus:outline-none focus:border-blue-500"
            placeholder="Objectif..."
          ></textarea>
          <div v-else class="text-sm text-gray-900 whitespace-pre-wrap">{{ test.testPurpose || 'N/A' }}</div>
        </div>

        <!-- Procedure Body avec étapes -->
        <div class="bg-blue-50 rounded-lg p-4">
          <div class="flex items-center justify-between mb-3">
            <div class="text-xs font-semibold text-blue-600 uppercase">Procedure Body (Étapes)</div>
            <button
              v-if="isEditing"
              @click="addProcedureStep"
              class="px-3 py-1 text-xs bg-blue-600 hover:bg-blue-700 text-white rounded flex items-center gap-1"
            >
              <svg class="w-3 h-3" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 4v16m8-8H4" />
              </svg>
              Ajouter étape
            </button>
          </div>

          <div v-if="isEditing" class="space-y-2">
            <div
              v-for="(step, idx) in procedureSteps"
              :key="idx"
              class="bg-white rounded p-3 border-l-4 border-blue-500 relative group"
            >
              <div class="flex items-start justify-between gap-2">
                <div class="flex-1">
                  <div class="text-xs font-semibold text-gray-500 mb-1">Étape {{ idx + 1 }}</div>
                  <textarea
                    v-model="procedureSteps[idx]"
                    rows="15"
                    class="w-full border border-gray-300 rounded px-2 py-1 text-sm focus:outline-none focus:border-blue-500 font-mono"
                    placeholder="Description de l'étape..."
                  ></textarea>
                </div>
                <button
                  v-if="procedureSteps.length > 1"
                  @click="removeProcedureStep(idx)"
                  class="flex-shrink-0 p-1 text-red-600 hover:bg-red-100 rounded"
                  title="Supprimer l'étape"
                >
                  <svg class="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M19 7l-.867 12.142A2 2 0 0116.138 21H7.862a2 2 0 01-1.995-1.858L5 7m5 4v6m4-6v6m1-10V4a1 1 0 00-1-1h-4a1 1 0 00-1 1v3M4 7h16" />
                  </svg>
                </button>
              </div>
            </div>
          </div>
          <div v-else-if="procedureSteps.length > 0" class="space-y-2">
            <div
              v-for="(step, idx) in procedureSteps"
              :key="idx"
              class="bg-white rounded p-3 border-l-4 border-blue-500"
            >
              <div class="text-xs font-semibold text-gray-500 mb-1">Étape {{ idx + 1 }}</div>
              <div class="text-sm text-gray-900 whitespace-pre-wrap">{{ step }}</div>
            </div>
          </div>
          <div v-else class="text-sm text-gray-500 italic">Aucune étape définie</div>
        </div>
      </div>

      <!-- Champs détaillés du TEST CASE (si applicable) -->
      <div v-if="test.type === 'test-case'" class="space-y-4">
        <h3 class="text-lg font-semibold text-gray-800 mb-3">Détails du test case</h3>

        <!-- Test ID -->
        <div class="bg-green-50 rounded-lg p-4">
          <div class="text-xs font-semibold text-green-600 uppercase mb-2">Test ID</div>
          <input
            v-if="isEditing"
            v-model="editableTest.testId"
            type="text"
            class="w-full border border-green-300 rounded px-3 py-2 text-sm focus:outline-none focus:border-green-500"
            placeholder="Ex: ACESM-CV-TC01"
          />
          <div v-else class="text-sm text-gray-900 font-mono">{{ test.testId || 'N/A' }}</div>
        </div>

        <!-- Use Case & Scenario en ligne -->
        <div class="grid grid-cols-2 gap-4">
          <div class="bg-green-50 rounded-lg p-4">
            <div class="text-xs font-semibold text-green-600 uppercase mb-2">Use Case</div>
            <input
              v-if="isEditing"
              v-model="editableTest.useCase"
              type="text"
              class="w-full border border-green-300 rounded px-3 py-2 text-sm focus:outline-none focus:border-green-500"
              placeholder="Ex: UC06"
            />
            <div v-else class="text-sm text-gray-900">{{ test.useCase || 'N/A' }}</div>
          </div>

          <div class="bg-green-50 rounded-lg p-4">
            <div class="text-xs font-semibold text-green-600 uppercase mb-2">Scenario</div>
            <input
              v-if="isEditing"
              v-model="editableTest.scenario"
              type="text"
              class="w-full border border-green-300 rounded px-3 py-2 text-sm focus:outline-none focus:border-green-500"
              placeholder="Ex: S01"
            />
            <div v-else class="text-sm text-gray-900">{{ test.scenario || 'N/A' }}</div>
          </div>
        </div>

        <!-- Test Purpose -->
        <div class="bg-green-50 rounded-lg p-4">
          <div class="text-xs font-semibold text-green-600 uppercase mb-2">Test Purpose</div>
          <textarea
            v-if="isEditing"
            v-model="editableTest.testPurpose"
            rows="3"
            class="w-full border border-green-300 rounded px-3 py-2 text-sm focus:outline-none focus:border-green-500"
            placeholder="Objectif du test..."
          ></textarea>
          <div v-else class="text-sm text-gray-900 whitespace-pre-wrap">{{ test.testPurpose || 'N/A' }}</div>
        </div>

        <!-- Test Strategy -->
        <div class="bg-green-50 rounded-lg p-4">
          <div class="text-xs font-semibold text-green-600 uppercase mb-2">Test Strategy</div>
          <textarea
            v-if="isEditing"
            v-model="editableTest.testStrategy"
            rows="3"
            class="w-full border border-green-300 rounded px-3 py-2 text-sm focus:outline-none focus:border-green-500"
            placeholder="Stratégie..."
          ></textarea>
          <div v-else class="text-sm text-gray-900 whitespace-pre-wrap">{{ test.testStrategy || 'N/A' }}</div>
        </div>

        <!-- Prerequisites -->
        <div class="bg-green-50 rounded-lg p-4">
          <div class="text-xs font-semibold text-green-600 uppercase mb-2">Prerequisites</div>
          <textarea
            v-if="isEditing"
            v-model="editableTest.prerequisites"
            rows="3"
            class="w-full border border-green-300 rounded px-3 py-2 text-sm focus:outline-none focus:border-green-500"
            placeholder="Prérequis..."
          ></textarea>
          <div v-else class="text-sm text-gray-900 whitespace-pre-wrap">{{ test.prerequisites || 'N/A' }}</div>
        </div>

        <!-- Preamble -->
        <div class="bg-green-50 rounded-lg p-4">
          <div class="text-xs font-semibold text-green-600 uppercase mb-2">Preamble</div>
          <textarea
            v-if="isEditing"
            v-model="editableTest.preamble"
            rows="4"
            class="w-full border border-green-300 rounded px-3 py-2 text-sm focus:outline-none focus:border-green-500"
            placeholder="Préambule..."
          ></textarea>
          <div v-else class="text-sm text-gray-900 whitespace-pre-wrap">{{ test.preamble || 'N/A' }}</div>
        </div>

        <!-- Test Body avec étapes -->
        <div class="bg-green-50 rounded-lg p-4">
          <div class="flex items-center justify-between mb-3">
            <div class="text-xs font-semibold text-green-600 uppercase">Test Body (Étapes)</div>
            <button
              v-if="isEditing"
              @click="addTestStep"
              class="px-3 py-1 text-xs bg-green-600 hover:bg-green-700 text-white rounded flex items-center gap-1"
            >
              <svg class="w-3 h-3" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 4v16m8-8H4" />
              </svg>
              Ajouter étape
            </button>
          </div>

          <div v-if="isEditing" class="space-y-2">
            <div
              v-for="(step, idx) in testBodySteps"
              :key="idx"
              class="bg-white rounded p-3 border-l-4 border-green-500 relative group"
            >
              <div class="flex items-start justify-between gap-2">
                <div class="flex-1">
                  <div class="text-xs font-semibold text-gray-500 mb-1">Step {{ idx + 1 }}</div>
                  <textarea
                    v-model="testBodySteps[idx]"
                    rows="3"
                    class="w-full border border-gray-300 rounded px-2 py-1 text-sm focus:outline-none focus:border-green-500"
                    placeholder="Description de l'étape..."
                  ></textarea>
                </div>
                <button
                  @click="removeTestStep(idx)"
                  class="flex-shrink-0 p-1 text-red-600 hover:bg-red-100 rounded"
                  title="Supprimer l'étape"
                >
                  <svg class="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M19 7l-.867 12.142A2 2 0 0116.138 21H7.862a2 2 0 01-1.995-1.858L5 7m5 4v6m4-6v6m1-10V4a1 1 0 00-1-1h-4a1 1 0 00-1 1v3M4 7h16" />
                  </svg>
                </button>
              </div>
            </div>
          </div>
          <div v-else-if="testBodySteps.length > 0" class="space-y-2">
            <div
              v-for="(step, idx) in testBodySteps"
              :key="idx"
              class="bg-white rounded p-3 border-l-4 border-green-500"
            >
              <div class="text-xs font-semibold text-gray-500 mb-1">Step {{ idx + 1 }}</div>
              <div class="text-sm text-gray-900 whitespace-pre-wrap">{{ step }}</div>
            </div>
          </div>
          <div v-else class="text-sm text-gray-500 italic">Aucune étape définie</div>
        </div>

        <!-- Postamble -->
        <div class="bg-green-50 rounded-lg p-4">
          <div class="text-xs font-semibold text-green-600 uppercase mb-2">Postamble</div>
          <textarea
            v-if="isEditing"
            v-model="editableTest.postamble"
            rows="3"
            class="w-full border border-green-300 rounded px-3 py-2 text-sm focus:outline-none focus:border-green-500"
            placeholder="Post-traitement..."
          ></textarea>
          <div v-else class="text-sm text-gray-900 whitespace-pre-wrap">{{ test.postamble || 'N/A' }}</div>
        </div>

        <!-- Expected Result -->
        <div class="bg-green-50 rounded-lg p-4">
          <div class="text-xs font-semibold text-green-600 uppercase mb-2">Expected Result</div>
          <textarea
            v-if="isEditing"
            v-model="editableTest.expectedResult"
            rows="3"
            class="w-full border border-green-300 rounded px-3 py-2 text-sm focus:outline-none focus:border-green-500"
            placeholder="Résultat attendu..."
          ></textarea>
          <div v-else class="text-sm text-gray-900 whitespace-pre-wrap">{{ test.expectedResult || 'N/A' }}</div>
        </div>

        <!-- Comment -->
        <div class="bg-green-50 rounded-lg p-4">
          <div class="text-xs font-semibold text-green-600 uppercase mb-2">Comment</div>
          <textarea
            v-if="isEditing"
            v-model="editableTest.comment"
            rows="3"
            class="w-full border border-green-300 rounded px-3 py-2 text-sm focus:outline-none focus:border-green-500"
            placeholder="Commentaires..."
          ></textarea>
          <div v-else class="text-sm text-gray-900 whitespace-pre-wrap">{{ test.comment || 'N/A' }}</div>
        </div>
      </div>

      <!-- Actions -->
      <div v-if="!isEditing" class="border-t border-gray-200 pt-6">
        <div class="flex gap-3 flex-wrap">
          <button
            @click="confirmDelete"
            class="px-4 py-2 bg-red-600 hover:bg-red-700 text-white rounded-md transition-colors duration-150 flex items-center gap-2"
          >
            <svg class="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M19 7l-.867 12.142A2 2 0 0116.138 21H7.862a2 2 0 01-1.995-1.858L5 7m5 4v6m4-6v6m1-10V4a1 1 0 00-1-1h-4a1 1 0 00-1 1v3M4 7h16" />
            </svg>
            Supprimer
          </button>

          <button
            @click="exportTest"
            class="px-4 py-2 bg-gray-600 hover:bg-gray-700 text-white rounded-md transition-colors duration-150 flex items-center gap-2"
          >
            <svg class="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 16v1a3 3 0 003 3h10a3 3 0 003-3v-1m-4-4l-4 4m0 0l-4-4m4 4V4" />
            </svg>
            Exporter en JSON
          </button>
        </div>
      </div>
    </div>
  </div>

  <!-- État vide -->
  <div v-else class="h-full flex items-center justify-center bg-gray-50">
    <div class="text-center">
      <svg class="w-24 h-24 mx-auto mb-4 text-gray-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
        <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 12h6m-6 4h6m2 5H7a2 2 0 01-2-2V5a2 2 0 012-2h5.586a1 1 0 01.707.293l5.414 5.414a1 1 0 01.293.707V19a2 2 0 01-2 2z" />
      </svg>
      <p class="text-lg font-semibold text-gray-600 mb-2">Aucun test sélectionné</p>
      <p class="text-sm text-gray-500">Cliquez sur un élément dans l'arbre pour voir ses détails</p>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, watch, computed } from 'vue'
import type { AtpTest, TestType } from '@/types'

const props = defineProps<{
  test: AtpTest | null
}>()

const emit = defineEmits<{
  update: [test: AtpTest]
  delete: [test: AtpTest]
}>()

const isEditing = ref(false)
const editableTest = ref<Partial<AtpTest>>({})
const testBodySteps = ref<string[]>([])
const procedureSteps = ref<string[]>([])

// Initialiser les étapes quand le test change
watch(() => props.test, (newTest) => {
  if (newTest) {
    if (newTest.type === 'test-case') {
      testBodySteps.value = newTest.testBodySteps || (newTest.testBody ? [newTest.testBody] : [])
    } else if (newTest.type === 'procedure') {
      // Charger depuis procedureSteps si disponible, sinon depuis procedureBody
      procedureSteps.value = newTest.procedureSteps || (newTest.procedureBody ? [newTest.procedureBody] : [])
    }
  }
  isEditing.value = false
}, { immediate: true })

const startEditing = () => {
  if (!props.test) return

  editableTest.value = { ...props.test }

  // Copier les étapes
  if (props.test.type === 'test-case') {
    testBodySteps.value = [...(props.test.testBodySteps || (props.test.testBody ? [props.test.testBody] : []))]
  } else if (props.test.type === 'procedure') {
    // Charger depuis procedureSteps si disponible, sinon depuis procedureBody
    procedureSteps.value = [...(props.test.procedureSteps || (props.test.procedureBody ? [props.test.procedureBody] : []))]
  }

  isEditing.value = true
}

const cancelEditing = () => {
  isEditing.value = false
  editableTest.value = {}
  if (props.test) {
    if (props.test.type === 'test-case') {
      testBodySteps.value = props.test.testBodySteps || (props.test.testBody ? [props.test.testBody] : [])
    } else if (props.test.type === 'procedure') {
      // Charger depuis procedureSteps si disponible, sinon depuis procedureBody
      procedureSteps.value = props.test.procedureSteps || (props.test.procedureBody ? [props.test.procedureBody] : [])
    }
  }
}

const saveChanges = () => {
  if (!props.test) return

  const updatedTest: AtpTest = {
    ...props.test,
    ...editableTest.value,
  }

  // Mettre à jour les étapes
  if (props.test.type === 'test-case') {
    updatedTest.testBodySteps = testBodySteps.value.filter(s => s.trim())
    updatedTest.testBody = testBodySteps.value.join('\n\n')
  } else if (props.test.type === 'procedure') {
    // Sauvegarder à la fois procedureSteps (array) et procedureBody (string)
    updatedTest.procedureSteps = procedureSteps.value.filter(s => s.trim())
    updatedTest.procedureBody = procedureSteps.value.filter(s => s.trim()).join('\n\n')
  }

  emit('update', updatedTest)
  isEditing.value = false
}

const addTestStep = () => {
  testBodySteps.value.push('')
}

const removeTestStep = (index: number) => {
  testBodySteps.value.splice(index, 1)
}

const addProcedureStep = () => {
  procedureSteps.value.push('')
}

const removeProcedureStep = (index: number) => {
  procedureSteps.value.splice(index, 1)
}

const getIcon = (type: TestType): string => {
  const icons = {
    'chapter': '📚',
    'section': '📄',
    'procedure': '📋',
    'test-case': '🧪'
  }
  return icons[type] || '📝'
}

const getIconClasses = (type: TestType): string => {
  return 'text-4xl'
}

const getHeaderClasses = (type: TestType): string => {
  const classes = {
    'chapter': 'border-green-500 bg-green-50',
    'section': 'border-yellow-500 bg-yellow-50',
    'procedure': 'border-cyan-500 bg-cyan-50',
    'test-case': 'border-red-500 bg-red-50'
  }
  return classes[type] || 'border-gray-500 bg-gray-50'
}

const getBadgeClasses = (type: TestType): string => {
  const classes = {
    'chapter': 'bg-green-600 text-white',
    'section': 'bg-yellow-600 text-white',
    'procedure': 'bg-cyan-600 text-white',
    'test-case': 'bg-red-600 text-white'
  }
  return classes[type] || 'bg-gray-600 text-white'
}

const getTypeLabel = (type: TestType): string => {
  const labels = {
    'chapter': 'Chapitre',
    'section': 'Section',
    'procedure': 'Procédure',
    'test-case': 'Cas de test'
  }
  return labels[type] || type
}

const confirmDelete = () => {
  if (!props.test) return

  if (confirm(`Êtes-vous sûr de vouloir supprimer "${props.test.title}" ?`)) {
    emit('delete', props.test)
  }
}

const exportTest = () => {
  if (!props.test) return

  const json = JSON.stringify(props.test, null, 2)
  const blob = new Blob([json], { type: 'application/json' })
  const url = URL.createObjectURL(blob)

  const a = document.createElement('a')
  a.href = url
  a.download = `${props.test._id}.json`
  a.click()

  URL.revokeObjectURL(url)
}

const toggleValidation = () => {
  if (!props.test) return

  const updatedTest: AtpTest = {
    ...props.test,
    validated: !props.test.validated
  }

  emit('update', updatedTest)
}
</script>
