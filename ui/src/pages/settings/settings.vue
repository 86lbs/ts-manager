<template>
  <scroller class="scroller">
    <div class="page">
      <text class="page-title">Tailscale 设置</text>

      <!-- Bridge 配置 -->
      <div class="card">
        <text class="card-title">Bridge 桥接器</text>
        <text class="desc">HTTP 代理转发到 Tailscale 节点（127.0.0.1:18888 → TARGET:PORT）</text>
        <div class="input-row">
          <text class="label">TARGET:</text>
          <input class="input" ref="bridgeInput" type="text" v-model="bridgeTarget" placeholder="100.x.x.x" />
        </div>
        <div class="btn primary" @click="saveBridge">
          <text class="btn-label">保存 Bridge 配置</text>
        </div>
      </div>

      <!-- 开机自启 -->
      <div class="card">
        <text class="card-title">开机自启</text>
        <text class="desc">设备启动时自动运行 Tailscale</text>
        <div class="btn" :class="autoStart ? 'down' : 'up'" @click="toggleAutostart">
          <text class="btn-label">{{ autoStart ? '已启用 (点击禁用)' : '已禁用 (点击启用)' }}</text>
        </div>
      </div>

      <!-- 版本信息 -->
      <div class="card">
        <text class="card-title">版本信息</text>
        <div class="row">
          <text class="label">Tailscale 版本</text>
          <text class="value">{{ version }}</text>
        </div>
        <div class="row">
          <text class="label">应用版本</text>
          <text class="value">0.1.0</text>
        </div>
      </div>

      <text class="hint">{{ statusText }}</text>

      <div class="btn" @click="loadSettings">
        <text class="btn-label">加载设置</text>
      </div>
      <div class="btn" @click="goBack">
        <text class="btn-label">返回首页</text>
      </div>
    </div>
  </scroller>
</template>

<script>
import { TsCtl } from 'tsctl'

export default {
  name: 'PageSettings',
  props: [],
  data() {
    return {
      bridgeTarget: '',
      autoStart: false,
      version: '—',
      statusText: '',
    }
  },
  methods: {
    onShow() {
      this.loadSettings()
    },
    onUnload() {},
    loadSettings() {
      this.statusText = '加载中…'
      try {
        this.version = TsCtl.getVersion() || '—'
        this.autoStart = !!TsCtl.isAutostartEnabled()
        const cfg = TsCtl.readConfigFile('bridge.conf')
        if (cfg) {
          const m = cfg.match(/^TARGET=(.+)$/m)
          this.bridgeTarget = m ? m[1].trim() : ''
        }
        this.statusText = '已加载'
      } catch (e) {
        this.statusText = '加载失败: ' + String(e)
      }
    },
    goBack() {
      this.$falcon.navTo('index', {})
    },
    async saveBridge() {
      this.statusText = '保存中…'
      try {
        const content = 'TARGET=' + (this.bridgeTarget || '').trim() + '\n'
        const ok = await TsCtl.writeConfigFile('bridge.conf', content)
        this.statusText = ok ? '已保存' : '保存失败'
      } catch (e) {
        this.statusText = '错误: ' + (e && e.message ? e.message : String(e))
      }
    },
    toggleAutostart() {
      try {
        const newState = !this.autoStart
        const ok = TsCtl.setAutostart(newState)
        if (ok) {
          this.autoStart = newState
          this.statusText = newState ? '开机自启已启用' : '开机自启已禁用'
        } else {
          this.statusText = '操作失败'
        }
      } catch (e) {
        this.statusText = '错误: ' + (e && e.message ? e.message : String(e))
      }
    },
  },
}
</script>

<style lang="less" scoped>
@import "base.less";

.scroller { width: 750rpx; height: 100%; }
.page { flex-direction: column; padding: 20px; background-color: @background-color; }

.page-title { font-size: 36px; color: @text-color; font-weight: bold; margin-bottom: 16px; }

.card { flex-direction: column; padding: 16px; border-radius: @radius-medium; background-color: @card-background-color; margin-bottom: 16px; }
.card-title { font-size: 28px; color: @text-color; margin-bottom: 8px; }
.desc { font-size: 22px; color: @text-secondary; margin-bottom: 12px; lines: 2; }

.row { flex-direction: row; justify-content: space-between; padding: 6px 0; }
.row .label { font-size: 24px; color: @text-secondary; }
.row .value { font-size: 24px; color: @text-color; }

.input-row { flex-direction: row; align-items: center; margin-bottom: 12px; }
.input-row .label { font-size: 22px; color: @text-secondary; margin-right: 8px; }
.input { flex: 1; height: 56px; padding: 0 12px; border-radius: 6px; background-color: #1a1a1a; color: #ffffff; font-size: 20px; }

.btn { flex-direction: row; align-items: center; justify-content: center; height: 64px; border-radius: @radius-medium; background-color: @card-background-color; margin-bottom: 12px; }
.btn.primary { background-color: @primary; }
.btn.up { background-color: #27ae60; }
.btn.down { background-color: #c0392b; }
.btn-label { color: #ffffff; font-size: 28px; }
.btn:active { opacity: 0.6; }

.hint { font-size: 20px; color: @text-secondary; text-align: center; margin-top: 8px; lines: 2; }
</style>