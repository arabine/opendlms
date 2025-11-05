<template>
  <div class="h-full overflow-y-auto bg-gray-50 p-6">
    <div class="max-w-4xl mx-auto">
      <h1 class="text-3xl font-bold text-gray-900 mb-2">COSEM Date & Time Utilities</h1>
      <p class="text-gray-600 mb-6">Generate COSEM date, time, and date-time octet strings with wildcard support</p>

      <!-- Format Selection -->
      <div class="bg-white rounded-lg shadow-md p-6 mb-6">
        <h2 class="text-xl font-semibold text-gray-800 mb-4">Format Selection</h2>
        <div class="flex gap-4">
          <button
            v-for="fmt in formats"
            :key="fmt.value"
            @click="format = fmt.value"
            :class="[
              'px-6 py-3 rounded-lg font-medium transition-all',
              format === fmt.value
                ? 'bg-blue-600 text-white shadow-md'
                : 'bg-gray-100 text-gray-700 hover:bg-gray-200'
            ]"
          >
            {{ fmt.label }}
          </button>
        </div>
      </div>

      <!-- Date Configuration -->
      <div v-if="format === 'date' || format === 'date-time'" class="bg-white rounded-lg shadow-md p-6 mb-6">
        <h2 class="text-xl font-semibold text-gray-800 mb-4">📅 Date Configuration</h2>

        <!-- Year -->
        <div class="mb-4">
          <label class="block text-sm font-medium text-gray-700 mb-2">Year</label>
          <div class="flex gap-4 items-center">
            <input
              v-model.number="date.year"
              type="number"
              min="0"
              max="65535"
              :disabled="date.yearNotSpecified"
              class="flex-1 px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500 disabled:bg-gray-100"
              placeholder="Year (0-65535)"
            />
            <label class="flex items-center gap-2 cursor-pointer">
              <input v-model="date.yearNotSpecified" type="checkbox" class="w-4 h-4" />
              <span class="text-sm text-gray-700">Not specified (0xFFFF)</span>
            </label>
          </div>
        </div>

        <!-- Month -->
        <div class="mb-4">
          <label class="block text-sm font-medium text-gray-700 mb-2">Month</label>
          <select
            v-model="date.month"
            class="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
          >
            <option value="0xFF">Not specified (0xFF)</option>
            <option value="0xFE">Daylight savings begin (0xFE)</option>
            <option value="0xFD">Daylight savings end (0xFD)</option>
            <option v-for="m in 12" :key="m" :value="m">{{ monthNames[m - 1] }} ({{ m }})</option>
          </select>
        </div>

        <!-- Day of Month -->
        <div class="mb-4">
          <label class="block text-sm font-medium text-gray-700 mb-2">Day of Month</label>
          <select
            v-model="date.dayOfMonth"
            class="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
          >
            <option value="0xFF">Not specified (0xFF)</option>
            <option value="0xFE">Last day of month (0xFE)</option>
            <option value="0xFD">2nd last day of month (0xFD)</option>
            <option v-for="d in 31" :key="d" :value="d">{{ d }}</option>
          </select>
        </div>

        <!-- Day of Week -->
        <div class="mb-4">
          <label class="block text-sm font-medium text-gray-700 mb-2">Day of Week</label>
          <select
            v-model="date.dayOfWeek"
            class="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
          >
            <option value="0xFF">Not specified (0xFF)</option>
            <option v-for="(day, idx) in dayNames" :key="idx" :value="idx + 1">{{ day }} ({{ idx + 1 }})</option>
          </select>
        </div>

        <!-- Date Interpretation Help -->
        <div v-if="dateInterpretation" class="mt-4 p-4 bg-blue-50 border border-blue-200 rounded-lg">
          <div class="flex items-start gap-2">
            <svg class="w-5 h-5 text-blue-600 mt-0.5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M13 16h-1v-4h-1m1-4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z" />
            </svg>
            <div class="flex-1">
              <p class="text-sm font-medium text-blue-800">Interpretation:</p>
              <p class="text-sm text-blue-700 mt-1">{{ dateInterpretation }}</p>
            </div>
          </div>
        </div>
      </div>

      <!-- Time Configuration -->
      <div v-if="format === 'time' || format === 'date-time'" class="bg-white rounded-lg shadow-md p-6 mb-6">
        <h2 class="text-xl font-semibold text-gray-800 mb-4">🕐 Time Configuration</h2>

        <div class="grid grid-cols-2 gap-4">
          <!-- Hour -->
          <div>
            <label class="block text-sm font-medium text-gray-700 mb-2">Hour</label>
            <div class="flex gap-2">
              <input
                v-model.number="time.hour"
                type="number"
                min="0"
                max="23"
                :disabled="time.hourNotSpecified"
                class="flex-1 px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500 disabled:bg-gray-100"
                placeholder="0-23"
              />
              <label class="flex items-center gap-1 cursor-pointer text-xs">
                <input v-model="time.hourNotSpecified" type="checkbox" class="w-4 h-4" />
                <span class="text-gray-600">0xFF</span>
              </label>
            </div>
          </div>

          <!-- Minute -->
          <div>
            <label class="block text-sm font-medium text-gray-700 mb-2">Minute</label>
            <div class="flex gap-2">
              <input
                v-model.number="time.minute"
                type="number"
                min="0"
                max="59"
                :disabled="time.minuteNotSpecified"
                class="flex-1 px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500 disabled:bg-gray-100"
                placeholder="0-59"
              />
              <label class="flex items-center gap-1 cursor-pointer text-xs">
                <input v-model="time.minuteNotSpecified" type="checkbox" class="w-4 h-4" />
                <span class="text-gray-600">0xFF</span>
              </label>
            </div>
          </div>

          <!-- Second -->
          <div>
            <label class="block text-sm font-medium text-gray-700 mb-2">Second</label>
            <div class="flex gap-2">
              <input
                v-model.number="time.second"
                type="number"
                min="0"
                max="59"
                :disabled="time.secondNotSpecified"
                class="flex-1 px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500 disabled:bg-gray-100"
                placeholder="0-59"
              />
              <label class="flex items-center gap-1 cursor-pointer text-xs">
                <input v-model="time.secondNotSpecified" type="checkbox" class="w-4 h-4" />
                <span class="text-gray-600">0xFF</span>
              </label>
            </div>
          </div>

          <!-- Hundredths -->
          <div>
            <label class="block text-sm font-medium text-gray-700 mb-2">Hundredths</label>
            <div class="flex gap-2">
              <input
                v-model.number="time.hundredths"
                type="number"
                min="0"
                max="99"
                :disabled="time.hundredthsNotSpecified"
                class="flex-1 px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500 disabled:bg-gray-100"
                placeholder="0-99"
              />
              <label class="flex items-center gap-1 cursor-pointer text-xs">
                <input v-model="time.hundredthsNotSpecified" type="checkbox" class="w-4 h-4" />
                <span class="text-gray-600">0xFF</span>
              </label>
            </div>
          </div>
        </div>
      </div>

      <!-- DateTime specific: Deviation & Clock Status -->
      <div v-if="format === 'date-time'" class="bg-white rounded-lg shadow-md p-6 mb-6">
        <h2 class="text-xl font-semibold text-gray-800 mb-4">🌍 Timezone & Clock Status</h2>

        <!-- Deviation -->
        <div class="mb-4">
          <label class="block text-sm font-medium text-gray-700 mb-2">
            Deviation (minutes from UTC)
          </label>
          <div class="flex gap-4 items-center">
            <input
              v-model.number="dateTime.deviation"
              type="number"
              min="-720"
              max="720"
              :disabled="dateTime.deviationNotSpecified"
              class="flex-1 px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500 disabled:bg-gray-100"
              placeholder="-720 to +720"
            />
            <label class="flex items-center gap-2 cursor-pointer">
              <input v-model="dateTime.deviationNotSpecified" type="checkbox" class="w-4 h-4" />
              <span class="text-sm text-gray-700">Not specified (0x8000)</span>
            </label>
          </div>
          <p class="text-xs text-gray-500 mt-1">Example: UTC+1 = 60, UTC-5 = -300</p>
        </div>

        <!-- Clock Status -->
        <div>
          <label class="block text-sm font-medium text-gray-700 mb-2">Clock Status</label>
          <div class="space-y-2">
            <label class="flex items-center gap-2 cursor-pointer">
              <input v-model="dateTime.statusNotSpecified" type="checkbox" class="w-4 h-4" />
              <span class="text-sm text-gray-700">Not specified (0xFF)</span>
            </label>
            <div v-if="!dateTime.statusNotSpecified" class="ml-6 space-y-2 border-l-2 border-gray-200 pl-4">
              <label class="flex items-center gap-2 cursor-pointer">
                <input v-model="dateTime.clockStatus.invalid" type="checkbox" class="w-4 h-4" />
                <span class="text-sm text-gray-700">Invalid value (bit 0)</span>
              </label>
              <label class="flex items-center gap-2 cursor-pointer">
                <input v-model="dateTime.clockStatus.doubtful" type="checkbox" class="w-4 h-4" />
                <span class="text-sm text-gray-700">Doubtful value (bit 1)</span>
              </label>
              <label class="flex items-center gap-2 cursor-pointer">
                <input v-model="dateTime.clockStatus.differentBase" type="checkbox" class="w-4 h-4" />
                <span class="text-sm text-gray-700">Different clock base (bit 2)</span>
              </label>
              <label class="flex items-center gap-2 cursor-pointer">
                <input v-model="dateTime.clockStatus.invalidStatus" type="checkbox" class="w-4 h-4" />
                <span class="text-sm text-gray-700">Invalid clock status (bit 3)</span>
              </label>
              <label class="flex items-center gap-2 cursor-pointer">
                <input v-model="dateTime.clockStatus.daylightSaving" type="checkbox" class="w-4 h-4" />
                <span class="text-sm text-gray-700">Daylight saving active (bit 7)</span>
              </label>
            </div>
          </div>
        </div>
      </div>

      <!-- Result -->
      <div class="bg-white rounded-lg shadow-md p-6">
        <div class="flex items-center justify-between mb-4">
          <h2 class="text-xl font-semibold text-gray-800">Generated Octet String</h2>
          <button
            @click="copyToClipboard"
            class="px-4 py-2 bg-blue-600 hover:bg-blue-700 text-white rounded-md transition-colors flex items-center gap-2"
          >
            <svg class="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M8 16H6a2 2 0 01-2-2V6a2 2 0 012-2h8a2 2 0 012 2v2m-6 12h8a2 2 0 002-2v-8a2 2 0 00-2-2h-8a2 2 0 00-2 2v8a2 2 0 002 2z" />
            </svg>
            {{ copySuccess ? 'Copied!' : 'Copy' }}
          </button>
        </div>

        <div class="font-mono text-lg bg-gray-50 p-4 rounded border border-gray-300 break-all">
          {{ generatedHex }}
        </div>

        <div class="mt-4 text-sm text-gray-600">
          <p><strong>Format:</strong> {{ format }}</p>
          <p><strong>Length:</strong> {{ generatedHex.length / 2 }} bytes</p>
        </div>
      </div>

      <!-- Examples -->
      <div class="bg-white rounded-lg shadow-md p-6 mt-6">
        <h2 class="text-xl font-semibold text-gray-800 mb-4">📚 Common Examples</h2>
        <div class="space-y-2">
          <button
            v-for="(example, idx) in examples"
            :key="idx"
            @click="loadExample(example)"
            class="w-full text-left px-4 py-3 bg-gray-50 hover:bg-gray-100 rounded-lg transition-colors"
          >
            <p class="font-medium text-gray-800">{{ example.name }}</p>
            <p class="text-sm text-gray-600 mt-1">{{ example.description }}</p>
          </button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed } from 'vue'

