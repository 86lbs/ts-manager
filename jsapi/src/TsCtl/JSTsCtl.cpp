#include "JSTsCtl.hpp"
#include <Exceptions/Exception.hpp>
#include <Exceptions/AssertFailed.hpp>

// ----------------------------------------------------------------------------
// Factory 骨架
// ----------------------------------------------------------------------------
extern JSValue createTsCtl(JQModuleEnv *env)
{
    JQFunctionTemplateRef tpl = JQFunctionTemplate::New(env, "TsCtl");
    tpl->InstanceTemplate()->setObjectCreator([]() { return new JSTsCtl(); });

    tpl->SetProtoMethod("getVersion", &JSTsCtl::getVersion);
    tpl->SetProtoMethod("isDaemonRunning", &JSTsCtl::isDaemonRunning);
    tpl->SetProtoMethod("readConfigFile", &JSTsCtl::readConfigFile);
    tpl->SetProtoMethod("testPopen", &JSTsCtl::testPopen);
    tpl->SetProtoMethod("isAutostartEnabled", &JSTsCtl::isAutostartEnabled);
    tpl->SetProtoMethod("setAutostart", &JSTsCtl::setAutostart);

    tpl->SetProtoMethodPromise("runTailscale", &JSTsCtl::runTailscale);
    tpl->SetProtoMethodPromise("startDaemon", &JSTsCtl::startDaemon);
    tpl->SetProtoMethodPromise("writeConfigFile", &JSTsCtl::writeConfigFile);

    return tpl->CallConstructor();
}

JSTsCtl::JSTsCtl()
    : obj_(std::make_unique<TsCtl>()) {}

JSTsCtl::~JSTsCtl() = default;

// ----------------------------------------------------------------------------
// 同步方法：JS 线程跑；别阻塞！
// ----------------------------------------------------------------------------
void JSTsCtl::getVersion(JQFunctionInfo &info)
{
    try {
        ASSERT(info.Length() == 0);
        TsCtl *o = getObj(); ASSERT(o != nullptr);
        info.GetReturnValue().Set(o->getVersion());
    } catch (const std::exception &e) {
        info.GetReturnValue().ThrowInternalError(e.what());
    }
}

void JSTsCtl::isDaemonRunning(JQFunctionInfo &info)
{
    try {
        ASSERT(info.Length() == 0);
        TsCtl *o = getObj(); ASSERT(o != nullptr);
        info.GetReturnValue().Set(o->isDaemonRunning());
    } catch (const std::exception &e) {
        info.GetReturnValue().ThrowInternalError(e.what());
    }
}

void JSTsCtl::readConfigFile(JQFunctionInfo &info)
{
    try {
        ASSERT(info.Length() == 1);
        JSContext *ctx = info.GetContext();
        const char *p = JS_ToCString(ctx, info[0]);
        if (!p) { info.GetReturnValue().ThrowInternalError("bad arg"); return; }
        std::string name(p);
        JS_FreeCString(ctx, p);
        TsCtl *o = getObj(); ASSERT(o != nullptr);
        info.GetReturnValue().Set(o->readConfigFile(name));
    } catch (const std::exception &e) {
        info.GetReturnValue().ThrowInternalError(e.what());
    }
}

void JSTsCtl::testPopen(JQFunctionInfo &info)
{
    try {
        ASSERT(info.Length() == 0);
        TsCtl *o = getObj(); ASSERT(o != nullptr);
        info.GetReturnValue().Set(o->testPopen());
    } catch (const std::exception &e) {
        info.GetReturnValue().ThrowInternalError(e.what());
    }
}

void JSTsCtl::isAutostartEnabled(JQFunctionInfo &info)
{
    try {
        ASSERT(info.Length() == 0);
        TsCtl *o = getObj(); ASSERT(o != nullptr);
        info.GetReturnValue().Set(o->isAutostartEnabled());
    } catch (const std::exception &e) {
        info.GetReturnValue().ThrowInternalError(e.what());
    }
}

void JSTsCtl::setAutostart(JQFunctionInfo &info)
{
    try {
        ASSERT(info.Length() == 1);
        JSContext *ctx = info.GetContext();
        bool enable = JS_ToBool(ctx, info[0]);
        TsCtl *o = getObj(); ASSERT(o != nullptr);
        info.GetReturnValue().Set(o->setAutostart(enable));
    } catch (const std::exception &e) {
        info.GetReturnValue().ThrowInternalError(e.what());
    }
}

// ----------------------------------------------------------------------------
// 异步 Promise：模块线程池里跑，可阻塞
// ----------------------------------------------------------------------------
void JSTsCtl::runTailscale(JQAsyncInfo &info)
{
    try {
        ASSERT(info.Length() == 1);
        ASSERT(info[0].is_string());
        std::string args = info[0].string_value();

        TsCtl *o = getObj(); ASSERT(o != nullptr);
        std::string output;
        if (!o->runTailscale(args, output))
            info.postError(output.empty() ? "tailscale failed" : output);
        else
            info.post(output);  // 直接 post 字符串，像 PenTool 一样
    } catch (const std::exception &e) {
        info.postError(e.what());
    }
}

void JSTsCtl::startDaemon(JQAsyncInfo &info)
{
    try {
        ASSERT(info.Length() == 0);
        TsCtl *o = getObj(); ASSERT(o != nullptr);
        std::string output;
        if (!o->startDaemon(output))
            info.postError(output.empty() ? "startDaemon failed" : output);
        else
            info.post(Bson::object{{"ok", true}, {"output", output}});
    } catch (const std::exception &e) {
        info.postError(e.what());
    }
}

void JSTsCtl::writeConfigFile(JQAsyncInfo &info)
{
    try {
        ASSERT(info.Length() == 2);
        ASSERT(info[0].is_string());
        ASSERT(info[1].is_string());
        std::string name = info[0].string_value();
        std::string content = info[1].string_value();

        TsCtl *o = getObj(); ASSERT(o != nullptr);
        bool ok = o->writeConfigFile(name, content);
        if (!ok)
            info.postError("write failed");
        else
            info.post(true);
    } catch (const std::exception &e) {
        info.postError(e.what());
    }
}
