#pragma once

#include <queue>
#include <set>
#include <vector>
#include <map>

#include "common/util.hh"
#include "aut.hh"

namespace nbautils {

using namespace std;
using namespace nbautils;

//return list of states that can be reached from given state
template <typename T>
auto reachable_states(Aut<T> const& g, state_t from) {
  std::set<state_t> reached;
  bfs(from, [&](state_t const& st, auto const& pusher, auto const&) {
      reached.emplace(st);
      for (auto sucst : g.succ(st))
        pusher(sucst);
  });
  return vector<state_t>(cbegin(reached), cend(reached));
}

//return list of states that can NOT be reached from given state
template <typename T>
vector<state_t> unreachable_states(Aut<T> const& g, state_t from) {
  vector<state_t> allstates = g.states();
  assert(is_set_vec(allstates));
  return set_diff(allstates, reachable_states(g, from));
}


//returns a Node sequence with start and target included, if a path is found
template <typename T>
vector<state_t> find_path_from_to(Aut<T> const& g, state_t from, state_t to) {
  map<state_t, state_t> pred;
  bfs(from, [&](state_t const& st, auto const& visit, auto const&) {
      for (state_t q : g.succ(st)) {
        if (!map_has_key(pred, q)) {
          pred[q] = st;
          visit(q);
        }
      }
  });

  if (!map_has_key(pred, to))
    return {};

  vector<state_t> res;
  res.push_back(to);
  res.push_back(pred.at(to));
  while (res.back() != from) {
    res.push_back(pred.at(res.back()));
  }
  reverse(begin(res),end(res));
  return res;
}

template <typename T>
vector<sym_t> get_word_from_path(Aut<T> const& aut, vector<state_t> const& p) {
  assert(p.size() >= 2);

  vector<sym_t> w;
  unsigned int i=0;
  while (i<p.size()-1) {
    bool found = false;
    for (auto const x : aut.state_outsyms(p.at(i))) {
      if (contains(aut.succ(p.at(i), x), p.at(i+1))) {
        w.push_back(x);
        ++i;
        found = true;
        break;
      }
    }
    if (!found)
      throw runtime_error("impossible event: found no word from a path!");
  }
  return w;
}

}
