// TsCtl 业务工作类：只做 OS/C++ 侧事情，不引 jsutil
// 职责：封装 tailscale CLI / tailscaled 的进程控制、socket 状态探测、配置读写
#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

class TsCtl
{
public:
    TsCtl();
    ~TsCtl();

    // ---- 同步短操作 ----
    // 返回 tailscale CLI 版本第一行（如 "1.98.3"）
    std::string getVersion() const;

    // 探测 tailscaled 是否运行（socket 是否存在）
    bool isDaemonRunning() const;

    // 读取安装目录（/userdisk/tailscale）内配置文件内容
    // name: "bridge.conf" 等；不存在返回空串
    std::string readConfigFile(const std::string &name) const;

    // 开机自启检测（/etc/init.d/S99tailscale 是否存在）
    bool isAutostartEnabled() const;

    // 设置开机自启
    // enable: true=创建, false=删除
    bool setAutostart(bool enable) const;

    // 安装/更新 Tailscale
    // version: 目标版本（如 "1.98.3"），空则用默认
    // 返回安装日志（多行）；失败时返回 false
    bool installTailscale(const std::string &version, std::string &log) const;

    // 从 pkgs.tailscale.com 获取最新稳定版版本号
    // 成功返回 true 并填充 version（如 "1.98.3"）
    bool getLatestVersion(std::string &version) const;

    // 生成登录认证 URL（用于二维码）
    // 后台运行 tailscale up --json，轮询解析 AuthURL
    // 成功返回 true 并填充 authUrl；已在线时返回 false 且 out 置空
    bool getAuthUrl(std::string &authUrl) const;

    // 停止后台的 tailscale up 进程（认证取消/超时用）
    void stopAuthWait() const;

    // ---- test ----
    // 测试 popen 在异步线程是否正常工作
    std::string testPopen() const;
    // 执行 tailscale CLI 命令，返回 stdout
    // args: 传给 tailscale 的参数（如 "status --json"）
    bool runTailscale(const std::string &args, std::string &output) const;

    // 执行 start_tailscale.sh（启动 daemon + 可选 auth）
    bool startDaemon(std::string &output) const;

    // 写入配置文件（如 bridge.conf）；返回 false 表示失败
    bool writeConfigFile(const std::string &name, const std::string &content) const;

private:
    std::string execCmd(const std::string &cmd, int &exitCode) const;

    mutable std::mutex mutex_;
};
