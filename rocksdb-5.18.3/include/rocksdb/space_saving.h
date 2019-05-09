#ifndef SPACE_SAVING_H
#define SPACE_SAVING_H

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
};

}

#endif
