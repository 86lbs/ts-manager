<template>
  <scroller class="scroller" direction="horizontal">
    <div class="page">
      <!-- 顶部标题 + 状态 -->
      <div class="topbar">
        <text class="app-title">Tailscale</text>
        <text class="conn" :class="up ? 'ok' : 'bad'">{{ up ? '已连接' : (daemonRunning ? '未认证' : '离线') }}</text>
      </div>

      <!-- 状态行：一行横排 -->
      <div class="status-row">
        <div class="stat">
          <text class="stat-label">IP</text>
          <text class="stat-value mono">{{ selfIp }}</text>
        </div>
        <div class="stat">
          <text class="stat-label">版本</text>
          <text class="stat-value">{{ version }}</text>
        </div>
        <div class="stat">
          <text class="stat-label">主机</text>
          <text class="stat-value">{{ selfName }}</text>
        </div>
      </div>

      <!-- 操作按钮：横排 -->
      <div class="actions">
        <div class="btn up" @click="doUp">
          <text class="btn-label">上线</text>
        </div>
        <div class="btn down" @click="confirmDown">
          <text class="btn-label">断开</text>
        </div>
        <div class="btn primary" @click="refresh">
          <text class="btn-label">刷新</text>
        </div>
      </div>

      <!-- 导航：横排 -->
      <div class="navs">
        <div class="btn" @click="goInstall">
          <text class="btn-label">安装</text>
        </div>
        <div class="btn" @click="goAuth">
          <text class="btn-label">认证</text>
        </div>
        <div class="btn" @click="goSettings">
          <text class="btn-label">设置</text>
        </div>
      </div>

      <text class="hint">{{ statusText }}</text>

      <!-- 断开确认弹窗 -->
      <modal class="confirm-modal" v-if="showConfirm" floating="true" focusable="true">
        <div class="confirm-box">
          <text class="confirm-title">确认断开？</text>
          <text class="confirm-desc">断开后远程 SSH 将不可用，只能通过 USB/ADB 或本机操作重新连接。</text>
          <div class="confirm-actions">
            <div class="btn down" @click="doDown">
              <text class="btn-label">确认断开</text>
            </div>
            <div class="btn" @click="showConfirm = false">
              <text class="btn-label">取消</text>
            </div>
          </div>
        </div>
      </modal>
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
      showConfirm: false,
    }
  },
  methods: {
    onShow() {
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
    confirmDown() {
      this.showConfirm = true
    },
    async doDown() {
      this.showConfirm = false
      this.statusText = '断开中…'
      try {
        const output = await TsCtl.runTailscale('down')
        this.statusText = output ? '断开完成' : '失败: ' + (output || '')
        this.refresh()
      } catch (e) {
        this.statusText = '错误: ' + (e && e.message ? e.message : String(e))
      }
    },
    goInstall() {
      this.$falcon.navTo('install', {})
    },
    goAuth() {
      this.$falcon.navTo('auth', {})
    },
    goSettings() {
      this.$falcon.navTo('settings', {})
    },
  },
}
</script>

<style lang="less" scoped>
@import "base.less";

.scroller {
  width: 750rpx;
  height: 100%;
  flex-direction: column;
}

.page {
  flex-direction: column;
  padding: 12px;
  background-color: @background-color;
}

.topbar {
  flex-direction: row;
  align-items: center;
  justify-content: space-between;
  padding: 4px 0 8px 0;
}

.app-title {
  font-size: 30px;
  color: @text-color;
  font-weight: bold;
}

.conn {
  font-size: 24px;
  color: @text-secondary;
}
.conn.ok { color: #2ecc71; }
.conn.bad { color: #e74c3c; }

.status-row {
  flex-direction: row;
  justify-content: space-between;
  padding: 8px;
  border-radius: @radius-medium;
  background-color: @card-background-color;
  margin-bottom: 8px;
}

.stat {
  flex: 1;
  flex-direction: column;
  align-items: center;
}

.stat-label {
  font-size: 18px;
  color: @text-secondary;
}

.stat-value {
  font-size: 22px;
  color: @text-color;
}

.stat-value.mono { font-size: 20px; }

.actions {
  flex-direction: row;
  justify-content: space-between;
  margin-bottom: 8px;
}

.navs {
  flex-direction: row;
  justify-content: space-between;
  margin-bottom: 8px;
}

.btn {
  flex: 1;
  flex-direction: row;
  align-items: center;
  justify-content: center;
  height: 56px;
  border-radius: @radius-medium;
  background-color: @card-background-color;
  margin-right: 8px;
}
.btn:last-child { margin-right: 0; }
.btn.primary { background-color: @primary; }
.btn.up { background-color: #27ae60; }
.btn.down { background-color: #c0392b; }
.btn-label { color: #ffffff; font-size: 24px; }
.btn:active { opacity: 0.6; }

.hint {
  font-size: 18px;
  color: @text-secondary;
  text-align: center;
  margin-top: 4px;
  lines: 2;
}

.confirm-modal {
  width: 750rpx;
  height: 100%;
  align-items: center;
  justify-content: center;
}

.confirm-box {
  flex-direction: column;
  padding: 24px;
  border-radius: 12px;
  background-color: #2a2a2a;
  width: 620rpx;
}

.confirm-title {
  font-size: 30px;
  color: #ffffff;
  font-weight: bold;
  margin-bottom: 12px;
}

.confirm-desc {
  font-size: 22px;
  color: #888888;
  lines: 3;
  margin-bottom: 16px;
}

.confirm-actions {
  flex-direction: row;
  justify-content: space-between;
}
</style>