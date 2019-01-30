#ifndef PIDDEFINITIONS_H
#define PIDDEFINITIONS_H

#include <string>
#include <vector>

struct PidDefinition {
    std::string name;
    std::string description;
    std::string formula;
    std::string unit;
    uint32_t id;
    uint16_t code;

    bool valid = false;
};

class PidDefinitions {
public:
    PidDefinitions() = default;

    void add(PidDefinition &&pid);

    PidDefinition *get(uint32_t id);

    const std::vector<PidDefinition> &pids() const { return pids_; }

private:
    std::vector<PidDefinition> pids_;
};

#endif // PIDDEFINITIONS_H
