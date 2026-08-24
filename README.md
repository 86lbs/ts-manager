# ts-manager — 有道词典笔 Tailscale 管理软件

在词典笔（X3S, aarch64/glibc 2.29）上管理 Tailscale 的 Falcon mini-app：
安装、启动/停止、认证（二维码 + auth key）、设置（bridge/自启/版本）、诊断。

## 架构

```
Falcon UI (Vue 3 SFC, 480×800)
  └── import { TsCtl } from 'tsctl'      ← native JSAPI 模块
        └── libjsapi_tsctl.so (C++17, aarch64-buildroot-linux-gnu-)
              └── popen/fork+setsid → tailscale CLI / tailscaled
                    └── /userdisk/tailscale/ (daemon + state)
```

## 设备探测结论（profiles/x3s.yml）

- miniapp 宿主进程 **UID=0 (root)** → native 可写 /etc/init.d、可 exec、可安装
- glibc 2.29, GLIBCXX 3.4.28；工具链 glibc 2.27/GLIBCXX 3.4.24 → 产物符号版本兼容
- Tailscale 1.98.3 已在运行（100.79.182.85）

## 构建

推荐用 GitHub Actions（本地交叉编译 iot_sdk 易 OOM）：

```bash
# 推送后手动触发，或 push 到 main 自动构建
gh workflow run build_x3s.yml
# 产物: Actions → ts-manager-x3s-amr (ui/*.amr)
```

本地（有交叉工具链时）：
```bash
export CROSS_TOOLCHAIN_PREFIX=/path/to/aarch64-buildroot-linux-gnu-
cmake -S jsapi -B jsapi/build -DCMAKE_BUILD_TYPE=Release
cmake --build jsapi/build --parallel 2
cp jsapi/build/libjsapi_tsctl.so ui/libs/
```

## 里程碑

- [ ] M0 骨架：状态页 UI + native 编译链路（进行中）
- [ ] M1 核心：status/up/down/auth-key/二维码认证/设置
- [ ] M2 安装：一键安装/修复/升级（native 安装器）
- [ ] M3 打磨：诊断页、PStore checkUpdate、真机多轮验证
