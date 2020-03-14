#ifndef LT_LIBRETUNER_H
#define LT_LIBRETUNER_H

#include <functional>

namespace lt
{

using LogCallback = std::function<void(const std::string & message)>;

void setLogCallback(LogCallback && cb);
void log(const std::string & message);

} // namespace lt

#endif
