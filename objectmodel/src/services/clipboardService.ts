class ClipboardService {
  async copyToClipboard(text: string): Promise<boolean> {
    try {
      await navigator.clipboard.writeText(text)
      return true
    } catch (err) {
      // Fallback pour les navigateurs plus anciens
      try {
        const textArea = document.createElement('textarea')
        textArea.value = text
        textArea.style.position = 'fixed'
        textArea.style.opacity = '0'
        document.body.appendChild(textArea)
        textArea.select()
        const successful = document.execCommand('copy')
        document.body.removeChild(textArea)
        return successful
      } catch (fallbackErr) {
        console.error('Échec de la copie dans le presse-papiers:', fallbackErr)
        return false
      }
    }
  }
}

export const clipboardService = new ClipboardService()