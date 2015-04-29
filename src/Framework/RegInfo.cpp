// Used by CPU classes to manage a list of of register structures.

#include "Framework/BasicCPU.hpp"
#include "Framework/RegInfo.hpp"

RegisterInformationList::RegisterInformationList(BasicCPU &cpu) {
  // Ask CPU to fill me up with RegisterInformation objects.
  cpu.BuildRegisterInformationList(*this);
}

RegisterInformationList::~RegisterInformationList() { }

void RegisterInformationList::Append(const std::string &name,
                                     const std::string &hexValue,
                                     const std::string &desc) {
  infos.push_back(RegisterInformation(name, hexValue, desc));
}

bool RegisterInformationList::Element(size_t index, RegisterInformation &info) {
  if (index >= infos.size()) { return false; }
  info = infos[index];
  return true;
}
