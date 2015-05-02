#include "Framework/BasicCPU.hpp"
#include "Framework/StatInfo.hpp"

StatisticalInformationList::StatisticalInformationList(BasicCPU &cpu) {
  cpu.BuildStatisticalInformationList(*this);
}

StatisticalInformationList::~StatisticalInformationList() {
  for (auto p : myList) delete p;
}

void StatisticalInformationList::Append(const std::string &statistic) {
  myList.push_back(new StatisticInformation(statistic));
}

bool StatisticalInformationList::Element(unsigned int index,
                                         StatisticInformation &info) {
  if (index >= NumberOfElements()) {
    return false;
  }
  auto *element = myList[index];
  info.Set(element->Statistic());
  return true;
}