type FormatType = 'date' | 'time' | 'date-time'

const formats = [
  { label: 'Date (5 bytes)', value: 'date' as FormatType },
  { label: 'Time (4 bytes)', value: 'time' as FormatType },
  { label: 'Date-Time (12 bytes)', value: 'date-time' as FormatType }
]

const monthNames = ['January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December']
const dayNames = ['Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday', 'Sunday']

const format = ref<FormatType>('date-time')
const copySuccess = ref(false)

// Date state
const date = ref({
  year: new Date().getFullYear(),
  yearNotSpecified: false,
  month: String(new Date().getMonth() + 1),
  dayOfMonth: String(new Date().getDate()),
  dayOfWeek: '0xFF'
})

// Time state
const time = ref({
  hour: 0,
  hourNotSpecified: false,
  minute: 0,
  minuteNotSpecified: false,
  second: 0,
  secondNotSpecified: false,
  hundredths: 0,
  hundredthsNotSpecified: true
})

// DateTime specific state
const dateTime = ref({
  deviation: 0,
  deviationNotSpecified: true,
  statusNotSpecified: true,
  clockStatus: {
    invalid: false,
    doubtful: false,
    differentBase: false,
    invalidStatus: false,
    daylightSaving: false
  }
})

// Date interpretation helper
const dateInterpretation = computed(() => {
  const y = date.value.yearNotSpecified ? 'any year' : String(date.value.year)
  const m = parseValue(date.value.month)
  const dom = parseValue(date.value.dayOfMonth)
  const dow = parseValue(date.value.dayOfWeek)

  if (m === 0xFF && dom === 0xFE && dow === 0xFF) {
    return 'Last day of the month in every year and month'
  }
  if (m === 0xFF && dom === 0xFE && dow !== 0xFF) {
    return `Last ${dayNames[dow - 1]} in every year and month`
  }
  if (y === 'any year' && m !== 0xFF && dom === 0xFE && dow === 0xFF) {
    return `Last day of ${monthNames[m - 1]} in every year`
  }
  if (y === 'any year' && m !== 0xFF && dom === 0xFE && dow !== 0xFF) {
    return `Last ${dayNames[dow - 1]} of ${monthNames[m - 1]} in every year`
  }
  if (y === 'any year' && m !== 0xFF && dom >= 1 && dom <= 31 && dow !== 0xFF) {
    return `First ${dayNames[dow - 1]} on or after ${monthNames[m - 1]} ${dom} in every year`
  }

  return null
})

