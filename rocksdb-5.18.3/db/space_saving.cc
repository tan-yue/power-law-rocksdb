#include <cstddef>
#include <iostream>
#include "rocksdb/space_saving.h"

#include "util/logging.h"
#include "rpc/client.h"
#include "rpc/server.h"

namespace rocksdb{

Parent::Parent() : left_(NULL), right_(NULL), child_(NULL), value_(0) {}

void Parent::Add(Child* c) {
  c->parent_ = this;
  if (child_ == NULL) {
    child_ = c;
    c->next_ = c;
    return;
  }
  c->next_ = child_->next_;
  child_->next_ = c;
  child_ = c;
}

Child::Child() : parent_(NULL), next_(NULL), element_(0) {}

void Child::Detach(Parent** smallest, HashMapSS* map) {
  if (next_ == this) {
    if (parent_ == *smallest) {
      *smallest = parent_->right_;
      parent_->right_->left_ = NULL;
      delete parent_;
      return;
    }
    parent_->right_->left_ = parent_->left_;
    parent_->left_->right_ = parent_->right_;
    delete parent_;
    return;
  }
  if (parent_->child_ == next_) {
    parent_->child_ = this;
  }
  unsigned long long temp = element_;
  element_ = next_->element_;
  next_->element_ = temp;
  if (element_) {
    map->Find(element_, &temp);
    map->values_[temp] = this;
  }
  if (next_->element_) {
    map->Find(next_->element_, &temp);
    map->values_[temp] = next_;
  }
  next_ = next_->next_;
}

SpaceSaving::SpaceSaving(const unsigned long long& num_counters) {
  map_ = new HashMapSS(num_counters, 0.75);
  smallest_ = new Parent();
  largest_ = smallest_;
  for (unsigned long long i = 0; i < num_counters; ++i) {
    smallest_->Add(new Child());
  }
}

SpaceSaving::SpaceSaving(HashMapSS* map) {
  map_ = map;
  smallest_ = new Parent();
  largest_ = smallest_;
  for (unsigned long long i = 0; i < map->size_; ++i) {
    if (map->keys_[i] != 0) {
      smallest_->Add(map->values_[i]);
    }
  }
}

SpaceSaving::~SpaceSaving() {
  for (unsigned long long i = 0; i < map_->size_; ++i) {
    if (map_->keys_[i] != 0) {
      delete map_->values_[i];
    }
  }
  while (smallest_ != NULL) {
    Parent* tmp = smallest_->right_;
    delete smallest_;
    smallest_ = tmp;
  }
  delete map_;
}

void SpaceSaving::Increment(Child* bucket) {
  unsigned long long count = bucket->parent_->value_ + 1;
  Parent* next = bucket->parent_->right_;
  if (next != NULL && next->value_ == count) {
    Child* temp = bucket->next_;
    bucket->Detach(&smallest_, map_);
    next->Add(temp);
  } else if (bucket->next_ == bucket) {
    bucket->parent_->value_ = count;
  } else {
    Child* temp = bucket->next_;
    bucket->Detach(&smallest_, map_);
    bucket = temp;
    Parent* p = new Parent();
    p->left_ = bucket->parent_;
    p->value_ = count;
    bucket->parent_->right_ = p;
    if (next != NULL) {
      p->right_ = next;
      next->left_ = p;
    } else {
      largest_ = p;
    }
    p->Add(bucket);
  }
}

void SpaceSaving::Process(const unsigned long long& element) {
  unsigned long long index;
  if (!map_->Find(element, &index)) {
    Child* bucket = smallest_->child_;
    map_->Remove(bucket->element_);
    bucket->element_ = element;
    map_->Insert(element, bucket);
    Increment(bucket);
  } else {
    Increment(map_->values_[index]);
  }
}

void SpaceSaving::ExtractTop(std::shared_ptr<Logger> info_log, const unsigned long long output_counters, HashMapSS* map) {
  unsigned long long count = 0;
  while(true) {
    Child* c = largest_->child_;
    do {
      Child* c2 = new Child();
      c2->element_ = c->element_;
	  ROCKS_LOG_INFO(info_log, "ASH element=%d, frequency=%d", c->element_, largest_->value_);
      //std::cout<< "element=" << c->element_<< "frequency=" <<largest_->value_ << std::endl;
      map->Insert(c->element_, c2);
      ++count;
      if (count == output_counters) break;
      c = c->next_;
    } while (c != largest_->child_);
    if (count == output_counters) break;
    largest_ = largest_->left_;
    if (largest_->value_ == 0) break;
  }
}

std::vector<std::pair<unsigned long long, unsigned long long>> SpaceSaving::ExtractTopVector(const unsigned long long output_counters) {
  unsigned long long count = 0;
  std::vector<std::pair<unsigned long long, unsigned long long>> topk_vector;
  while(true) {
    Child* c = largest_->child_;
    do {
      Child* c2 = new Child();
      c2->element_ = c->element_;
      topk_vector.push_back({c->element_, largest_->value_});
      ++count;
      if (count == output_counters) break;
      c = c->next_;
    } while (c != largest_->child_);
    if (count == output_counters) break;
    largest_ = largest_->left_;
    if (largest_->value_ == 0) break;
  }
  return topk_vector;
}

void SpaceSaving::Print(char** ngrams) {
  unsigned long long index=0;
  while (largest_ != NULL) {
    Child* c = largest_->child_;
    do {
      map_->Find(c->element_, &index);
      c = c->next_;
      std::cout << ngrams[index] << "\t" << largest_->value_ << std::endl;
    } while (c != largest_->child_);
    largest_ = largest_->left_;
  }
}

// custom space saving implementation
void SpaceSaving::SetMin(){
  for(int i=0; i<num_items_; i++){
    if (freq_array_[i].second < freq_array_[min_key_index_].second){
      min_key_index_ = i;
      //std::cout<<"New min key index = "<<min_key_index_<<" pair = "<<freq_array_[min_key_index_].first<<freq_array_[min_key_index_].second<<std::endl;
      return;
    }
  }
}

void SpaceSaving::PrintFreqArray(std::shared_ptr<Logger> info_log){
  for (int i=0; i<num_items_; i++){
    //std::cout<<freq_array_[i].first<<" "<<freq_array_[i].second<<" ";
    ROCKS_LOG_INFO(info_log, "ASH element=%d, frequency=%d", freq_array_[i].first, freq_array_[i].second);
  }
  //std::cout << std::endl;
}

void SpaceSaving::ProcessKey(unsigned long long key){

  // check if key is in the array
  for (int i=0; i<num_items_; i++){
    if (freq_array_[i].first == key){
      freq_array_[i].second++;
      // if this key was min, then find the new min
      if (key == freq_array_[min_key_index_].first){
        SetMin();
      }
      return;
    }
  }

  // if array hasn't filled up yet
  if (num_items_ < m_){
    freq_array_[num_items_] = std::make_pair(key,1);
    // set this item as new min
    min_key_index_ = num_items_; 
    num_items_++;
  }
  else{
    // array is full, evict the least frequent key and assign its freq to new key
    unsigned long long new_freq = freq_array_[min_key_index_].second + 1;
    freq_array_[min_key_index_] = std::make_pair(key, new_freq);
    SetMin();
  }
}

// Compares two intervals according to staring times. 
bool ComparePair(std::pair<unsigned long long, unsigned long long> p1, std::pair<unsigned long long, unsigned long long> p2) 
{
  return (p1.second > p2.second); 
}

void SpaceSaving::ResetFreqArray(){
  num_items_ = 0;
}

std::vector<std::pair<unsigned long long, unsigned long long>> SpaceSaving::ExtractTopK(const unsigned long long k) {
  std::vector<std::pair<unsigned long long, unsigned long long>> topk_vector;
  freq_array_[min_key_index_].second = 0;
  std::sort(freq_array_, freq_array_+m_, ComparePair);
  for(unsigned long long i=0; i<k; i++){
    topk_vector.push_back({freq_array_[i].first, freq_array_[i].second});
  }
  ResetFreqArray();
  return topk_vector;
}

}
