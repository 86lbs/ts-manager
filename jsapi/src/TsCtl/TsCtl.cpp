#include <sys/wait.h>

#include "TsCtl.hpp"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace {

const char *kBaseDir = "/userdisk/tailscale";
const char *kTailscaleCli = "/userdisk/tailscale/tailscale";
const char *kSocket = "/userdisk/tailscale/tailscaled.sock";
const char *kStartScript = "/userdisk/tailscale/start_tailscale.sh";

bool fileExists(const std::string &path)
{
    struct stat st;
    return ::stat(path.c_str(), &st) == 0;
}

} // namespace

TsCtl::TsCtl() = default;
TsCtl::~TsCtl() = default;

// 执行任意 shell 命令，捕获 stdout；返回 stdout，exitCode 填退出码
std::string TsCtl::execCmd(const std::string &cmd, int &exitCode) const
{
    FILE *fp = ::popen(cmd.c_str(), "r");
    if (!fp) {
        exitCode = -1;
        return "";
    }
    char buf[4096];
    std::stringstream ss;
    while (::fgets(buf, sizeof(buf), fp) != nullptr)
        ss << buf;
    exitCode = ::pclose(fp);
    return ss.str();
}

std::string TsCtl::getVersion() const
{
    if (!fileExists(kTailscaleCli)) return "not-installed";
    int rc;
    std::string out = execCmd(std::string(kTailscaleCli) + " --socket=" + kSocket + " version 2>/dev/null | head -1", rc);
    // 去掉尾部空白
    while (!out.empty() && (out.back() == '\n' || out.back() == '\r')) out.pop_back();
    return out;
}

bool TsCtl::isDaemonRunning() const
{
    return fileExists(kSocket);
}

std::string TsCtl::readConfigFile(const std::string &name) const
{
    // 防路径穿越
    if (name.empty() || name.find('/') != std::string::npos || name.find("..") != std::string::npos)
        return "";
    std::ifstream in(std::string(kBaseDir) + "/" + name);
    if (!in) return "";
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

bool TsCtl::writeConfigFile(const std::string &name, const std::string &content) const
{
    if (name.empty() || name.find('/') != std::string::npos || name.find("..") != std::string::npos)
        return false;
    std::ofstream out(std::string(kBaseDir) + "/" + name, std::ios::trunc);
    if (!out) return false;
    out << content;
    out.flush();
    return out.good();
}

bool TsCtl::runTailscale(const std::string &args, std::string &output) const
{
    if (!fileExists(kTailscaleCli)) {
        output = "tailscale not installed";
        return false;
    }
    int rc;
    output = execCmd(std::string(kTailscaleCli) + " --socket=" + kSocket + " " + args + " 2>&1", rc);
    return true;
}

bool TsCtl::startDaemon(std::string &output) const
{
    if (!fileExists(kStartScript)) {
        output = "start_tailscale.sh missing";
        return false;
    }
    int rc;
    output = execCmd(std::string("sh ") + kStartScript + " 2>&1", rc);
    return true;
}

// ---- test ----
std::string TsCtl::testPopen() const
{
    int rc;
    std::string out = execCmd("echo 'popen_ok' 2>&1", rc);
    return out + "|rc=" + std::to_string(rc);
}

// ---- autostart ----
bool TsCtl::isAutostartEnabled() const
{
    struct stat st;
    if (::stat("/etc/init.d/S99tailscale", &st) != 0) return false;
    return (st.st_mode & S_IFMT) == S_IFREG;
}

bool TsCtl::setAutostart(bool enable) const
{
    const char *path = "/etc/init.d/S99tailscale";
    if (enable) {
        if (!fileExists(kStartScript)) return false;
        std::string content =
            "#!/bin/sh\n"
            "# Tailscale 开机自启 - 由 ts-manager 管理\n"
            "#\n"
            "BASE=/userdisk/tailscale\n"
            "SOCKET=$BASE/tailscaled.sock\n"
            "STATE=$BASE/tailscaled.state\n"
            "LOG=$BASE/tailscaled.log\n"
            "\n"
            "if ! pgrep -x tailscaled >/dev/null 2>&1; then\n"
            "  unset ALL_PROXY all_proxy HTTP_PROXY http_proxy HTTPS_PROXY https_proxy NO_PROXY no_proxy\n"
            "  nohup $BASE/tailscaled --state=$STATE --socket=$SOCKET --tun=userspace-networking --socks5-server=localhost:1055 >> $LOG 2>&1 &\n"
            "  for i in 1 2 3; do\n"
            "    [ -S \"$SOCKET\" ] && break\n"
            "    sleep 2\n"
            "  done\n"
            "fi\n";
        std::ofstream out(path, std::ios::trunc);
        if (!out) return false;
        out << content;
        out.flush();
        if (!out.good()) return false;
        return ::chmod(path, 0755) == 0;
    } else {
        return ::unlink(path) == 0;
    }
}
