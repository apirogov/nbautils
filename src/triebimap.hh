#pragma once

#include <map>
#include <memory>
#include <vector>

namespace nbautils {

template <typename K, typename V> struct trie_bimap_node {
  typedef std::unique_ptr<trie_bimap_node<K, V>> node_ptr;
  trie_bimap_node *parent = nullptr;
  K key = 0;
  std::unique_ptr<V> value = nullptr;

  std::map<K, trie_bimap_node<K, V>> suc;
};

// TODO: add access to leaves? other operations?
// TODO: abstract bimap interface with lookups, insertion and removal
template <typename K, typename V> class trie_bimap {
  trie_bimap_node<K, V> root;
  std::map<V, trie_bimap_node<K, V> *> revmap;

  // traverse trie to given node and return it. when create=false and it does
  // not exist, return null pointer
  trie_bimap_node<K, V>* traverse(std::vector<K> const &ks, bool create = false) {
    auto *curr = &root;
    for (int i = ks.size() - 1; i >= 0; i--) {
      if (curr->suc.find(ks[i]) == curr->suc.end()) {
        if (create) {
          curr->suc[ks[i]].parent = curr;
          curr->suc[ks[i]].key = ks[i];
        } else
          return nullptr;
      }
      curr = &curr->suc[ks[i]];
    }
    return curr;
  }

public:
  size_t size() const { return revmap.size(); }

  // puts a set,value pair
  void put(std::vector<K> const &ks, V val) {
    auto curr = traverse(ks, true);
    if (curr->value)
      revmap.erase(revmap.find(*(curr->value)));
    curr->value = std::make_unique<V>(val);
    revmap[val] = curr;
  }

  V put_or_get(std::vector<K> const &ks, V val) {
    auto *curr = traverse(ks, true);
    if (!curr->value) {
      curr->value = std::make_unique<V>(val);
      revmap[val] = curr;
    }
    return *(curr->value);
  }

  bool has(std::vector<K> const &ks) {
    auto *curr = traverse(ks, false);
    if (!curr || !curr->value)
      return false;
    return true;
  }

  bool has(V val) { return revmap.find(val) != revmap.end(); }

  V get(std::vector<K> const &ks) {
    auto const *curr = traverse(ks, false);
    return *(curr->value);
  }

  std::vector<K> get(V val) {
    auto const *curr = revmap[val];
    std::vector<K> ret;
    while (curr->parent) {
      ret.push_back(curr->key);
      curr = curr->parent;
    }
    return ret;
  }
};
}
