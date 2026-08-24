<template>
  <scroller class="scroller">
    <div class="page">
      <text class="app-title">Tailscale Manager</text>

      <div class="card">
        <div class="row">
          <text class="label">运行状态</text>
          <text class="value" :class="daemonRunning ? 'ok' : 'bad'">
            {{ daemonRunning ? '运行中' : '未运行' }}
          </text>
        </div>
        <div class="row">
          <text class="label">版本</text>
          <text class="value">{{ version }}</text>
        </div>
        <div class="row">
          <text class="label">本机 IP</text>
          <text class="value mono">{{ selfIp }}</text>
        </div>
        <div class="row">
          <text class="label">主机名</text>
          <text class="value">{{ selfName }}</text>
        </div>
      </div>

      <div class="btn primary" @click="refresh">
        <text class="btn-label">刷新状态</text>
      </div>
      <div class="btn up" @click="doUp">
        <text class="btn-label">上线 (up)</text>
      </div>
      <div class="btn down" @click="doDown">
        <text class="btn-label">断开 (down)</text>
      </div>

      <text class="hint">{{ statusText }}</text>
    </div>
  </scroller>
</template>

<script>
import { TsCtl } from 'tsctl'

export default {
  name: 'PageIndex',
  props: [],
  data() {
    return {
      daemonRunning: false,
      version: '—',
      selfIp: '—',
      selfName: '—',
      up: false,
      statusText: '',
      refreshing: false,
    }
  },
  methods: {
    onShow() {
      // 诊断：测试 popen 在同步线程是否工作，结果放状态栏
      try {
        const test = TsCtl.testPopen()
        this.statusText = 'popen: ' + test
      } catch(e) {
        this.statusText = 'popen err: ' + String(e)
      }
      this.refresh()
    },
    onUnload() {},
    async refresh() {
      if (this.refreshing) return
      this.refreshing = true
      this.statusText = '刷新中…'
      try {
        this.daemonRunning = !!TsCtl.isDaemonRunning()
        this.version = TsCtl.getVersion() || '未安装'
        const raw = await TsCtl.runTailscale('status --json')
        if (raw) {
          try {
            const j = JSON.parse(raw)
            const self = j.Self || {}
            const ips = j.Self && j.Self.TailscaleIPs ? j.Self.TailscaleIPs : []
            this.selfIp = ips.length ? ips[0] : '—'
            this.selfName = self.HostName || '—'
            const state = self.BackendState || ''
            this.up = state === 'Running' || ips.length > 0
          } catch (e) {
            this.selfIp = '—'
            this.selfName = '—'
            this.up = false
          }
        } else {
          this.up = false
        }
        this.statusText = '已刷新'
      } catch (e) {
        this.statusText = '错误: ' + (e && e.message ? e.message : String(e))
      } finally {
        this.refreshing = false
      }
    },
    async doUp() {
      this.statusText = '连接中…'
      try {
        const output = await TsCtl.runTailscale('up --accept-routes')
        this.statusText = output ? '上线完成' : '失败: ' + (output || '')
        this.refresh()
      } catch (e) {
        this.statusText = '错误: ' + (e && e.message ? e.message : String(e))
      }
    },
    async doDown() {
      this.statusText = '断开中…'
      try {
        const output = await TsCtl.runTailscale('down')
        this.statusText = output ? '断开完成' : '失败: ' + (output || '')
        this.refresh()
      } catch (e) {
        this.statusText = '错误: ' + (e && e.message ? e.message : String(e))
      }
    },
  },
}
</script>

<style lang="less" scoped>
@import "base.less";

.scroller {
  width: 750rpx;
  height: 100%;
}

.page {
  flex-direction: column;
  padding: 20px;
  background-color: @background-color;
}

.app-title {
  font-size: 36px;
  color: @text-color;
  font-weight: bold;
  margin-bottom: 16px;
}

.card {
  flex-direction: column;
  padding: 16px;
  border-radius: @radius-medium;
  background-color: @card-background-color;
  margin-bottom: 16px;
}

.row {
  flex-direction: row;
  justify-content: space-between;
  padding: 6px 0;
}

.label {
  font-size: 24px;
  color: @text-secondary;
}

.value {
  font-size: 24px;
  color: @text-color;
}

.value.ok { color: #2ecc71; }
.value.bad { color: #e74c3c; }
.value.mono { font-size: 22px; }

.btn {
  flex-direction: row;
  align-items: center;
  justify-content: center;
  height: 64px;
  border-radius: @radius-medium;
  background-color: @card-background-color;
  margin-bottom: 12px;
}

.btn-label {
  color: #ffffff;
  font-size: 28px;
}

.btn.primary {
  background-color: @primary;
}

.btn.up {
  background-color: #27ae60;
}

.btn.down {
  background-color: #c0392b;
}

.btn:active {
  opacity: 0.6;
}

.hint {
  font-size: 20px;
  color: @text-secondary;
  text-align: center;
  margin-top: 8px;
  lines: 2;
}
</style>