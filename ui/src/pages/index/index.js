import IndexComponent from "./index.vue";
import { BasePage } from "../../base-page.js";

class PageIndex extends BasePage {
  constructor() {
    super();
  }

  // 首次进入页面（接收 $falcon.navTo 的 options 入参）
  onLoad(options) {
    super.onLoad(options);
    this.setRootComponent(IndexComponent);
  }

  // 已活页面被再次 navTo 时拿新参
  onNewOptions(options) {
    super.onNewOptions(options);
  }

}

export default PageIndex;
