//
// HAPHashMap.hpp
// Homekit
//
//  Created on: 14.12.2018
//      Author: michael
//
#ifndef _HASHMAP_HPP_
#define _HASHMAP_HPP_

#include <Arduino.h>

template<typename K, typename V, unsigned int capacity>
class HAPHashMap {
  public:
    HAPHashMap();
    ~HAPHashMap();
    unsigned int Size();
    void Put(K key, V value);
    K GetKeyAt(int pos);
    V GetValueAt(int pos);
    V *GetValuePointerAt(int pos);
    V Get(K key);
    V *GetPointer(K key);
    V Get(K key, V defaultValue);
    bool ContainsKey(K key);
    unsigned int GetIndexOfKey(K key);
    void Remove(K key);
    void Dump();
    void DumpKeys();
    void Clear();
    int GetCapacity();
    
  protected:
    K m_keys[capacity];
    V m_values[capacity];
    int m_position;
    int m_capacity;

};

template<typename K, typename V, unsigned int capacity>
int HAPHashMap<K, V, capacity>::GetCapacity() {
  return m_capacity;
}

template<typename K, typename V, unsigned int capacity>
HAPHashMap<K, V, capacity>::HAPHashMap() {
  m_capacity = capacity;
  Clear();
}

template<typename K, typename V, unsigned int capacity>
void HAPHashMap<K, V, capacity>::Clear() {
  m_position = 0;
}


template<typename K, typename V, unsigned int capacity>
HAPHashMap<K, V, capacity>::~HAPHashMap() {
}

template<typename K, typename V, unsigned int capacity>
unsigned int HAPHashMap<K, V, capacity>::Size() {
  return m_position;
}

template<typename K, typename V, unsigned int capacity>
void HAPHashMap<K, V, capacity>::Put(K key, V value) {
  m_keys[m_position] = key;
  m_values[m_position] = value;
  m_position++;
}

template<typename K, typename V, unsigned int capacity>
V HAPHashMap<K, V, capacity>::Get(K key) {
  unsigned int index = GetIndexOfKey(key);
  if (index != -1) {
    return m_values[index];
  }
  return nullptr;
}

template<typename K, typename V, unsigned int capacity>
V *HAPHashMap<K, V, capacity>::GetPointer(K key) {
  unsigned int index = GetIndexOfKey(key);
  if (index != -1) {
    return &m_values[index];
  }
  return nullptr;
}

template<typename K, typename V, unsigned int capacity>
K HAPHashMap<K, V, capacity>::GetKeyAt(int pos) {
  return m_keys[pos];
}

template<typename K, typename V, unsigned int capacity>
V HAPHashMap<K, V, capacity>::GetValueAt(int pos) {
  return m_values[pos];
}

template<typename K, typename V, unsigned int capacity>
V *HAPHashMap<K, V, capacity>::GetValuePointerAt(int pos) {
  return &m_values[pos];
}


template<typename K, typename V, unsigned int capacity>
V HAPHashMap<K, V, capacity>::Get(K key, V defaultValue) {
  unsigned int index = GetIndexOfKey(key);
  if (index != -1) {
    return m_values[index];
  }
  else {
    return defaultValue;
  }
}


template<typename K, typename V, unsigned int capacity>
unsigned int HAPHashMap<K, V, capacity>::GetIndexOfKey(K key) {
  for (int i = 0; i < m_position; i++) {
    if (key == m_keys[i]) {
      return i;
    }
  }
  return -1;
}

template<typename K, typename V, unsigned int capacity>
bool HAPHashMap<K, V, capacity>::ContainsKey(K key) {
  return GetIndexOfKey(key) != -1;
}

template<typename K, typename V, unsigned int capacity>
void HAPHashMap<K, V, capacity>::Remove(K key) {
  int index = GetIndexOfKey(key);
  if (index != -1) {
    for (int i = index; i < capacity - 1; i++) {
      m_keys[i] = m_keys[i + 1];
      m_values[i] = m_values[i + 1];
    }
    m_position--;
  }
}

template<typename K, typename V, unsigned int capacity>
void HAPHashMap<K, V, capacity>::Dump() {
  for (unsigned int i = 0; i < Size(); i++) {
    Serial.print("Key:");
    Serial.print(m_keys[i]);
    Serial.print("  Val:");
    Serial.print(m_values[i]);
    Serial.println();
  }
}

template<typename K, typename V, unsigned int capacity>
void HAPHashMap<K, V, capacity>::DumpKeys() {
  for (unsigned int i = 0; i < Size(); i++) {
    Serial.print("Key:");
    Serial.print(m_keys[i]);
    Serial.println();
  }
}

#endif