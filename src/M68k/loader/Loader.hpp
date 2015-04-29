#ifndef M68K_LOADER_LOADER_HPP_
#define M68K_LOADER_LOADER_HPP_

#include <iosfwd>
#include <string>

#include "Framework/BasicLoader.hpp"

// Loads object files in Motorola S-Record format.
class Loader : public BasicLoader {
public:
  Loader(BasicCPU &c) : BasicLoader(c) { }

  // Loads the named file and returns an error message or the empty string.
  std::string Load(const std::string &filename, int addressSpace) override;

private:
  // Loads a Motorola S-Record file.
  std::string LoadMotorolaSRecord(std::ifstream &file, int addressSpace);
};

#endif  // M68K_LOADER_LOADER_HPP_