// Generate hex string
const generatedHex = computed(() => {
  let hex = ''

  if (format.value === 'date' || format.value === 'date-time') {
    // Year (2 bytes)
    const year = date.value.yearNotSpecified ? 0xFFFF : date.value.year
    hex += toHex(year >> 8, 2) + toHex(year & 0xFF, 2)

    // Month (1 byte)
    hex += toHex(parseValue(date.value.month), 2)

    // Day of Month (1 byte)
    hex += toHex(parseValue(date.value.dayOfMonth), 2)

    // Day of Week (1 byte)
    hex += toHex(parseValue(date.value.dayOfWeek), 2)
  }

  if (format.value === 'time' || format.value === 'date-time') {
    // Hour (1 byte)
    hex += toHex(time.value.hourNotSpecified ? 0xFF : time.value.hour, 2)

    // Minute (1 byte)
    hex += toHex(time.value.minuteNotSpecified ? 0xFF : time.value.minute, 2)

    // Second (1 byte)
    hex += toHex(time.value.secondNotSpecified ? 0xFF : time.value.second, 2)

    // Hundredths (1 byte)
    hex += toHex(time.value.hundredthsNotSpecified ? 0xFF : time.value.hundredths, 2)
  }

  if (format.value === 'date-time') {
    // Deviation (2 bytes)
    const deviation = dateTime.value.deviationNotSpecified ? 0x8000 : (dateTime.value.deviation & 0xFFFF)
    hex += toHex(deviation >> 8, 2) + toHex(deviation & 0xFF, 2)

    // Clock status (1 byte)
    let status = 0
    if (dateTime.value.statusNotSpecified) {
      status = 0xFF
    } else {
      if (dateTime.value.clockStatus.invalid) status |= 0x01
      if (dateTime.value.clockStatus.doubtful) status |= 0x02
      if (dateTime.value.clockStatus.differentBase) status |= 0x04
      if (dateTime.value.clockStatus.invalidStatus) status |= 0x08
      if (dateTime.value.clockStatus.daylightSaving) status |= 0x80
    }
    hex += toHex(status, 2)
  }

  return hex.toUpperCase()
})

