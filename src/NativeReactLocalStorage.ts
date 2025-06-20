import type { TurboModule } from 'react-native';
import { TurboModuleRegistry } from 'react-native';
type V2Config = {
    id: string
    content: string 
}
export interface Spec extends TurboModule {
  multiply(a: number, b: number): number;
  setItem(value: string, key: string): void;
  getItem(key: string): string | null;
  removeItem(key: string): void;
  clear(): void;
  // 启动
  startV2Ray(config: string ): void;
  // 退出
  quitApp(): void;
  // 停止
  stopV2Ray(): void;
  // 测试realPing
  testAllRealPing(configList: V2Config[],type: string): void;
  // 获取缓存realPing
  getCacheRealPingById(guid: string): number;
  // 获取设备id
  getDeviceId():Promise<string>;
  // 检查vpn权限
  doPrepare():boolean;
  // 检查vpn状态
  getVpnStatus():Promise<string>;
  // 设置unlimited
  setUnlimited(limited: string):void;
  // 视频奖励
  addReward():void
  // 复制
  copyTheInviteCode(code: string): void;
}

export default TurboModuleRegistry.getEnforcing<Spec>('ReactLocalStorage');
