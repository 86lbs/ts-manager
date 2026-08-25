<template>
  <div class="app">
    <!-- 左侧导航栏 -->
    <div class="sidebar">
      <div class="logo">
        <text class="logo-text">TS</text>
      </div>
      <div
        v-for="item in navItems"
        :key="item.id"
        class="nav-item"
        :class="currentTab === item.id ? 'active' : ''"
        @click="switchTab(item.id)"
      >
        <text class="nav-label" :class="currentTab === item.id ? 'active' : ''">{{ item.label }}</text>
      </div>
      <div class="sidebar-bottom">
        <div class="nav-item mini" :class="up ? 'up' : 'down'" @click="currentTab === 'home' ? toggleQuick() : switchTab('home')">
          <text class="nav-label mini">{{ up ? '断开' : '上线' }}</text>
        </div>
      </div>
    </div>

    <!-- 右侧内容区 -->
    <div class="content">
      <!-- ===== 首页 ===== -->
      <div v-if="currentTab === 'home'" class="tab-page">
        <div class="topbar">
          <text class="app-title">Tailscale</text>
          <text class="status-tip" :class="tipClass">{{ statusText }}</text>
        </div>

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

        <div class="actions">
          <div class="btn up" @click="doUp">
            <text class="btn-label">上线</text>
          </div>
          <div class="btn down" @click="confirmDown">
            <text class="btn-label">断开</text>
          </div>
        </div>
      </div>

      <!-- ===== 安装 ===== -->
      <div v-if="currentTab === 'install'" class="tab-page">
        <div class="topbar">
          <text class="app-title">安装 / 更新</text>
        </div>

        <div class="card">
          <div class="row">
            <text class="row-label">Tailscale</text>
            <text class="row-value" :class="installed ? 'ok' : 'bad'">{{ installed ? '已安装' : '未安装' }}</text>
          </div>
          <div class="row">
            <text class="row-label">当前版本</text>
            <text class="row-value">{{ currentVersion }}</text>
          </div>
          <div class="row">
            <text class="row-label">最新版本</text>
            <text class="row-value" :class="latestVersion ? 'ok' : 'bad'">{{ latestVersion || '获取失败' }}</text>
          </div>
          <div class="row">
            <text class="row-label">后台服务</text>
            <text class="row-value" :class="daemonRunning ? 'ok' : 'bad'">{{ daemonRunning ? '运行中' : '未运行' }}</text>
          </div>
        </div>

        <div class="btn primary" @click="doInstall">
          <text class="btn-label">{{ installing ? '安装中…' : (installed ? '更新到最新版' : '安装最新版') }}</text>
        </div>

        <div class="card" v-if="installLog">
          <text class="card-title">安装日志</text>
          <text class="log">{{ installLog }}</text>
        </div>
      </div>

      <!-- ===== 认证 ===== -->
      <div v-if="currentTab === 'auth'" class="tab-page">
        <div class="topbar">
          <text class="app-title">扫码认证</text>
        </div>

        <div class="card">
          <text class="desc">生成二维码，用手机 Tailscale 扫码</text>
          <div class="qrcode-wrap" v-if="authUrl">
            <qrcode class="qrcode" :value="authUrl" level="Q" color="black" />
          </div>
          <div class="btn primary" @click="genAuthUrl">
            <text class="btn-label">{{ authLoading ? '生成中…' : '生成二维码' }}</text>
          </div>
          <text class="val" v-if="authUrl && !authLoading">{{ authUrl }}</text>
        </div>
      </div>

      <!-- ===== 设置 ===== -->
      <div v-if="currentTab === 'settings'" class="tab-page">
        <div class="topbar">
          <text class="app-title">设置</text>
          <text class="status-tip" :class="tipClass">{{ statusText }}</text>
        </div>

        <div class="card">
          <text class="card-title">Bridge 目标</text>
          <div class="row">
            <text class="row-label">当前:</text>
            <text class="row-value" :class="bridgeTarget ? 'ok' : 'bad'">{{ bridgeTarget || '未设置' }}</text>
          </div>
          <text class="list-title">在线节点</text>
          <div class="node-list">
            <div
              v-for="(peer, idx) in onlinePeers"
              :key="'o' + idx"
              class="node-item"
              :class="{ selected: peer.ip === bridgeTarget }"
              @click="selectPeer(peer)"
            >
              <text class="node-name">{{ peer.name }}</text>
              <text class="node-ip">{{ peer.ip }}</text>
            </div>
            <text class="empty" v-if="!onlinePeers.length">无在线节点</text>
          </div>
        </div>

        <div class="card">
          <div class="row">
            <text class="row-label">开机自启</text>
            <text class="row-value" :class="autoStart ? 'ok' : 'bad'">{{ autoStart ? '已启用' : '已禁用' }}</text>
          </div>
        </div>
      </div>
    </div>

    <!-- 断开确认弹窗 -->
    <modal class="confirm-modal" v-if="showConfirm" floating="true" focusable="true">
      <div class="confirm-box">
        <text class="confirm-title">确认断开？</text>
        <text class="confirm-desc">断开后远程 SSH 将不可用。</text>
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
</template>

