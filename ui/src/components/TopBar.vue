<template>
  <div class="topbar">
    <div class="back-btn" @click="goBack" v-if="showBack">
      <text class="back-icon">‹</text>
    </div>
    <text class="page-title">{{ title }}</text>
    <text class="status-tip" :class="tipClass">{{ tip }}</text>
  </div>
</template>

<script>
export default {
  name: 'TopBar',
  props: {
    title: { type: String, default: '' },
    tip: { type: String, default: '' },
    tipClass: { type: String, default: '' },
    showBack: { type: Boolean, default: true },
  },
  methods: {
    goBack() {
      this.$falcon.navTo('index', {})
    },
  },
}
</script>

<style lang="less" scoped>
@import "base.less";

.topbar {
  flex-direction: row;
  align-items: center;
  padding: 4px 0 10px 0;
}

.back-btn {
  width: 64px;
  height: 64px;
  align-items: center;
  justify-content: center;
  margin-right: 4px;
}

.back-btn:active {
  opacity: 0.5;
}

.back-icon {
  font-size: 56px;
  color: @primary;
  font-weight: bold;
}

.page-title {
  flex: 1;
  font-size: 32px;
  color: @text-color;
  font-weight: bold;
}

.status-tip {
  font-size: 20px;
  color: @text-secondary;
}

.status-tip.ok { color: #2ecc71; }
.status-tip.bad { color: #e74c3c; }
.status-tip.warn { color: #f39c12; }
</style>