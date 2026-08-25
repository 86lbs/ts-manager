<template>
  <scroller class="scroller">
    <div class="page">
      <text class="page-title">Tailscale 设置</text>

      <!-- Bridge 配置：节点列表选择 -->
      <div class="card">
        <text class="card-title">Bridge 目标</text>
        <text class="desc">选一个节点作为桥接目标（127.0.0.1:18888 → 目标IP）</text>

        <div class="current-row">
          <text class="label">当前:</text>
          <text class="value" :class="bridgeTarget ? 'ok' : 'bad'">{{ bridgeTarget || '未设置' }}</text>
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
            <text class="node-name" :class="peer.ip === bridgeTarget ? 'selected' : ''">{{ peer.name }}</text>
            <text class="node-ip">{{ peer.ip }}</text>
          </div>
          <text class="empty" v-if="!onlinePeers.length">无在线节点</text>
        </div>

        <text class="list-title">离线节点</text>
        <div class="node-list">
          <div
            v-for="(peer, idx) in offlinePeers"
            :key="'x' + idx"
            class="node-item"
            :class="{ selected: peer.ip === bridgeTarget }"
            @click="selectPeer(peer)"
          >
            <text class="node-name" :class="peer.ip === bridgeTarget ? 'selected' : ''">{{ peer.name }}</text>
            <text class="node-ip">{{ peer.ip }}</text>
          </div>
          <text class="empty" v-if="!offlinePeers.length">无离线节点</text>
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
        <text class="btn-label">重新加载</text>
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
      onlinePeers: [],
      offlinePeers: [],
      loadingPeers: false,
    }
  },
  methods: {
    onShow() {
      this.loadSettings()
    },
    onUnload() {},
    async loadSettings() {
      this.statusText = '加载中…'
      try {
        this.version = TsCtl.getVersion() || '—'
        this.autoStart = !!TsCtl.isAutostartEnabled()
        const cfg = TsCtl.readConfigFile('bridge.conf')
        if (cfg) {
          const m = cfg.match(/^TARGET=(.+)$/m)
          this.bridgeTarget = m ? m[1].trim() : ''
        }
        // 加载节点列表
        await this.loadPeers()
        this.statusText = '已加载'
      } catch (e) {
        this.statusText = '加载失败: ' + String(e)
      }
    },
    async loadPeers() {
      if (this.loadingPeers) return
      this.loadingPeers = true
      try {
        const raw = await TsCtl.runTailscale('status --json')
        const online = []
        const offline = []
        if (raw) {
          const j = JSON.parse(raw)
          const peers = j.Peer || {}
          Object.keys(peers).forEach((key) => {
            const p = peers[key]
            const ips = p.TailscaleIPs || []
            if (!ips.length) return
            const name = p.HostName || key.substring(0, 12)
            const entry = { name, ip: ips[0], online: !!p.Online }
            if (p.Online) online.push(entry)
            else offline.push(entry)
          })
          online.sort((a, b) => a.name.localeCompare(b.name))
          offline.sort((a, b) => a.name.localeCompare(b.name))
        }
        this.onlinePeers = online
        this.offlinePeers = offline
      } catch (e) {
        this.onlinePeers = []
        this.offlinePeers = []
      } finally {
        this.loadingPeers = false
      }
    },
    async selectPeer(peer) {
      this.bridgeTarget = peer.ip
      this.statusText = '保存 ' + peer.name + ' (' + peer.ip + ')…'
      try {
        const content = 'TARGET=' + peer.ip + '\n'
        const ok = await TsCtl.writeConfigFile('bridge.conf', content)
        this.statusText = ok ? '已设为 ' + peer.name : '保存失败'
      } catch (e) {
        this.statusText = '错误: ' + String(e)
      }
    },
    goBack() {
      this.$falcon.navTo('index', {})
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

.current-row { flex-direction: row; justify-content: space-between; padding: 6px 0; margin-bottom: 8px; }
.current-row .label { font-size: 24px; color: @text-secondary; }
.current-row .value { font-size: 24px; color: @text-color; }
.current-row .value.ok { color: #2ecc71; }
.current-row .value.bad { color: #e74c3c; }

.list-title { font-size: 22px; color: @text-secondary; margin: 8px 0 6px 0; }
.node-list { flex-direction: column; }
.node-item {
  flex-direction: row;
  justify-content: space-between;
  align-items: center;
  padding: 10px 12px;
  border-radius: 6px;
  margin-bottom: 4px;
}
.node-item:active { background-color: #3a3a3a; }
.node-item.selected { background-color: rgba(16, 142, 233, 0.3); }
.node-name { font-size: 24px; color: @text-color; }
.node-name.selected { color: @primary; }
.node-ip { font-size: 20px; color: @text-secondary; }
.empty { font-size: 22px; color: @text-secondary; padding: 8px 0; }

.row { flex-direction: row; justify-content: space-between; padding: 6px 0; }
.row .label { font-size: 24px; color: @text-secondary; }
.row .value { font-size: 24px; color: @text-color; }

.btn { flex-direction: row; align-items: center; justify-content: center; height: 64px; border-radius: @radius-medium; background-color: @card-background-color; margin-bottom: 12px; }
.btn.primary { background-color: @primary; }
.btn.up { background-color: #27ae60; }
.btn.down { background-color: #c0392b; }
.btn-label { color: #ffffff; font-size: 28px; }
.btn:active { opacity: 0.6; }

.hint { font-size: 20px; color: @text-secondary; text-align: center; margin-top: 8px; lines: 2; }
</style>