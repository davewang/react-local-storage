import { Text, View, StyleSheet, TextInput, Button,
   // 1. 导入必要的模块
  NativeEventEmitter,
  NativeModules,
 } from 'react-native';
import React,{useEffect} from 'react';
import { ReactLocalStorage } from '@davewang/react-local-storage';

//const result = NativeLocalStorage.multiply(3, 7);
const EMPTY = '<empty>';
// 1. 在这里添加一个日志，它在每次组件渲染时都会被调用
console.log('--- App component is rendering ---');
export default function App() {
    // 2. 在这里也添加一个日志
  console.log('--- App function body is executing ---');
  const [value, setValue] = React.useState<string | null>(null);
  const [deviceId, setDeviceId] = React.useState<string | null>(null);

    // 2. 设置事件监听器
  useEffect(() => {
    console.log('--- useEffect for event listener is running ---');
    // 确保原生模块存在
    if (NativeModules.ReactLocalStorage) {
      const eventEmitter = new NativeEventEmitter(
        NativeModules.ReactLocalStorage
      );

      // 添加监听器，监听名为 'NativeLog' 的事件
      const subscription = eventEmitter.addListener('NativeLog', (event) => {
        // 当事件被触发时，这个回调函数会被执行
        console.log('[C++ Log]:', event.message); // 打印 C++ 发送过来的消息
      });

      // 关键：在组件卸载时，移除监听器以防止内存泄漏
      return () => {
        subscription.remove();
      };
    }
  }, []); // 空依赖数组确保这个 effect 只在组件挂载时运行一次
  React.useEffect(() => {
    const storedValue = ReactLocalStorage?.getItem('myKey');
    setValue(storedValue ?? '');
    ReactLocalStorage?.getDeviceId().then((id)=>{
      console.log('Device ID:', id);
      setDeviceId(id ?? EMPTY);
    });
   
  }, []);
  const [editingValue, setEditingValue] = React.useState<string | null>(null);
  function saveValue() {
    ReactLocalStorage?.setItem(editingValue ?? EMPTY, 'myKey');
    setValue(editingValue);
  }

  function clearAll() {
    ReactLocalStorage?.clear();
    setValue('');
  }
  function startV2Ray() {
    const storedValue = ReactLocalStorage?.getItem('myKey');
    // 1. 添加这个日志来确认函数被调用和值是什么
    console.log('Attempting to call startV2Ray with value:', storedValue ?? '');
    
    ReactLocalStorage?.startV2Ray(storedValue?? '');
  }
  function deleteValue() {
    ReactLocalStorage?.removeItem('myKey');
    setValue('');
  }
  return (
    <View style={styles.container}>
      <Text style={styles.text}>
        Current stored value is: {value ?? 'No Value'}
      </Text>
       <Text style={styles.text}>
        Current deviceId value is: {deviceId ?? 'No Value'}
      </Text>
      {/* <Text>Result: {result}</Text> */}
      <TextInput
        placeholder="Enter the text you want to store"
        style={styles.textInput}
        onChangeText={setEditingValue}
      />
      <Button title="Save" onPress={saveValue} />
      <Button title="Delete" onPress={deleteValue} />
      <Button title="Clear" onPress={clearAll} />
      <Button title="StartV2ray1" onPress={startV2Ray} />
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    alignItems: 'center',
    justifyContent: 'center',
  },
  text: {
    margin: 10,
    fontSize: 20,
  },
  textInput: {
    margin: 10,
    height: 40,
    borderColor: 'black',
    borderWidth: 1,
    paddingLeft: 5,
    paddingRight: 5,
    borderRadius: 5,
  },
});
