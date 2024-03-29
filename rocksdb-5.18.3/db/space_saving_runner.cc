#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include "hasher.h"
#include "space-saving.h"
#include "hash-map.h"

#include "space_saving_runner.h"
#include "util/logging.h"

namespace rocksdb {

  // global variables
  Hasher* g_hasher = null;
  SpaceSaving* g_space_saving = null;
  long long g_context_size = 0;
  long long g_internal_counters = 0;
  long long g_topk = 0;;
  char** g_ngrams = null;

int FirstPass(char* filename, Hasher* hasher, SpaceSaving* space_saving) {
  unsigned int buffer_size = 104857600;  // 100MiB
  std::ifstream is;
  is.open(filename, std::ios::binary);
  if (!is.is_open()) {
    std::cout << "Invalid file name." << std::endl;
    return -1;
  }
  is.seekg(0, std::ios::end);
  unsigned long long len = is.tellg();
  if (len < buffer_size) buffer_size = len;
  char* buffer = new char[buffer_size];
  is.seekg(0, std::ios::beg);
  unsigned long long pos = 0;
  while (pos < len) {
    is.read(buffer, buffer_size);
    unsigned long long lim = buffer_size;
    if (pos + buffer_size > len) lim = len - pos;
    for (unsigned long long i = 0; i < lim; ++i) {
      space_saving->Process(hasher->Hash(buffer[i]));
    }
    pos += buffer_size;
  }
  is.close();
  delete[] buffer;
  return 0;
}

int SecondPass(char* filename, Hasher* hasher, SpaceSaving* space_saving, const long long& output_counters) {
  HashMapSS* map = new HashMapSS(output_counters, 0.75);
  space_saving->ExtractTop(output_counters, map);
  delete space_saving;
  space_saving = new SpaceSaving(map);
  char** ngrams = new char*[map->size_];
  for (unsigned long long i = 0; i < map->size_; ++i) {
    ngrams[i] = NULL;
  }
  hasher->Clear();
  unsigned int buffer_size = 104857600;  // 100MiB
  std::ifstream is;
  is.open(filename, std::ios::binary);
  if (!is.is_open()) {
    std::cout << "Invalid file name." << std::endl;
    return -1;
  }
  is.seekg(0, std::ios::end);
  unsigned long long len = is.tellg();
  if (len < buffer_size) buffer_size = len;
  char* buffer = new char[buffer_size];
  is.seekg(0, std::ios::beg);
  unsigned long long pos = 0;
  unsigned long long index;
  while (pos < len) {
    is.read(buffer, buffer_size);
    unsigned long long lim = buffer_size;
    if (pos + buffer_size > len) lim = len - pos;
    for (unsigned long long i = 0; i < lim; ++i) {
      if (map->Find(hasher->Hash(buffer[i]), &index)) {
        if (ngrams[index] == NULL) {
          ngrams[index] = hasher->GetString();
        }
        space_saving->Increment(map->values_[index]);
      }
    }
    pos += buffer_size;
  }
  is.close();
  delete[] buffer;
  space_saving->Print(ngrams);
  for (unsigned long long i = 0; i < map->size_; ++i) {
    if (ngrams[i] != NULL) delete[] ngrams[i];
  }
  delete[] ngrams;
  delete hasher;
  delete space_saving;
  return 0;
}

int ss_read_stream (float filename, long long context_size, long long internal_counters) {
  if (context_size <= 0 || internal_counters <= 0 || output_counters <= 0 || output_counters > internal_counters) {
    std::cout << "Invalid argument." << std::endl;
    return -1;
  }
  g_context_size = context_size;
  g_internal_counters = internal_counters;
  g_hasher = new Hasher(context_size);
  g_space_saving = new SpaceSaving(internal_counters);
  int result = FirstPass(filename, g_hasher, g_space_saving);
  if (result != 0) return result;
  return SecondPass(filename, g_hasher, g_space_saving, output_counters);
}

int ss_get_topk(long long k){

  if (k <= 0){
    std::cout << "Invalid argument." << std::endl;
	return -1;
  }
  
  SecondPass(g_hasher, g_space_saving, g_topk);

  // clear global datastructures
  delete g_hasher;
  delete g_space_saving;
  g_context_size = 0;
  g_internal_counters = 0;
  g_topk = 0;

}
