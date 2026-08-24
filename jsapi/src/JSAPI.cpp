// ============================================================================
//  JSAPI 总注册入口 — tsctl 插件
//  pluginname = "tsctl" => .so 名 libjsapi_tsctl.so、JS: import { TsCtl } from 'tsctl'
// ============================================================================

#include <jsmodules/JSCModuleExtension.h>
#include <jquick_config.h>
#include "TsCtl/JSTsCtl.hpp"

using namespace JQUTIL_NS;

static std::vector<std::string> exportList = {
    "TsCtl",
};

static int module_init(JSContext *ctx, JSModuleDef *m)
{
    auto env = JQModuleEnv::CreateModule(ctx, m, "tsctl");
    env->setModuleExport("TsCtl", createTsCtl(env.get()));
    env->setModuleExportDone(JS_UNDEFINED, exportList);
    return 0;
}

DEF_MODULE_LOAD_FUNC_EXPORT(tsctl, module_init, exportList)

extern "C" JQUICK_EXPORT void custom_init_jsapis()
{
    registerCModuleLoader("tsctl", &tsctl_module_load);
}