<script>
import { TsCtl } from 'tsctl'

export default {
  name: 'PageIndex',
  props: [],
  data() {
    return {
      currentTab: 'home',
      navItems: [
        { id: 'home', label: '首页' },
        { id: 'install', label: '安装' },
        { id: 'auth', label: '认证' },
        { id: 'settings', label: '设置' },
      ],
      // 首页状态
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
      // 安装
      installed: false,
      currentVersion: '—',
      latestVersion: '',
      installing: false,
      installLog: '',
      // 认证
      authUrl: '',
      authLoading: false,
      // 设置
      bridgeTarget: '',
      autoStart: false,
      onlinePeers: [],
    }
  },
  methods: {
    onShow() {
      this.refresh()
      this.loadInstallState()
      this.fetchLatestVersion()
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
    switchTab(id) {
      this.currentTab = id
      if (id === 'settings') this.loadPeers()
      if (id === 'install') {
        this.loadInstallState()
        this.fetchLatestVersion()
      }
    },
    toggleQuick() {
      this.up ? this.confirmDown() : this.doUp()
    },

    // ===== 首页 =====
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
        this.tipClass = 'bad'
        this.refresh()
      } catch (e) {
        this.statusText = '错误: ' + (e && e.message ? e.message : String(e))
        this.tipClass = 'bad'
      }
    },

    // ===== 安装 =====
    loadInstallState() {
      try {
        const v = TsCtl.getVersion()
        this.installed = v !== 'not-installed' && !!v
        this.currentVersion = this.installed ? v : '未安装'
        this.daemonRunning = !!TsCtl.isDaemonRunning()
      } catch (e) {}
    },
    async fetchLatestVersion() {
      try {
        const v = await TsCtl.getLatestVersion()
        if (v) this.latestVersion = v
      } catch (e) {
        this.latestVersion = ''
      }
    },
    async doInstall() {
      this.installing = true
      this.installLog = ''
      this.statusText = '安装中，请稍候…'
      try {
        const log = await TsCtl.installTailscale()
        this.installLog = log
        this.statusText = '安装流程已执行'
        this.loadInstallState()
      } catch (e) {
        this.statusText = '错误: ' + (e && e.message ? e.message : String(e))
      } finally {
        this.installing = false
      }
    },

    // ===== 认证 =====
    async genAuthUrl() {
      this.authLoading = true
      this.statusText = '检查状态…'
      this.authUrl = ''
      try {
        const statusRaw = await TsCtl.runTailscale('status --json')
        let online = false
        if (statusRaw) {
          try {
            const sj = JSON.parse(statusRaw)
            const sstate = (sj.Self && sj.Self.BackendState) || sj.BackendState || ''
            const ips = sj.Self && sj.Self.TailscaleIPs ? sj.Self.TailscaleIPs : []
            if (sstate === 'Running' && ips.length > 0) {
              online = true
              this.statusText = '已在线 (IP: ' + ips[0] + ')，无需认证'
            }
          } catch (e) {}
        }
        if (online) return
        this.statusText = '生成二维码…'
        const url = await TsCtl.getAuthUrl()
        if (url) {
          this.authUrl = url
          this.statusText = '请用手机 Tailscale 扫码'
        } else {
          this.statusText = '未获取到认证链接'
        }
      } catch (e) {
        this.statusText = '错误: ' + (e && e.message ? e.message : String(e))
      } finally {
        this.authLoading = false
      }
    },

    // ===== 设置 =====
    async loadPeers() {
      try {
        this.bridgeTarget = ''
        const cfg = TsCtl.readConfigFile('bridge.conf')
        if (cfg) {
          const m = cfg.match(/^TARGET=(.+)$/m)
          this.bridgeTarget = m ? m[1].trim() : ''
        }
        this.autoStart = !!TsCtl.isAutostartEnabled()
        const raw = await TsCtl.runTailscale('status --json')
        const online = []
        if (raw) {
          const j = JSON.parse(raw)
          const self = j.Self || {}
          const selfIps = self.TailscaleIPs || []
          const peers = j.Peer || {}
          Object.keys(peers).forEach((key) => {
            const p = peers[key]
            const ips = p.TailscaleIPs || []
            if (!ips.length) return
            if (selfIps.indexOf(ips[0]) !== -1) return
            const dns = (p.DNSName || '').split('.')[0]
            const name = dns || p.HostName || key.substring(0, 12)
            if (p.Online) online.push({ name, ip: ips[0] })
          })
          online.sort((a, b) => a.name.localeCompare(b.name))
        }
        this.onlinePeers = online
      } catch (e) {}
    },
    async selectPeer(peer) {
      this.bridgeTarget = peer.ip
      this.statusText = '保存 ' + peer.name + '…'
      try {
        const content = 'TARGET=' + peer.ip + '\n'
        const ok = await TsCtl.writeConfigFile('bridge.conf', content)
        this.statusText = ok ? '已设为 ' + peer.name : '保存失败'
        this.tipClass = ok ? 'ok' : 'bad'
      } catch (e) {
        this.statusText = '错误: ' + String(e)
      }
    },
  },
}
</script>

