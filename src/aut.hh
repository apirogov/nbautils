#pragma once

#include <cassert>
#include <functional>
#include <algorithm>
#include <cinttypes>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <range/v3/all.hpp>

#include "common/types.hh"
#include "common/parity.hh"
#include "common/bimap.hh"

namespace nbautils {
using namespace std;

// parity automaton with unique initial state,
// priorities at nodes or edges and an arbitrary label at nodes
// can represent (Co)Büchi as well
template <typename T>
class Aut {
public:
  using uptr = unique_ptr<Aut<T>>;
  using sptr = shared_ptr<Aut<T>>;

  // using tag_printer = function<std::string(const T&)>;

private:
  bool normalized = true; //using state ids 0..n
  bool sba = false; //is state-based (priorities for nodes)

  // name in header, i.e. formula etc (for output later)
  string name = "";

  //atomic prop names (just for output later)
  vector<string> aps = {};

  // parity condition type: max even / min odd / etc
  PAType patype = PAType::MIN_EVEN;

  // initial state
  state_t init;
  //all used priorities with count
  map<pri_t, int> prio_cnt;
  // state priorities (if SBA)
  map<state_t, pri_t> state_pri;

  // source -> sym -> target -> edge label
  map<state_t, map<sym_t, map<state_t, pri_t>>> adj;

public:
  // node tags
  naive_unordered_bimap<T, state_t> tag;
  // default tag printing function
  function<void(ostream&,T const&)> tag_to_str = [](ostream& out, T const&){ out << "<?>"; };

  void print_state_tag(ostream& out, state_t const s) const {
    if (tag.hasi(s))
      tag_to_str(out, tag.geti(s));
      // out << tag.geti(s);
  }

  bool is_sba() const { return sba; }
  void set_sba(bool b) { sba = b; }
  // convert state-based to transition-based by shifting pris to edges
  void to_tba() {
    assert(is_sba());
    for (auto const p : states()) {
      auto const pri = has_pri(p) ? get_pri(p) : -1; //get state prio if any
      set_pri(p,-1); //remove state priority
      for (auto const x : state_outsyms(p)) //add it to all outgoing edges
        for (auto &es : adj.at(p).at(x)) {
          es.second = pri;
          prio_cnt[pri]++;
        }
    }
    set_sba(false);
  }

  string const& get_name() const { return name; }
  void set_name(string const& n) { name = n; }

  size_t num_syms() const { return 1 << aps.size(); }
  auto syms() const { return ranges::view::ints((int)0,(int)num_syms()); }

  vector<string> const& get_aps() const { return aps; }
  void set_aps(vector<string> const& ap) {
    assert(aps.empty()); // Can set APs only once!
    aps = ap;
  }

  PAType get_patype() const { return patype; }
  void set_patype(PAType t) { patype = t; }

  void set_init(state_t initial) {
    assert(has_state(initial)); // set must already exist!
    init = initial;
  }
  state_t get_init() const { return init; }
  bool is_init(state_t const s) const { return contains(init, s); }

  auto pris() const { return ranges::view::keys(prio_cnt); }
  pair<pri_t,pri_t> pri_bounds() const {
    if (prio_cnt.empty())
      return pa_acc_is_even(get_patype()) ? make_pair(1,1) : make_pair(0,0);
    return make_pair(begin(prio_cnt)->first, rbegin(prio_cnt)->first);
  }

  Aut(bool statebased=false, string const& title="",
      vector<string> const& ap={}, state_t initial={})
    : sba(statebased), name(title), aps(ap) {
      add_state(initial);
      set_init(initial);
  }

  size_t num_states() const { return adj.size(); }
  auto states() const { return ranges::view::keys(adj); }
  bool has_state(state_t const s) const { return map_has_key(adj, s); }

  // add a new state (must have unused id)
  void add_state(state_t const s) {
    assert(!has_state(s));

    if (s != num_states()) { //not densely used state ids
      normalized = false;
    }

    adj[s] = {}; //mark as created
  }

  // priorities marked on nodes

  bool has_pri(state_t const s) const {
    // assert(is_sba());
    assert(has_state(s));
    return map_has_key(state_pri, s);
  }
  pri_t get_pri(state_t const s) const {
    assert(is_sba());
    assert(has_pri(s));
    return state_pri.at(s);
  }

