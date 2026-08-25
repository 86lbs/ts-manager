// JS 包壳类：JS 端可见的 API、协议、线程模型都在这里组织
#pragma once
#include "TsCtl.hpp"
#include <jqutil_v2/jqutil.h>
#include <memory>
#include <mutex>

using namespace JQUTIL_NS;

class JSTsCtl : public JQPublishObject
{
public:
    JSTsCtl();
    ~JSTsCtl();

    // 同步短操作
    void getVersion(JQFunctionInfo &info);
    void isDaemonRunning(JQFunctionInfo &info);
    void readConfigFile(JQFunctionInfo &info);
    void testPopen(JQFunctionInfo &info);
    void isAutostartEnabled(JQFunctionInfo &info);
    void setAutostart(JQFunctionInfo &info);

    // 异步阻塞操作
    void runTailscale(JQAsyncInfo &info);
    void startDaemon(JQAsyncInfo &info);
    void writeConfigFile(JQAsyncInfo &info);

private:
    std::unique_ptr<TsCtl> obj_;
    mutable std::mutex objMutex_;
    TsCtl *getObj() const {
        std::lock_guard<std::mutex> lock(objMutex_);
        return obj_.get();
    }
};

extern JSValue createTsCtl(JQModuleEnv *env);
