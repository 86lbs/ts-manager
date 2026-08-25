<template>
  <scroller class="scroller">
    <div class="page">
      <text class="page-title">安装 / 更新</text>

      <!-- 当前状态 -->
      <div class="card">
        <text class="card-title">当前状态</text>
        <div class="row">
          <text class="label">Tailscale</text>
          <text class="value" :class="installed ? 'ok' : 'bad'">
            {{ installed ? '已安装' : '未安装' }}
          </text>
        </div>
        <div class="row">
          <text class="label">版本</text>
          <text class="value">{{ currentVersion }}</text>
        </div>
        <div class="row">
          <text class="label">后台服务</text>
          <text class="value" :class="daemonRunning ? 'ok' : 'bad'">
            {{ daemonRunning ? '运行中' : '未运行' }}
          </text>
        </div>
      </div>

      <!-- 安装 -->
      <div class="card">
        <text class="card-title">安装 / 更新</text>
        <text class="desc">从 pkgs.tailscale.com 下载安装。建议使用默认版本。</text>
        <div class="input-row">
          <text class="label">版本:</text>
          <input class="input" ref="versionInput" type="text" v-model="targetVersion" placeholder="1.98.3" />
        </div>
        <div class="btn primary" @click="doInstall">
          <text class="btn-label">{{ installing ? '安装中…' : (installed ? '更新' : '安装') }}</text>
        </div>
        <text class="hint">{{ statusText }}</text>
      </div>

      <!-- 安装日志 -->
      <div class="card" v-if="logText">
        <text class="card-title">安装日志</text>
        <text class="log">{{ logText }}</text>
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
  name: 'PageInstall',
  props: [],
  data() {
    return {
      installed: false,
      currentVersion: '—',
      daemonRunning: false,
      targetVersion: '1.98.3',
      installing: false,
      statusText: '',
      logText: '',
    }
  },
  methods: {
    onShow() {
      this.loadState()
    },
    onUnload() {},
    loadState() {
      try {
        const v = TsCtl.getVersion()
        this.installed = v !== 'not-installed' && !!v
        this.currentVersion = this.installed ? v : '未安装'
        this.daemonRunning = !!TsCtl.isDaemonRunning()
        if (this.installed) this.targetVersion = v
      } catch (e) {
        this.statusText = '读取状态失败: ' + String(e)
      }
    },
    goBack() {
      this.$falcon.navTo('index', {})
    },
    async doInstall() {
      this.installing = true
      this.logText = ''
      this.statusText = '安装中，请稍候…'
      try {
        const ver = (this.targetVersion || '').trim()
        const log = await TsCtl.installTailscale(ver || undefined)
        this.logText = log
        this.statusText = '安装流程已执行'
        this.loadState()
      } catch (e) {
        this.statusText = '错误: ' + (e && e.message ? e.message : String(e))
      } finally {
        this.installing = false
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
.label { font-size: 24px; color: @text-secondary; }
.value { font-size: 24px; color: @text-color; }
.value.ok { color: #2ecc71; }
.value.bad { color: #e74c3c; }

.input-row { flex-direction: row; align-items: center; margin-bottom: 12px; }
.input-row .label { font-size: 22px; color: @text-secondary; margin-right: 8px; }
.input { flex: 1; height: 56px; padding: 0 12px; border-radius: 6px; background-color: #1a1a1a; color: #ffffff; font-size: 20px; }

.btn { flex-direction: row; align-items: center; justify-content: center; height: 64px; border-radius: @radius-medium; background-color: @card-background-color; margin-bottom: 12px; }
.btn.primary { background-color: @primary; }
.btn-label { color: #ffffff; font-size: 28px; }
.btn:active { opacity: 0.6; }

.hint { font-size: 20px; color: @text-secondary; text-align: center; margin-top: 8px; lines: 2; }

.log { font-size: 18px; color: #2ecc71; font-family: monospace; lines: 20; }
</style>