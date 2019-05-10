#ifndef SPACE_SAVING_H
#define SPACE_SAVING_H

#include <functional>
#include <queue>
#include <vector>
#include <iostream>
#include <utility>

#include "rocksdb/hash_map.h"

namespace rocksdb {

class Child;

class Parent {
 public:
  Parent();
  void Add(Child* c);
  Parent* left_;
  Parent* right_;
  Child* child_;
  unsigned long long value_;
};

class Child {
 public:
  Child();
  void Detach(Parent** smallest, HashMapSS* map);
  Parent* parent_;
  Child* next_;
  unsigned long long element_;
};

class SpaceSaving {
 public:
  SpaceSaving(const unsigned long long& num_counters);
  SpaceSaving(HashMapSS* map);
  ~SpaceSaving();
  void Process(const unsigned long long& element);
  void Increment(Child* bucket);
  void ExtractTop(std::shared_ptr<Logger> info_log, const unsigned long long output_counters, HashMapSS* map);
  std::vector<std::pair<unsigned long long, unsigned long long> > ExtractTopVector(const unsigned long long output_counters);
  void Print(char** ngrams);
  HashMapSS* map_;
  Parent* smallest_;
  Parent* largest_;
  // custom implementation of space saving
  void SetMin();
  void ProcessKey(unsigned long long key);
  std::vector<std::pair<unsigned long long, unsigned long long> > ExtractTopK(const unsigned long long k);
  void PrintFreqArray(std::shared_ptr<Logger> info_log);
  void ResetFreqArray();
  int num_items_ = 0;
  static const int m_ = 31; // monitored keys
  int min_key_index_ = -1;
  std::pair<unsigned long long, unsigned long long> freq_array_[m_];
};

}

#endif