  void set_pri(state_t const s, pri_t p) {
    assert(is_sba());
    assert(has_state(s)); // otherwise adding acceptance to non-exinting state

    // remove count of old, if any
    if (has_pri(s)) {
      auto const old = state_pri.at(s);
      if (map_has_key(prio_cnt, old)) {
        prio_cnt.at(old)--;
        if (prio_cnt.at(old) == 0)
          prio_cnt.erase(prio_cnt.find(old));
      }
    }

    // overwrite and increase counts of new, if any provided
    if (p>=0) {
      state_pri[s] = p;
      prio_cnt[p]++;
    } else { //remove if negative
      if (has_pri(s))
        state_pri.erase(state_pri.find(s));
    }
  }

  // --------------------------------------------

  auto state_outsyms(state_t const p) const {
    assert(has_state(p));
    return ranges::view::keys(adj.at(p));
  }

  bool state_has_outsym(state_t const p, sym_t const x) const {
    return map_has_key(adj.at(p), x);
  }

  bool has_edge(state_t const p, sym_t const x, state_t const q) {
    return map_has_key(adj.at(p), x) && map_has_key(adj.at(p).at(x), q);
  }
  void add_edge(state_t const p, sym_t const x, state_t const q, pri_t pri=-1) {
#ifndef NDEBUG
    assert(has_state(p));
    assert(x < num_syms());
    assert(has_state(q));
    assert(pri < 0 || !sba);
    assert(!has_edge(p,x,q));
#endif

    adj.at(p)[x][q] = pri;
    if (pri>=0) {
      prio_cnt[pri]++;
    }
  }

  //modify an existing edge
  void mod_edge(state_t const p, sym_t const x, state_t const q, pri_t pri=-1) {
    auto const oldpri = adj.at(p).at(x).at(q); //gives exception if does not exist
    if (oldpri>=0) {
      prio_cnt[oldpri]--;
      if (prio_cnt.at(oldpri) == 0)
        prio_cnt.erase(prio_cnt.find(oldpri));
    }
    adj[p][x][q] = pri;
    if (pri>=0) {
      prio_cnt[pri]++;
    }
  }

  //remove an existing edge
  void remove_edge(state_t const p, sym_t const x, state_t const q) {
    auto const epri = adj.at(p).at(x).at(q); //gives exception if does not exist
    if (epri>=0) {
      prio_cnt[epri]--;
      if (prio_cnt.at(epri) == 0)
        prio_cnt.erase(prio_cnt.find(epri));
    }
    adj.at(p).at(x).erase(adj.at(p).at(x).find(q)); //kill edge
  }

  // return all successors with edge label (-1 means no label)
  map<state_t,pri_t> const& succ_edges_raw(state_t const p, sym_t const x) const {
    assert(has_state(p));
    assert(state_has_outsym(p,x));
    return adj.at(p).at(x);
  }

  // x-label edge successors
private:
  map<state_t,pri_t> emptysucc;
public:

  // return all successors with edge label (-1 means no label, no entry means no edge)
  map<state_t,pri_t> const& succ_edges(state_t const p, sym_t const x) const {
    assert(has_state(p));
    // assert(state_has_outsym(p,x));

    auto const& edges = adj.at(p);
    if (!map_has_key(edges, x)) return emptysucc;
    return edges.at(x);
  }

  // return all successors for a symbol (without edge label)
  auto const succ(state_t const p, sym_t const x) const {
    return ranges::view::keys(succ_edges(p, x));
  }

  // return all successors (independent of symbol)
  vector<state_t> succ(state_t const p) const {
    assert(has_state(p));
    auto const& syms = adj.at(p);

    // collect successors for any symbol
    set<state_t> s;
    for (auto const& it : syms) {
      for (auto const &jt : it.second) {
        s.emplace(jt.first);
      }
    }
    return vector<state_t>(begin(s), end(s));
  }

  // --------------------------------------------

  //Buchi = SBA + at most two priorities and the best (explicit) is a good one
  bool is_buchi() const {
    vector<pri_t> best = pris();
    auto better = stronger_priority_f(patype);
    ranges::sort(best, [&](int p, int q){ return p != better(p,q); });
    return is_sba() && (best.size()< 2
                    || (best.size()==2 && good_priority(patype, best.back())) );
  }

  // does not check whether it is really buchi
  // if automaton is Buchi, a state is acc. iff it is marked with good priority
  bool state_buchi_accepting(state_t const s) const {
    return has_pri(s) && good_priority(patype, get_pri(s));
  }

  //at most one outgoing edge per sym
  bool is_deterministic() const {
    for (auto const p : states())
      for (auto const i : state_outsyms(p))
        if (succ(p,i).size() > 1)
          return false;
    return true;
  }

  //for each sym, at least one outgoing edge
  bool is_complete() const {
    for (auto const p : states())
      for (auto const i : syms())
        if (succ(p,i).empty())
          return false;
    return true;
  }

