<template>
  <scroller class="scroller" direction="horizontal">
    <div class="page">
      <!-- 顶部标题 + 状态提示 -->
      <div class="topbar">
        <text class="app-title">Tailscale</text>
        <text class="status-tip" :class="tipClass">{{ statusText }}</text>
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
      tipClass: '',
      refreshing: false,
      showConfirm: false,
      timerId: 0,
    }
  },
  methods: {
    onShow() {
      this.refresh()
      // 定时刷新（30 秒）
      if (!this.timerId) {
        this.timerId = this.$page.setInterval(() => {
          this.refresh()
        }, 30000)
      }
    },
    onUnload() {
      if (this.timerId) {
        this.$page.clearInterval(this.timerId)
        this.timerId = 0
      }
    },
    async refresh() {
      if (this.refreshing) return
      this.refreshing = true
      this.statusText = '刷新中…'
      this.tipClass = ''
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
            // 主机名用 DNSName 短名（与手机端一致），如 ydpen-185
            const dns = self.DNSName || ''
            this.selfName = dns ? dns.split('.')[0] : (self.HostName || '—')
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
        this.statusText = this.up ? '已连接' : (this.daemonRunning ? '未认证' : '离线')
        this.tipClass = this.up ? 'ok' : 'bad'
      } catch (e) {
        this.statusText = '错误: ' + (e && e.message ? e.message : String(e))
        this.tipClass = 'bad'
      } finally {
        this.refreshing = false
      }
    },
    async doUp() {
      this.statusText = '连接中…'
      this.tipClass = ''
      try {
        const output = await TsCtl.runTailscale('up --accept-routes')
        this.statusText = output ? '上线完成' : '失败: ' + (output || '')
        this.tipClass = output ? 'ok' : 'bad'
        this.refresh()
      } catch (e) {
        this.statusText = '错误: ' + (e && e.message ? e.message : String(e))
        this.tipClass = 'bad'
      }
    },
    confirmDown() {
      this.showConfirm = true
    },
    async doDown() {
      this.showConfirm = false
      this.statusText = '断开中…'
      this.tipClass = ''
      try {
        const output = await TsCtl.runTailscale('down')
        this.statusText = output ? '已断开' : '失败: ' + (output || '')
        this.tipClass = output ? 'bad' : 'bad'
        this.refresh()
      } catch (e) {
        this.statusText = '错误: ' + (e && e.message ? e.message : String(e))
        this.tipClass = 'bad'
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
  flex: 1;
  font-size: 28px;
  color: @text-color;
  font-weight: bold;
}

.status-tip {
  font-size: 20px;
  color: @text-secondary;
}
.status-tip.ok { color: #2ecc71; }
.status-tip.bad { color: #e74c3c; }

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
  font-size: 16px;
  color: @text-secondary;
}

.stat-value {
  font-size: 20px;
  color: @text-color;
}

.stat-value.mono { font-size: 18px; }

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
  height: 44px;
  border-radius: @radius-medium;
  background-color: @card-background-color;
  margin-right: 8px;
}
.btn:last-child { margin-right: 0; }
.btn.primary { background-color: @primary; }
.btn.up { background-color: #27ae60; }
.btn.down { background-color: #c0392b; }
.btn-label { color: #ffffff; font-size: 22px; }
.btn:active { opacity: 0.6; }

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
  font-size: 28px;
  color: #ffffff;
  font-weight: bold;
  margin-bottom: 8px;
}

.confirm-desc {
  font-size: 20px;
  color: #888888;
  lines: 3;
  margin-bottom: 16px;
}

.confirm-actions {
  flex-direction: row;
  justify-content: space-between;
}
</style>