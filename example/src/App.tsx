import { Text, View, StyleSheet, TextInput, Button } from 'react-native';
import React from 'react';
import { getItem, setItem, clear, removeItem } from 'react-local-storage';

//const result = NativeLocalStorage.multiply(3, 7);
const EMPTY = '<empty>';
export default function App() {
  const [value, setValue] = React.useState<string | null>(null);
  React.useEffect(() => {
    const storedValue = getItem('myKey');
    setValue(storedValue ?? '');
  }, []);
  const [editingValue, setEditingValue] = React.useState<string | null>(null);
  function saveValue() {
    setItem(editingValue ?? EMPTY, 'myKey');
    setValue(editingValue);
  }

  function clearAll() {
    clear();
    setValue('');
  }

  function deleteValue() {
    removeItem('myKey');
    setValue('');
  }
  return (
    <View style={styles.container}>
      <Text style={styles.text}>
        Current stored value is: {value ?? 'No Value'}
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
