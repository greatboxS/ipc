#include "osal/system_time.h"
#include <time.h>

namespace ipc::core {
TIMESTAMP_T get_timestamp() {
    int64_t ltime;
    time((time_t *)&ltime);
    struct tm *timeinfo = gmtime((time_t *)&ltime); /* Convert to UTC */
    ltime = mktime(timeinfo);                       /* Store as unix timestamp */
    return ltime;
}
} // namespace ipc::core