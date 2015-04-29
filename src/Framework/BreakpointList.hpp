//
// Manage breakpoints.
//

#ifndef FRAMEWORK_BREAKPOINTLIST_HPP_
#define FRAMEWORK_BREAKPOINTLIST_HPP_

#include <iterator>
#include <set>

class BreakpointList {
public:
  // Adds a break point to the list.
  void Add(Address address) {
    breakpoints.insert(address);
  }

  // Deletes a break point from the list.
  // Returns true iff breakpoint was in the set.
  bool Delete(Address address) {
    return breakpoints.erase(address) == 1;
  }

  // Returns the number of breakpoints currently in the set.
  size_t NumberOfBreakpoints() const {
    return breakpoints.size();
  }

  // Gets the breakpoint with the given index.
  // Returns true iff the address was in the set.
  bool GetBreakpoint(size_t index, Address &address) const {
    if (index >= breakpoints.size()) { return false; }
    auto it = breakpoints.begin();
    std::advance(it, index);
    address = *it;
    return true;
  }

  // Returns true iff the given address is a breakpoint.
  bool Check(Address address) const {
    return breakpoints.find(address) != breakpoints.end();
  }

private:
  std::set<Address> breakpoints;
};

#endif  // FRAMEWORK_BREAKPOINTLIST_HPP_
