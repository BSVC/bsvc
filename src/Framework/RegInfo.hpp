#ifndef FRAMEWORK_REGINFO_HPP_
#define FRAMEWORK_REGINFO_HPP_

#include <string>
#include <vector>

class BasicCPU;

class RegisterInformation {
public:
  RegisterInformation(const std::string &n,
                      const std::string &v,
                      const std::string &d)
      : name(n), value(v), description(d) { }
  RegisterInformation() { }

  // Sets the name, hex value, and the description fields.
  void Set(const std::string &n,
           const std::string &v,
           const std::string &d) {
    name = n;
    value = v;
    description = d;
  }

  std::string Name() const { return name; }
  std::string HexValue() const { return value; }
  std::string Description() const { return description; }

private:
  std::string name;         // The name given to the register ("D0", "PC", etc).
  std::string value;        // The value of the register in hexidecimal.
  std::string description;  // A short description of the register.
};

class RegisterInformationList {
public:
  RegisterInformationList(BasicCPU &cpu);
  ~RegisterInformationList();

  void Append(const std::string &name, const std::string &value,
              const std::string &desc);

  size_t NumberOfElements() const { return infos.size(); }

  // Gets the element with the given index.
  // Returns true iff the index is valid.
  bool Element(size_t index, RegisterInformation &info);

private:
  std::vector<RegisterInformation> infos;
};

#endif  // FRAMEWORK_REGINFO_HPP_
