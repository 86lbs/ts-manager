<template>
  <scroller class="scroller">
    <div class="page">
      <text class="page-title">Tailscale 认证</text>

      <!-- 二维码认证 -->
      <div class="card">
        <text class="card-title">扫码认证</text>
        <text class="desc">点击下方按钮生成登录二维码，用手机 Tailscale 扫码</text>
        <div class="qrcode-wrap" v-if="authUrl">
          <qrcode ref="qrRef" class="qrcode" :value="authUrl" level="Q" color="black" />
        </div>
        <div class="btn primary" @click="genAuthUrl">
          <text class="btn-label">{{ loading ? '生成中…' : '生成二维码' }}</text>
        </div>
        <text class="val" v-if="authUrl && !loading">{{ authUrl }}</text>
        <text class="val warn" v-if="authUrl && !loading && qrFailed">（二维码未渲染，请复制上方链接到手机浏览器打开）</text>
      </div>

      <!-- Auth Key 输入（默认折叠，高级选项） -->
      <div class="card">
        <div class="adv-toggle" @click="showAuthKey = !showAuthKey">
          <text class="card-title">Auth Key 认证</text>
          <text class="adv-hint">{{ showAuthKey ? '收起 ▲' : '展开 ▼' }}</text>
        </div>
        <div v-if="showAuthKey">
          <text class="desc">输入 Tailscale 控制台生成的预认证密钥（无手机扫码时使用）</text>
          <div class="input-row">
            <text class="label">Auth Key:</text>
            <input class="input" ref="authKeyInput" type="text" v-model="authKey" placeholder="tskey-auth-..." />
          </div>
          <div class="btn primary" @click="doAuthKey">
            <text class="btn-label">{{ loading ? '连接中…' : '认证并连接' }}</text>
          </div>
        </div>
      </div>

      <text class="hint">{{ statusText }}</text>

      <div class="btn" @click="goBack">
        <text class="btn-label">返回首页</text>
      </div>
    </div>
  </scroller>
</template>

<script>
import { TsCtl } from 'tsctl'

export default {
  name: 'PageAuth',
  props: [],
  data() {
    return {
      authUrl: '',
      authKey: '',
      showAuthKey: false,
      loading: false,
      qrFailed: false,
      statusText: '',
    }
  },
  methods: {
    onShow() {},
    onUnload() {},
    goBack() {
      this.$falcon.navTo('index', {})
    },
    async genAuthUrl() {
      this.loading = true
      this.qrFailed = false
      this.statusText = '检查状态…'
      this.authUrl = ''
      try {
        // 先查状态，若已在线则无需认证
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
        if (online) {
          this.loading = false
          return
        }
        // 未在线：后台 up 获取 AuthURL（不等 up 退出，避免阻塞）
        this.statusText = '生成二维码…'
        const url = await TsCtl.getAuthUrl()
        if (url) {
          this.authUrl = url
          this.statusText = '请用手机 Tailscale 扫码'
        } else {
          this.statusText = '未获取到认证链接（可能已在线或超时）'
        }
      } catch (e) {
        this.statusText = '错误: ' + (e && e.message ? e.message : String(e))
      } finally {
        this.loading = false
      }
    },
    async doAuthKey() {
      const key = (this.authKey || '').trim()
      if (!key) {
        this.statusText = '请输入 Auth Key'
        return
      }
      this.loading = true
      this.statusText = '连接中…'
      try {
        const raw = await TsCtl.runTailscale('up --auth-key="' + key + '" --accept-routes')
        this.statusText = raw ? '认证成功: ' + raw.substring(0, 80) : '认证成功（无输出）'
        this.authKey = ''
      } catch (e) {
        this.statusText = '错误: ' + (e && e.message ? e.message : String(e))
      } finally {
        this.loading = false
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

.adv-toggle { flex-direction: row; align-items: center; justify-content: space-between; }
.adv-hint { font-size: 20px; color: @text-secondary; }

.qrcode-wrap { align-items: center; justify-content: center; padding: 16px; }
.qrcode { width: 200px; height: 200px; }

.val { font-size: 18px; color: @text-secondary; margin-top: 8px; lines: 3; }
.val.warn { color: #f39c12; }

.input-row { flex-direction: row; align-items: center; margin-bottom: 12px; }
.input-row .label { font-size: 22px; color: @text-secondary; margin-right: 8px; }
.input { flex: 1; height: 56px; padding: 0 12px; border-radius: 6px; background-color: #1a1a1a; color: #ffffff; font-size: 20px; }

.btn { flex-direction: row; align-items: center; justify-content: center; height: 64px; border-radius: @radius-medium; background-color: @card-background-color; margin-bottom: 12px; }
.btn.primary { background-color: @primary; }
.btn-label { color: #ffffff; font-size: 28px; }
.btn:active { opacity: 0.6; }

.hint { font-size: 20px; color: @text-secondary; text-align: center; margin-top: 8px; lines: 2; }
</style>