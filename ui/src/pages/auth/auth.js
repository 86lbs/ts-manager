import AuthComponent from "./auth.vue";
import { BasePage } from "../../base-page.js";

class PageAuth extends BasePage {
  constructor() { super(); }
  onLoad(options) {
    super.onLoad(options);
    this.setRootComponent(AuthComponent);
  }
}
export default PageAuth;