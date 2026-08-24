// tsctl native 模块类型声明（对应 libjsapi_tsctl.so）
declare module 'tsctl' {
  export class TsCtl {
    /** 同步：返回 tailscale CLI 版本（如 "1.98.3"），未安装返回 "not-installed" */
    getVersion(): string
    /** 同步：tailscaled 是否运行（socket 存在） */
    isDaemonRunning(): boolean
    /** 同步：读 /userdisk/tailscale/<name> 配置（如 bridge.conf） */
    readConfigFile(name: string): string
    /** 异步：执行 tailscale CLI 命令，返回 { ok, output } */
    runTailscale(args: string): Promise<{ ok: boolean; output: string }>
    /** 异步：执行 start_tailscale.sh 启动 daemon */
    startDaemon(): Promise<{ ok: boolean; output: string }>
    /** 异步：写 /userdisk/tailscale/<name> 配置 */
    writeConfigFile(name: string, content: string): Promise<boolean>
  }
  export const TsCtl: TsCtl
}
