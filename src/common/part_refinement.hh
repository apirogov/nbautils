#pragma once

#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <utility>
#include <memory>

#include "common/util.hh"

namespace nbautils {
  using namespace std;

//partition refinement structure for a fixed number of elements
template<typename T>
class PartitionRefiner {
public:
  using vec_it = typename vector<T>::iterator;
  using bounds = pair<vec_it, vec_it>;
  using sym_set = typename list<bounds>::iterator;

private:
  vector<T> elements; //states are grouped by sets
  list<bounds> sets;  //set boundaries over elements as iterator pairs
  // map<T, sym_set> set_of; //map from states back to iterator pair

  vector<sym_set> sym_sets; //symbolic representation of sets (not ordered)

 public:
  size_t num_sets() const { return sets.size(); }
  size_t get_set_size(sym_set const& ref) const { return ref->second - ref->first; }

  //start with existing sets (or singleton set)
  PartitionRefiner(vector<vector<T>> const& startsets) {
    // cout << "copy" << endl;
    for (auto s : startsets)
      copy(cbegin(s),cend(s), back_inserter(elements));

    auto l = elements.begin();
    for (auto s : startsets) {
      auto r = l+s.size();

      // cout << "sort" << endl;
      sort(l, r);
      // cout << "add pair" << endl;
      sets.emplace_back(make_pair(l,r));
      // cout << "add backmap" << endl;
      auto sym = sets.end();
      --sym;
      sym_sets.push_back(sym);

      // for (auto el : s)
      //   set_of[el] = sym;

      l = r;
    }
    // cout << "constr done" << endl;
  };

  decltype(sets)& get_sets() {
    return sets;
  }

  //a set is identified by the iterator to the bound pair
  //returned vector has no specific order (just order of creation)
  vector<sym_set> get_set_ids() {
    return sym_sets;
    /*
    vector<sym_set> ret;
    ret.reserve(sets.size());
    for (auto it=begin(sets); it!=end(sets); ++it)
      ret.push_back(it);
    return ret;
    */
  }
  //convenience function - returns all sets
  vector<vector<T>> get_refined_sets() {
    vector<vector<T>> ret;
    for (auto it=begin(sets); it!=end(sets); ++it) {
      ret.push_back(get_elements_of(it));
    }
    return ret;
  }

  vector<T> get_elements_of(sym_set const& ref) {
    vector<T> ret;
    get_elements_of(ref, ret);
    return ret;
  }
  void get_elements_of(sym_set const& ref, vector<T> &ret) {
    ret.clear();
    ret.reserve(get_set_size(ref));
    for (auto it=ref->first; it!=ref->second; ++it)
      ret.push_back(*it);
    sort(begin(ret),end(ret));
  }

  // sym_set get_set_of(T const& el) const {
  //   assert(map_has_key(set_of, el));
  //   return set_of.at(el);
  // }

private:
  //split on nonempty intersection and difference
  //if both sides nonempty, returns token of second set, otherwise returns nullptr
  unique_ptr<sym_set> split_set(sym_set& set, vec_it const& mid) {
    if (mid == set->first || mid == set->second)
      return nullptr;
    auto newset = make_unique<sym_set>(sets.insert(set, make_pair(set->first, mid)));
    sym_sets.push_back(*newset);

    // for (auto it=(*newset)->first; it!=(*newset)->second; ++it)
    //   set_of[*it] = *newset; //update element -> set mapping

    set->first = mid;
    return newset;
  }

public:

  //separate given set into satisfying and not satisfying predicate
  //if both are nonempty, returns token of second set, otherwise returns nullptr
  unique_ptr<sym_set> separate(sym_set& set, function<bool(T)> const& pred) {
    auto const mid = partition(set->first, set->second, pred);
    return move(split_set(set, mid));
  }

};

}  // namespace nbautils
