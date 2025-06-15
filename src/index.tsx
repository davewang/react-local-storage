import ReactLocalStorage from './NativeReactLocalStorage';

export function multiply(a: number, b: number): number {
  return ReactLocalStorage.multiply(a, b);
}
export function setItem(value: string, key: string): void {
  ReactLocalStorage.setItem(value, key);
}
export function getItem(key: string): string | null {
  return ReactLocalStorage.getItem(key);
}
export function removeItem(key: string): void {
  ReactLocalStorage.removeItem(key);
}
export function clear(): void {
  ReactLocalStorage.clear();
}
