//
// Created by altenius on 4/9/19.
//

#include "job.h"

namespace lt
{

void JobControl::setProgress(double progress) noexcept
{
    job_->progress_ = progress;
    job_->eventProgress_(progress);
}

} // namespace lt