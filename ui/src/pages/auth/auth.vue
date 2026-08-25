<template>
  <scroller class="scroller">
    <div class="page">
      <TopBar title="Tailscale 认证" :tip="statusText" :tip-class="tipClass" />

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
    </div>
  </scroller>
</template>

<script>
import TopBar from '../../components/TopBar.vue'
import { TsCtl } from 'tsctl'

export default {
  name: 'PageAuth',
  components: { TopBar },
  props: [],
  data() {
    return {
      authUrl: '',
      loading: false,
      qrFailed: false,
      statusText: '',
      tipClass: '',
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
  },
}
</script>

<style lang="less" scoped>
@import "base.less";

.scroller { width: 750rpx; height: 100%; }
.page { flex-direction: column; padding: 20px; background-color: @background-color; }

.card { flex-direction: column; padding: 16px; border-radius: @radius-medium; background-color: @card-background-color; margin-bottom: 16px; }
.card-title { font-size: 26px; color: @text-color; margin-bottom: 8px; }
.desc { font-size: 20px; color: @text-secondary; margin-bottom: 12px; lines: 2; }

.qrcode-wrap { align-items: center; justify-content: center; padding: 16px; }
.qrcode { width: 200px; height: 200px; }

.val { font-size: 18px; color: @text-secondary; margin-top: 8px; lines: 3; }
.val.warn { color: #f39c12; }

.btn { flex-direction: row; align-items: center; justify-content: center; height: 48px; border-radius: @radius-medium; background-color: @card-background-color; margin-bottom: 8px; }
.btn.primary { background-color: @primary; }
.btn-label { color: #ffffff; font-size: 26px; }
.btn:active { opacity: 0.6; }
</style>