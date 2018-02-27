#pragma once

#include <list>
#include <vector>

namespace nbautils {

// This provides a relative order structure. Input is some numeric values
// representing a linear order. Returns references to according numeric values
// that can be dereferenced for comparison (</=/>) and capability to kill any
// one so that its order becomes the biggest. Then allows to convert back to
// normalized relative order.
//
// Conversion process O(n),
// comparisons and killing O(1) in any rational use case after more than MAXINT
// operations needs renormalization needs an integer type bigger than the number
// of elements we want to deal with for this to work and becomes expensive when
// number of operations or items is of same order of magnitude as integer size,
// then an AVL-tree based approach should be preferred with log(n)
//
// Technically, this is just a thin wrapper around a doubly linked list and the
// references to all of its elements for quick access.

class RelOrder {
 public:
  using ord_t = unsigned;
  using ordref_raw = std::list<ord_t>::iterator;
   class ordref { //wrap a mutable iterator (should not be written to from the outside)
     friend RelOrder;
      ordref_raw ref;
     public:
      ordref(ordref_raw it) : ref(it) {}
      ord_t operator*() const { return *ref; }
      ord_t operator==(ordref const other) const { return ref == other.ref; }
   };

 private:
  std::list<ord_t> order;

  bool normalized;
  ord_t nextfree;

 public:
  // create space for n order elements
  RelOrder(int n);
  // kill element referenced, give fresh order element
  ordref kill(ordref &ref);

  std::vector<ordref> from_ranks(std::vector<ord_t> const &ranks);
  std::vector<ord_t> to_ranks(std::vector<ordref> const &refs);
  void normalize();  // renumber
};

}  // namespace nbautils