  //if not complete, add rejecting sink and missing edges
  void make_complete() {
    if (is_complete() || num_syms()==0)
      return;

    state_t rejsink = num_states();
    add_state(rejsink);

    pri_t rejpri = pa_acc_is_even(patype) ? 1 : 0;
    if (sba)
      set_pri(rejsink, rejpri); //a rejecting prio

    //missing edges -> edge to rejecting sink
    for (auto const st : states()) {
      for (auto const i : syms()) {
        if (succ(st, i).empty())
          add_edge(st, i, rejsink, sba ? -1 : rejpri);
      }
    }
  }

  // each state/edge has an assigned priority
  bool is_colored() const {
    for (auto const p : states()) {
      if (sba && !has_pri(p))
        return false;
      if (!sba) {
        for (auto const i : state_outsyms(p)) {
          for (auto const it : succ_edges(p,i)) {
            if (it.second == -1)
              return false;
          }
        }
      }
    }
    return true;
  }

  // each state/edge gets a weak priority that does not change semantics
  void make_colored() {
    assert(pa_acc_is_min(patype)); //for simplicity.

    //get a maximally weak bad priority
    pri_t badpri = pris().empty() ? 0 : pris().back();
    if (good_priority(patype, badpri))
      badpri++;

    for (auto const p : states()) {
      if (sba && !has_pri(p))
        set_pri(p,badpri);
      if (!sba) {
        for (auto const i : state_outsyms(p)) {
          for (auto& it : adj.at(p).at(i)) {
            if (it.second == -1)
              it.second = badpri; //set missing priority
          }
        }
      }
    }
  }

  // --------------------------------------------

  // given set in sorted(!!) vector, kill states + all their edges
  // if initial state included, first non-removed becomes initial
  void remove_states(vector<state_t> const& tokill) {
#ifndef NDEBUG
    assert(is_set_vec(tokill));
    for (auto s : tokill)
      assert(has_state(s));
#endif
    bool killinit = sorted_contains(tokill, get_init());

    // cerr << "removing " << seq_to_str(tokill) << endl;

    for (auto const& it : tokill) {
      if (sba && has_pri(it))
        set_pri(it, -1);  // kill priority
    }
    for (auto const& it : tokill) {
      if (tag.hasi(it))
        tag.erasei(it);  // kill tag
    }
    for (auto const& it : tokill) {
      if (map_has_key(adj,it))
        adj.erase(adj.find(it));  // kill state + outgoing edges
    }

    // kill states from edge targets
    for (auto& it : adj) { //from every state p
      for (auto& jt : it.second) { //and every symbol x
        for (auto const v : tokill) { //remove the target state
          if (map_has_key(jt.second,v))
            jt.second.erase(jt.second.find(v));
        }
      }
    }

    if (killinit)
      init = cbegin(adj)->first;

    normalized = false;
  }

  //stupidly paste another automaton (ignore initial states)
  void insert(Aut<T> const& other) {
    assert(get_aps() == other.get_aps()); //same alphabet
    assert(set_intersect_empty(states()       | ranges::to_vector,
                               other.states() | ranges::to_vector)); // no common states

    if (!normalized || !other.normalized || cbegin(other.adj)->first != num_states())
      normalized = false;

    for (auto const st : other.states()) {
      if (!has_state(st))
        add_state(st);
      if (sba && other.sba && other.has_pri(st))
        set_pri(st, other.get_pri(st));
      if (other.tag.hasi(st))
        tag.put(other.tag.geti(st), st);

      for (auto const sym : other.syms()) {
        for (auto const es : other.succ_edges(st, sym)) {
          if (!has_state(es.first))
            add_state(es.first);
          add_edge(st, sym, es.first, es.second);
        }
      }
    }
  }

  // given set in sorted(!!) vector, merge states into one rep. merged mustnot be initial
  // after merge graph not normalized
  void merge_states(vector<state_t> const& others, state_t rep) {
    if (others.empty()) return; //nothing to do

#ifndef NDEBUG
    assert(has_state(rep));
    for (auto q : others)
      assert(has_state(q));
    assert(is_set_vec(others));
    assert(!sorted_contains(others, get_init()));
    assert(!sorted_contains(others, rep));
#endif

    // add ingoing edges into all class members to representative (with same edge prio)
    for (auto const st : states()) {
      for (auto const sym : state_outsyms(st)) {
        auto const tokill_sucs = set_intersect(succ(st, sym) | ranges::to_vector, others);
        if (tokill_sucs.empty())
          continue;

        pri_t const epri = succ_edges(st, sym).at(tokill_sucs.front());
        add_edge(st, sym, rep, epri);
      }
    }

    // finally kill the others!!
    remove_states(others);
  }

