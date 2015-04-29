//
// This abstract base class provides methods to load object files into the
// the simulator.
//

#ifndef FRAMEWORK_BASICLOADER_HPP_
#define FRAMEWORK_BASICLOADER_HPP_

#include <string>

class BasicCPU;

class BasicLoader {
public:
  BasicLoader(BasicCPU &c) : myCPU(c) { }
  virtual ~BasicLoader() { }

  BasicCPU &CPU() { return myCPU; }

  // Loads the named file and answers an error message or the empty string.
  virtual std::string Load(const std::string &filename, int addressSpace) = 0;

protected:
  // Load into this CPU.
  BasicCPU &myCPU;
};

#endif
