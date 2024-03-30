#include "osal/SystemTime.h"
#include <time.h>

namespace gbs {
namespace osal {
TIMESTAMP_T GetTimeStamp() {
    int64_t ltime;
    time((time_t *)&ltime);
    struct tm *timeinfo = gmtime((time_t *)&ltime); /* Convert to UTC */
    ltime = mktime(timeinfo);                       /* Store as unix timestamp */
    return ltime;
}
} // namespace osal
} // namespace gbs