<style lang="less" scoped>
@import "base.less";

.app {
  flex: 1;
  flex-direction: row;
  background-color: @background-color;
}

/* 侧边栏 */
.sidebar {
  width: 120px;
  flex-direction: column;
  background-color: #222222;
  padding: 8px 0;
}

.logo {
  align-items: center;
  padding: 4px 0 10px 0;
}

.logo-text {
  font-size: 26px;
  color: @primary;
  font-weight: bold;
}

.nav-item {
  align-items: center;
  justify-content: center;
  padding: 12px 0;
  margin: 2px 8px;
  border-radius: 8px;
}

.nav-item:active {
  background-color: #3a3a3a;
}

.nav-item.active {
  background-color: @primary;
}

.nav-label {
  font-size: 22px;
  color: @text-secondary;
}

.nav-label.active {
  color: #ffffff;
}

.sidebar-bottom {
  flex: 1;
  flex-direction: column;
  justify-content: flex-end;
}

.nav-item.mini {
  background-color: #2a2a2a;
}

.nav-item.mini.up { background-color: #27ae60; }
.nav-item.mini.down { background-color: #c0392b; }

.nav-label.mini {
  font-size: 20px;
  color: #ffffff;
}

/* 内容区 */
.content {
  flex: 1;
  flex-direction: column;
  padding: 10px;
}

.tab-page {
  flex-direction: column;
}

.topbar {
  flex-direction: row;
  align-items: center;
  justify-content: space-between;
  padding: 2px 0 8px 0;
}

.app-title {
  font-size: 28px;
  color: @text-color;
  font-weight: bold;
}

.status-tip {
  font-size: 18px;
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

.stat { flex: 1; flex-direction: column; align-items: center; }
.stat-label { font-size: 16px; color: @text-secondary; }
.stat-value { font-size: 22px; color: @text-color; }
.stat-value.mono { font-size: 20px; }

.actions {
  flex-direction: row;
  justify-content: space-between;
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
.btn-label { color: #ffffff; font-size: 22px; }
.btn:active { opacity: 0.6; }

.card {
  flex-direction: column;
  padding: 12px;
  border-radius: @radius-medium;
  background-color: @card-background-color;
  margin-bottom: 8px;
}
.card-title { font-size: 24px; color: @text-color; margin-bottom: 6px; }
.desc { font-size: 18px; color: @text-secondary; margin-bottom: 8px; lines: 2; }

.row { flex-direction: row; justify-content: space-between; padding: 4px 0; }
.row-label { font-size: 20px; color: @text-secondary; }
.row-value { font-size: 20px; color: @text-color; }
.row-value.ok { color: #2ecc71; }
.row-value.bad { color: #e74c3c; }

.list-title { font-size: 18px; color: @text-secondary; margin: 6px 0 4px 0; }
.node-list { flex-direction: column; }
.node-item {
  flex-direction: row;
  justify-content: space-between;
  align-items: center;
  padding: 6px 8px;
  border-radius: 6px;
}
.node-item:active { background-color: #3a3a3a; }
.node-item.selected { background-color: rgba(16, 142, 233, 0.3); }
.node-name { font-size: 20px; color: @text-color; }
.node-ip { font-size: 18px; color: @text-secondary; }
.empty { font-size: 18px; color: @text-secondary; padding: 6px 0; }

.qrcode-wrap { align-items: center; justify-content: center; padding: 10px; }
.qrcode { width: 160px; height: 160px; }
.val { font-size: 16px; color: @text-secondary; margin-top: 6px; lines: 3; }

.log { font-size: 16px; color: #2ecc71; font-family: monospace; lines: 15; }

.confirm-modal {
  width: 750rpx;
  height: 100%;
  align-items: center;
  justify-content: center;
}
.confirm-box {
  flex-direction: column;
  padding: 20px;
  border-radius: 12px;
  background-color: #2a2a2a;
  width: 580rpx;
}
.confirm-title { font-size: 26px; color: #ffffff; font-weight: bold; margin-bottom: 10px; }
.confirm-desc { font-size: 18px; color: #888888; lines: 3; margin-bottom: 12px; }
.confirm-actions { flex-direction: row; justify-content: space-between; }
</style>