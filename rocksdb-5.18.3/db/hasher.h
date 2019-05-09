#ifndef HASHER_H
#define HASHER_H

namespace rocksdb {

class Hasher {
 public:
  Hasher(const int& context_size);
  ~Hasher();
  unsigned long long Hash(const unsigned char& c);
  unsigned long long HashLong(const unsigned long long& c);
  char* GetString();
  void Clear();
 private:
  unsigned long long pow_;
  //unsigned char* context_;
  unsigned long long* context_;
  unsigned int context_pos_;
  unsigned int context_size_;
  unsigned long long hash_;
};

}

#endif
