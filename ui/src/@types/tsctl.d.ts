// tsctl native 模块类型声明（对应 libjsapi_tsctl.so）
declare module 'tsctl' {
  export class TsCtl {
    /** 同步：返回 tailscale CLI 版本（如 "1.98.3"），未安装返回 "not-installed" */
    getVersion(): string
    /** 同步：tailscaled 是否运行（socket 存在） */
    isDaemonRunning(): boolean
    /** 同步：读 /userdisk/tailscale/<name> 配置（如 bridge.conf） */
    readConfigFile(name: string): string
    /** 同步：测试 popen 是否正常 */
    testPopen(): string
    /** 同步：开机自启是否已启用 */
    isAutostartEnabled(): boolean
    /** 同步：设置开机自启 */
    setAutostart(enable: boolean): boolean
    /** 同步：写 /userdisk/tailscale/<name> 配置 */
    writeConfigFile(name: string, content: string): Promise<boolean>
    /** 异步：执行 tailscale CLI 命令，返回 stdout 字符串 */
    runTailscale(args: string): Promise<string>
    /** 异步：执行 start_tailscale.sh 启动 daemon */
    startDaemon(): Promise<string>
  }
  export const TsCtl: TsCtl
}