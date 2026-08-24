import { BasePage } from './base-page.js';

// X3S 实测：DRM card0-DSI-1 modes = 480x800
// 采用标准 Falcon 逻辑宽度 750，rpx 单位自动缩放
const DESIGN_WIDTH = 750;

class App extends $falcon.App {
  constructor() {
    super();
  }

  onLaunch(options) {
    super.onLaunch(options);
    this.setViewPort(DESIGN_WIDTH);
    $falcon.useDefaultBasePageClass(BasePage);
  }

  onShow() {
    super.onShow();
  }

  onHide() {
    super.onHide();
  }

  onDestroy() {
    super.onDestroy();
  }
}

export default App;