// Helper functions
const parseValue = (val: string): number => {
  if (val.startsWith('0x')) {
    return parseInt(val, 16)
  }
  return parseInt(val, 10)
}

const toHex = (val: number, width: number): string => {
  return val.toString(16).toUpperCase().padStart(width, '0')
}

const copyToClipboard = async () => {
  try {
    await navigator.clipboard.writeText(generatedHex.value)
    copySuccess.value = true
    setTimeout(() => {
      copySuccess.value = false
    }, 2000)
  } catch (err) {
    console.error('Failed to copy:', err)
  }
}

// Examples
const examples = [
  {
    name: 'Every hour at :00',
    description: 'Hourly execution at minute 00 (for use with Single Action Schedule type 2)',
    config: {
      format: 'time' as FormatType,
      time: { hourNotSpecified: true, minute: 0, second: 0, hundredthsNotSpecified: true }
    }
  },
  {
    name: 'Every 15 minutes (:00)',
    description: 'First of 4 entries - use for :00, :15, :30, :45 (requires 4 separate schedules)',
    config: {
      format: 'time' as FormatType,
      time: { hourNotSpecified: true, minute: 0, second: 0, hundredthsNotSpecified: true }
    }
  },
  {
    name: 'Every 10 minutes (:00)',
    description: 'First of 6 entries - use for :00, :10, :20, :30, :40, :50 (requires 6 schedules)',
    config: {
      format: 'time' as FormatType,
      time: { hourNotSpecified: true, minute: 0, second: 0, hundredthsNotSpecified: true }
    }
  },
  {
    name: 'Every Monday at 9:00',
    description: 'Weekly execution on Monday',
    config: {
      format: 'date-time' as FormatType,
      date: { yearNotSpecified: true, month: '0xFF', dayOfMonth: '0xFF', dayOfWeek: '1' },
      time: { hour: 9, minute: 0, second: 0, hundredthsNotSpecified: true }
    }
  },
  {
    name: 'First day of every month at 00:00',
    description: 'Monthly execution on the 1st',
    config: {
      format: 'date-time' as FormatType,
      date: { yearNotSpecified: true, month: '0xFF', dayOfMonth: '1', dayOfWeek: '0xFF' },
      time: { hour: 0, minute: 0, second: 0, hundredthsNotSpecified: true }
    }
  },
  {
    name: 'Last day of every month',
    description: 'year=0xFFFF, month=0xFF, dayOfMonth=0xFE, dayOfWeek=0xFF',
    config: {
      format: 'date' as FormatType,
      date: { yearNotSpecified: true, month: '0xFF', dayOfMonth: '0xFE', dayOfWeek: '0xFF' }
    }
  },
  {
    name: 'Last Sunday of every month',
    description: 'year=0xFFFF, month=0xFF, dayOfMonth=0xFE, dayOfWeek=7',
    config: {
      format: 'date' as FormatType,
      date: { yearNotSpecified: true, month: '0xFF', dayOfMonth: '0xFE', dayOfWeek: '7' }
    }
  },
  {
    name: 'Last Sunday in March (DST start)',
    description: 'Last Sunday in March every year at 02:00',
    config: {
      format: 'date-time' as FormatType,
      date: { yearNotSpecified: true, month: '3', dayOfMonth: '0xFE', dayOfWeek: '7' },
      time: { hour: 2, minute: 0, second: 0, hundredthsNotSpecified: true }
    }
  },
  {
    name: 'Last Sunday in October (DST end)',
    description: 'Last Sunday in October every year at 03:00',
    config: {
      format: 'date-time' as FormatType,
      date: { yearNotSpecified: true, month: '10', dayOfMonth: '0xFE', dayOfWeek: '7' },
      time: { hour: 3, minute: 0, second: 0, hundredthsNotSpecified: true }
    }
  },
  {
    name: 'Daily at midnight',
    description: 'Every day at 00:00:00',
    config: {
      format: 'date-time' as FormatType,
      date: { yearNotSpecified: true, month: '0xFF', dayOfMonth: '0xFF', dayOfWeek: '0xFF' },
      time: { hour: 0, minute: 0, second: 0, hundredthsNotSpecified: true }
    }
  },
  {
    name: 'Daily at 6:00 AM',
    description: 'Every day at 06:00:00',
    config: {
      format: 'date-time' as FormatType,
      date: { yearNotSpecified: true, month: '0xFF', dayOfMonth: '0xFF', dayOfWeek: '0xFF' },
      time: { hour: 6, minute: 0, second: 0, hundredthsNotSpecified: true }
    }
  },
  {
    name: 'Weekdays at 8:30 AM',
    description: 'Monday at 08:30 (create 5 schedules for Mon-Fri)',
    config: {
      format: 'date-time' as FormatType,
      date: { yearNotSpecified: true, month: '0xFF', dayOfMonth: '0xFF', dayOfWeek: '1' },
      time: { hour: 8, minute: 30, second: 0, hundredthsNotSpecified: true }
    }
  },
  {
    name: 'Current date-time with UTC+1',
    description: 'Complete date-time with timezone',
    config: {
      format: 'date-time' as FormatType,
      dateTime: { deviationNotSpecified: false, deviation: 60, statusNotSpecified: false }
    }
  }
]

