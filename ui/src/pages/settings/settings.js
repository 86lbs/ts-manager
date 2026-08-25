import SettingsComponent from "./settings.vue";
import { BasePage } from "../../base-page.js";

class PageSettings extends BasePage {
  constructor() { super(); }
  onLoad(options) {
    super.onLoad(options);
    this.setRootComponent(SettingsComponent);
  }
}
export default PageSettings;