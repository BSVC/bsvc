//
// Used by BasicCPU (and derived classes) to manage a list of
// of statistics objects.
//

#ifndef FRAMEWORK_STATINFO_HXX
#define FRAMEWORK_STATINFO_HXX

#include <string>
#include <list>

class BasicCPU;

class StatisticInformation {
public:
  StatisticInformation(const std::string &statistic) : myStatistic(statistic) { }
  StatisticInformation() { }
  ~StatisticInformation() { }

  // Set the statistic fields.
  void Set(const std::string &statistic) { myStatistic = statistic; }

  // Returns my statistic.
  const std::string &Statistic() const { return myStatistic; }

private:
  std::string myStatistic;
};

class StatisticalInformationList {
public:
  StatisticalInformationList(BasicCPU &cpu);
  ~StatisticalInformationList();

  // Appends an element to the list.
  void Append(const std::string &statistic);

  // Returns the number of elements in the list.
  unsigned int NumberOfElements() const { return myList.size(); }

  // Gets the element with the given index. Returns true iff successful.
  bool Element(unsigned int index, StatisticInformation &info);

private:
  std::list<StatisticInformation *> myList;
};

#endif  // FRAMEWORK_STATINFO_HXX