const loadExample = (example: any) => {
  format.value = example.config.format

  if (example.config.date) {
    if (example.config.date.yearNotSpecified !== undefined) {
      date.value.yearNotSpecified = example.config.date.yearNotSpecified
    }
    if (example.config.date.month) date.value.month = example.config.date.month
    if (example.config.date.dayOfMonth) date.value.dayOfMonth = example.config.date.dayOfMonth
    if (example.config.date.dayOfWeek) date.value.dayOfWeek = example.config.date.dayOfWeek
  }

  if (example.config.time) {
    time.value.hour = example.config.time.hour ?? 0
    time.value.minute = example.config.time.minute ?? 0
    time.value.second = example.config.time.second ?? 0
    time.value.hundredthsNotSpecified = example.config.time.hundredthsNotSpecified ?? true
  }

  if (example.config.dateTime) {
    if (example.config.dateTime.deviationNotSpecified !== undefined) {
      dateTime.value.deviationNotSpecified = example.config.dateTime.deviationNotSpecified
    }
    if (example.config.dateTime.deviation !== undefined) {
      dateTime.value.deviation = example.config.dateTime.deviation
    }
    if (example.config.dateTime.statusNotSpecified !== undefined) {
      dateTime.value.statusNotSpecified = example.config.dateTime.statusNotSpecified
    }
  }
}
</script>