  //given equivalence classes, perform merges
  void quotient(vector<vector<state_t>> const& equiv) {
    auto initial = get_init();
    bool seenini = false;
    for (auto ecl : equiv) {
      if (ecl.size() < 2)
        continue; //nothing to do

      auto rep = ecl.back();
      if (!seenini) {
        auto it = lower_bound(begin(ecl), end(ecl), initial);
        if (it != end(ecl) && *it == initial) {
          ecl.erase(it);
          rep = initial;
          seenini = true;
        } else {
          ecl.pop_back();
        }
      } else {
        ecl.pop_back();
      }

      // cerr << "merging " << seq_to_str(ecl) << " into " << rep << endl;

      merge_states(ecl, rep);
    }
  }

  map<state_t, state_t> normalize(state_t const offset=0) {
    map<state_t, state_t> m;
    tie(*this, m) = get_normalized(offset);
    normalized = true;
    return m;
  }

  //renumber all states continuously starting from provided offset
  pair<Aut<T>, map<state_t, state_t>> get_normalized(state_t const offset=0) {
    //calculate state renumbering
    map<state_t, state_t> m;
    state_t i=offset;
    bool needs_renumbering = false;
    for (auto const& st : states()) {
      m[st] = i++;
      // cerr << st << " -> " << m[st] << endl;
      if (m.at(st) != st)
        needs_renumbering = true;
    }
    if (!needs_renumbering) { //everything is fine already -> return copy
      auto const ret = *this; //copy
      map<state_t,state_t> retmap;
      for (auto const s : ret.states())
        retmap[s] = s;
      return make_pair(move(ret), move(retmap));
    }

    // cerr << "offset: " << offset << endl;
    // cerr << "old init: " << init << endl;
    // cerr << "mapped init: " << m[init] << endl;
    Aut<T> ret(sba, name, aps, m[init]);
    // cerr << seq_to_str(ret.states() | ranges::to_vector) << endl;
    ret.init = m[init];
    ret.patype = patype;
    ret.tag_to_str = tag_to_str;
    for (auto const& st : states()) {
      if (!ret.has_state(m[st]))
        ret.add_state(m[st]);
      if (tag.hasi(st))
        ret.tag.put(tag.geti(st), m[st]);
      if (sba && has_pri(st))
        ret.set_pri(m[st], get_pri(st));
      for (auto const& sym : state_outsyms(st)) {
        for (auto const& es : succ_edges(st, sym)) {
          if (!ret.has_state(m[es.first]))
            ret.add_state(m[es.first]);
          ret.add_edge(m[st], sym, m[es.first], es.second);
        }
      }
    }

    return make_pair(move(ret), move(m));
  }

};

//common stuff working on any automaton

function<vector<state_t>(state_t)> aut_succ(auto const& aut) {
  return [&aut](state_t p){ return aut.succ(p); };
}

//adjacency matrix for NBA to speed things up
using adj_mat = vector<vector<nba_bitset>>;
adj_mat get_adjmat(auto const& aut) {
  auto const n = 1+ranges::max(aut.states());
  assert(n <= nba_bitset(0).size());

  adj_mat mat(aut.num_syms(), vector<nba_bitset>(n, nba_bitset(0)));
  for (state_t const p : aut.states()) {
    for (sym_t const x : aut.state_outsyms(p)) {
      for (state_t const q : aut.succ(p,x)) {
        mat.at(x).at(p)[q] = 1;
      }
    }
  }
  return mat;
}

//takes adj matrix, set of source states, transition symbol
//a set of accepting sinks
//a complete map of strict subsumptions (if bit i is set, &= with corresponding mask)
//returns successors
inline nba_bitset powersucc(adj_mat const& mat, nba_bitset from, sym_t x, nba_bitset sinks=0, map<unsigned,nba_bitset> impl_mask={}) {
  // cerr << pretty_bitset(from) << ", " << (int)x << endl;
  nba_bitset ret = 0;
  auto const& xmat = mat[x];
  //collect all successors
  for (int const i : ranges::view::ints(0, (int)ret.size())) {
    if (from[i])
      ret |= xmat[i];
  }
  if ((ret & sinks) != 0) //reached acc sink
    return sinks;

  //remove subsumed states
  if (!impl_mask.empty())
    for (int const i : ranges::view::ints(0, (int)ret.size())) {
      if (ret[i] && map_has_key(impl_mask, (unsigned)i))
        ret &= impl_mask[i];
    }

  return ret;
}

}  // namespace nbautils
