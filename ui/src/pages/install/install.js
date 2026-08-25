import InstallComponent from "./install.vue";
import { BasePage } from "../../base-page.js";

class PageInstall extends BasePage {
  constructor() { super(); }
  onLoad(options) {
    super.onLoad(options);
    this.setRootComponent(InstallComponent);
  }
}
export default PageInstall;