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

// ---- install/update ----
bool TsCtl::installTailscale(const std::string &version, std::string &log) const
{
    // 目标版本：默认最新（与脚本一致），校验只含 [0-9.]
    std::string ver = version.empty() ? "1.98.3" : version;
    if (ver.find_first_not_of("0123456789.") != std::string::npos) {
        log = "版本号不合法: " + ver;
        return false;
    }

    // 检测架构
    int rc;
    std::string arch = execCmd("uname -m", rc);
    std::string tsArch;
    if (arch.find("aarch64") != std::string::npos) tsArch = "arm64";
    else if (arch.find("armv7") != std::string::npos || arch.find("armv8") != std::string::npos) tsArch = "arm";
    else if (arch.find("x86_64") != std::string::npos) tsArch = "amd64";
    else { log = "不支持架构: " + arch; return false; }

    // 生成安装脚本（参考 install_tailscale.sh，保留已有 state 避免重新认证）
    std::string script =
        "set -e\n"
        "BASE=/userdisk/tailscale\n"
        "VERSION=" + ver + "\n"
        "TSARCH=" + tsArch + "\n"
        "unset ALL_PROXY all_proxy HTTP_PROXY http_proxy HTTPS_PROXY https_proxy NO_PROXY no_proxy\n"
        "echo '[1/6] 检测架构: $(uname -m) -> '${TSARCH}\n"
        "mkdir -p $BASE\n"
        "echo '[2/6] 下载 tailscale_'${VERSION}'_'${TSARCH}'.tgz...'\n"
        "URL=\"https://pkgs.tailscale.com/stable/tailscale_${VERSION}_${TSARCH}.tgz\"\n"
        "curl -fsSL \"$URL\" -o /tmp/ts_install.tgz || { echo '下载失败: $URL'; exit 1; }\n"
        "echo '[3/6] 校验 sha256...'\n"
        "if command -v sha256sum >/dev/null 2>&1; then\n"
        "  EXPECTED=$(curl -fsSL \"${URL}.sha256\" 2>/dev/null | awk '{print $1}' || echo '')\n"
        "  if [ -n \"$EXPECTED\" ]; then\n"
        "    ACTUAL=$(sha256sum /tmp/ts_install.tgz | awk '{print $1}')\n"
        "    if [ \"$EXPECTED\" != \"$ACTUAL\" ]; then\n"
        "      echo '哈希校验失败!'; exit 1;\n"
        "    fi\n"
        "    echo '哈希校验通过'\n"
        "  fi\n"
        "fi\n"
        "echo '[4/6] 解压...'\n"
        "rm -rf /tmp/ts_pkg\n"
        "mkdir -p /tmp/ts_pkg\n"
        "tar -xzf /tmp/ts_install.tgz -C /tmp/ts_pkg || { echo '解压失败'; exit 1; }\n"
        "SRC=$(find /tmp/ts_pkg -name 'tailscale' -type f | head -1 | xargs dirname)\n"
        "echo '  包目录: '$SRC\n"
        "echo '[5/6] 停止旧 tailscaled...'\n"
        "OLD_PIDS=$(ps w | grep 'tailscaled' | grep -v grep | grep -v 'sh -l' | awk '{print \$1}')\n"
        "if [ -n \"$OLD_PIDS\" ]; then\n"
        "  kill -9 $OLD_PIDS 2>/dev/null || true\n"
        "  sleep 2\n"
        "  echo '  已停止: ' $OLD_PIDS\n"
        "fi\n"
        "echo '[6/6] 部署并启动新版本...'\n"
        "cp \"$SRC/tailscale\" $BASE/tailscale\n"
        "cp \"$SRC/tailscaled\" $BASE/tailscaled\n"
        "chmod 755 $BASE/tailscale $BASE/tailscaled\n"
        "rm -rf /tmp/ts_pkg /tmp/ts_install.tgz\n"
        "nohup $BASE/tailscaled --state=$BASE/tailscaled.state --socket=$BASE/tailscaled.sock --tun=userspace-networking --socks5-server=localhost:1055 >> $BASE/tailscaled.log 2>&1 &\n"
        "for i in 1 2 3 4 5; do\n"
        "  [ -S $BASE/tailscaled.sock ] && break\n"
        "  sleep 1\n"
        "done\n"
        "echo '  新 tailscaled 已启动'\n"
        "echo '[7/7] 配置开机自启...'\n"
        "if [ -x \"$BASE/start_tailscale.sh\" ]; then :; else\n"
        "  cat > $BASE/start_tailscale.sh << 'SCRIPT'\n"
        "#!/bin/sh\n"
        "BASE=/userdisk/tailscale\n"
        "SOCKET=$BASE/tailscaled.sock\n"
        "STATE=$BASE/tailscaled.state\n"
        "LOG=$BASE/tailscaled.log\n"
        "if ! pgrep -x tailscaled >/dev/null 2>&1; then\n"
        "  unset ALL_PROXY all_proxy HTTP_PROXY http_proxy HTTPS_PROXY https_proxy NO_PROXY no_proxy\n"
        "  nohup $BASE/tailscaled --state=$STATE --socket=$SOCKET --tun=userspace-networking --socks5-server=localhost:1055 >> $LOG 2>&1 &\n"
        "  for i in 1 2 3; do\n"
        "    [ -S \"$SOCKET\" ] && break\n"
        "    sleep 2\n"
        "  done\n"
        "fi\n"
        "SCRIPT\n"
        "  chmod 755 $BASE/start_tailscale.sh\n"
        "fi\n"
        "echo '[7/7] 安装完成: tailscale '${VERSION}\n"
        "$BASE/tailscale --socket=$BASE/tailscaled.sock version 2>/dev/null | head -1 || true\n";

    // 写安装脚本到临时文件再执行（避免 shell 引号冲突）
    const char *scriptPath = "/tmp/ts_install.sh";
    {
        std::ofstream out(scriptPath, std::ios::trunc);
        if (!out) { log = "无法写安装脚本"; return false; }
        out << script;
        out.flush();
        if (!out.good()) { log = "写安装脚本失败"; return false; }
        ::chmod(scriptPath, 0755);
    }
    log = execCmd(std::string("sh ") + scriptPath + " 2>&1", rc);
    return true;
}

// ---- latest version ----
bool TsCtl::getLatestVersion(std::string &version) const
{
    int rc;
    std::string out = execCmd(
        "curl -fsSL --max-time 15 'https://pkgs.tailscale.com/stable/?mode=json' 2>/dev/null",
        rc);
    if (out.empty()) return false;

    // 解析 JSON 里的 "Version":"x.y.z"（简单字符串查找，不引 JSON 库）
    size_t vpos = out.find("\"Version\"");
    if (vpos == std::string::npos) return false;
    size_t colon = out.find(':', vpos);
    if (colon == std::string::npos) return false;
    size_t q1 = out.find('"', colon);
    if (q1 == std::string::npos) return false;
    size_t q2 = out.find('"', q1 + 1);
    if (q2 == std::string::npos) return false;

    version = out.substr(q1 + 1, q2 - q1 - 1);
    // 校验版本号格式
    if (version.empty() ||
        version.find_first_not_of("0123456789.-") != std::string::npos) {
        version.clear();
        return false;
    }
    return true;
}
