#include "piddefinitions.h"

PidDefinitions::PidDefinitions()
{

}

void PidDefinitions::add(PidDefinition &&pid)
{
    pid.valid = true;
    if (pids_.size() <= pid.id) {
        pids_.resize(pid.id + 1);
    }
    pids_[pid.id] = std::move(pid);
}

PidDefinition *PidDefinitions::get(uint32_t id)
{
    if (id >= pids_.size()) {
        return nullptr;
    }
    return &pids_[id];
}
