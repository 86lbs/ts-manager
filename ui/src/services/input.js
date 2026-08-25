// 系统输入法适配层：封装 global.startTextEdit 状态机
// 用法：
//   import { showTextEdit, closeTextEditSession } from '../../services/input.js'
//   await showTextEdit({ text, placeholder, maxlength, inputType })
//   返回用户确认的文本；取消返回 null

import globalModule from 'global'

let manager = null
let handlerAttached = false
let currentUuid = null
let pendingResolve = null

function getInputManager() {
  if (!manager) manager = new globalModule.Global()
  return manager
}

function normalizeText(value) {
  if (value && typeof value === 'object') {
    if (typeof value.value === 'string') return value.value
    if (typeof value.text === 'string') return value.text
  }
  return typeof value === 'string' ? value : ''
}

// 挂一次全局 handler（整个应用生命周期）
function ensureHandler() {
  if (handlerAttached) return
  const g = getInputManager()
  if (!g.textEditFinished) return
  g.textEditFinished.on((result) => {
    if (!pendingResolve) return
    const resolve = pendingResolve
    pendingResolve = null
    try {
      const parsed = typeof result === 'string' ? JSON.parse(result) : result
      // 只处理当前会话的 UUID
      if (parsed.uuid && currentUuid && parsed.uuid !== currentUuid) {
        // 不是当前会话，忽略（避免旧回调污染）
        return
      }
      if (parsed.editConfirmed === true) {
        const text = normalizeText(parsed.text || parsed.value)
        resolve({ ok: true, text, uuid: parsed.uuid })
      } else {
        resolve({ ok: false, cancelled: true, uuid: parsed.uuid })
      }
    } catch (e) {
      resolve({ ok: false, cancelled: true, error: String(e) })
    }
  })
  handlerAttached = true
}

// 打开输入法，返回用户确认的文本
// config: { text, placeholder, maxlength, inputType, autofocus, enterButtonText }
export function showTextEdit(config = {}) {
  ensureHandler()
  const g = getInputManager()
  if (!g || !g.startTextEdit) {
    return Promise.resolve({ ok: false, error: '输入法不可用' })
  }

  // 关闭旧会话
  if (currentUuid) {
    try { g.closeTextEdit(currentUuid) } catch (e) {}
    currentUuid = null
  }

  // 拼配置
  const cfg = {
    text: config.text || '',
    placeholder: config.placeholder || '',
    maxlength: config.maxlength || 64,
    inputType: config.inputType || 'EnUSPreferred',
    autofocus: true,
    showCursor: true,
    cursorColor: '#108ee9',
    cursorSize: 2,
    multiLinesEditVisible: false,
    enterButtonText: config.enterButtonText || '确定',
    ...config,
  }

  return new Promise((resolve) => {
    pendingResolve = resolve
    try {
      const uuid = g.startTextEdit(JSON.stringify(cfg))
      currentUuid = uuid
      // 超时保护（30 秒无响应视为取消）
      setTimeout(() => {
        if (pendingResolve) {
          const r = pendingResolve
          pendingResolve = null
          r({ ok: false, cancelled: true, timeout: true })
        }
      }, 30000)
    } catch (e) {
      pendingResolve = null
      resolve({ ok: false, error: String(e) })
    }
  })
}

// 手动关闭当前会话
export function closeTextEditSession() {
  const g = getInputManager()
  if (currentUuid && g && g.closeTextEdit) {
    try { g.closeTextEdit(currentUuid) } catch (e) {}
  }
  currentUuid = null
  if (pendingResolve) {
    const r = pendingResolve
    pendingResolve = null
    r({ ok: false, cancelled: true })
  }
}

// 页面销毁时清理
export function cleanupInput() {
  closeTextEditSession()
  const g = getInputManager()
  if (g && g.textEditFinished && handlerAttached) {
    // 挂载期管理由 BasePage 的 on/off 负责，这里只重置状态
  }
  handlerAttached = false
}
