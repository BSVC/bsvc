//   This is the abstract base class for all microprocessors

#include "Framework/BasicCPU.hpp"

BasicCPU::BasicCPU(const std::string &name, int granularity,
                   std::vector<AddressSpace *> &addressSpaces,
                   const std::string &traceRecordFormat,
                   const std::string &defaultTraceRecordEntries)
    : myAddressSpaces(addressSpaces),
      myName(name),
      myGranularity(granularity),
      myExecutionTraceRecord(traceRecordFormat),
      myDefaultExecutionTraceEntries(defaultTraceRecordEntries) { }

BasicCPU::~BasicCPU() { }
