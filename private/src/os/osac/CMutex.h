#ifndef __CMUTEX_H__
#define __CMUTEX_H__

#include "osal/OSAL.h"

#define MtxSafeWrite(mtx, obj, val, cast) mtx.SafeWrite(obj, cast(val));
#define MtxSafeWriteInt(mtx, obj, val)    MtxSafeWrite(mtx, obj, val, static_cast<int>)
#define MtxSafeWriteInt32(mtx, obj, val)  MtxSafeWrite(mtx, obj, val, static_cast<int32_t>)
#define MtxSafeWriteUInt32(mtx, obj, val) MtxSafeWrite(mtx, obj, val, static_cast<uint32_t>)
#define MtxSafeWriteInt64(mtx, obj, val)  MtxSafeWrite(mtx, obj, val, static_cast<int64_t>)
#define MtxSafeWriteUInt64(mtx, obj, val) MtxSafeWrite(mtx, obj, val, static_cast<uint64_t>)

#define MtxSafeReadComp(mtx, obj, operator, val) (mtx.SafeRead(obj) operator(val))
#define MtxSafeReadEqual(mtx, obj, val)          MtxSafeReadComp(mtx, obj, ==, val)
#define MtxSafeReadDiff(mtx, obj, val)           MtxSafeReadComp(mtx, obj, !=, val)
#define MtxSafeReadLess(mtx, obj, val)           MtxSafeReadComp(mtx, obj, <, val)
#define MtxSafeReadGreater(mtx, obj, val)        MtxSafeReadComp(mtx, obj, >, val)
#define MtxSafeReadLessEqual(mtx, obj, val)      MtxSafeReadComp(mtx, obj, <=, val)
#define MtxSafeReadGreaterEqual(mtx, obj, val)   MtxSafeReadComp(mtx, obj, >=, val)

#define MtxSafeReadEqualInt32(mtx, obj, val)        MtxSafeReadEqual(mtx, obj, static_cast<int32_t>(val))
#define MtxSafeReadDiffInt32(mtx, obj, val)         MtxSafeReadDiff(mtx, obj, static_cast<int32_t>(val))
#define MtxSafeReadLessInt32(mtx, obj, val)         MtxSafeReadLess(mtx, obj, static_cast<int32_t>(val))
#define MtxSafeReadGreaterInt32(mtx, obj, val)      MtxSafeReadGreater(mtx, obj, static_cast<int32_t>(val))
#define MtxSafeReadLessEqualInt32(mtx, obj, val)    MtxSafeReadLessEqual(mtx, obj, static_cast<int32_t>(val))
#define MtxSafeReadGreaterEqualInt32(mtx, obj, val) MtxSafeReadGreaterEqual(mtx, obj, static_cast<int32_t>(val))

#define MtxSafeReadEqualUInt32(mtx, obj, val)        MtxSafeReadEqual(mtx, obj, static_cast<uint32_t>(val))
#define MtxSafeReadDiffUInt32(mtx, obj, val)         MtxSafeReadDiff(mtx, obj, static_cast<uint32_t>(val))
#define MtxSafeReadLessUInt32(mtx, obj, val)         MtxSafeReadLess(mtx, obj, static_cast<uint32_t>(val))
#define MtxSafeReadGreaterUInt32(mtx, obj, val)      MtxSafeReadGreater(mtx, obj, static_cast<uint32_t>(val))
#define MtxSafeReadLessEqualUInt32(mtx, obj, val)    MtxSafeReadLessEqual(mtx, obj, static_cast<uint32_t>(val))
#define MtxSafeReadGreaterEqualUInt32(mtx, obj, val) MtxSafeReadGreaterEqual(mtx, obj, static_cast<uint32_t>(val))

#define MtxSafeReadEqualInt64(mtx, obj, val)        MtxSafeReadEqual(mtx, obj, static_cast<int64_t>(val))
#define MtxSafeReadDiffInt64(mtx, obj, val)         MtxSafeReadDiff(mtx, obj, static_cast<int64_t>(val))
#define MtxSafeReadLessInt64(mtx, obj, val)         MtxSafeReadLess(mtx, obj, static_cast<int64_t>(val))
#define MtxSafeReadGreaterInt64(mtx, obj, val)      MtxSafeReadGreater(mtx, obj, static_cast<int64_t>(val))
#define MtxSafeReadLessEqualInt64(mtx, obj, val)    MtxSafeReadLessEqual(mtx, obj, static_cast<int64_t>(val))
#define MtxSafeReadGreaterEqualInt64(mtx, obj, val) MtxSafeReadGreaterEqual(mtx, obj, static_cast<int64_t>(val))

#define MtxSafeReadEqualUInt64(mtx, obj, val)        MtxSafeReadEqual(mtx, obj, static_cast<uint64_t>(val))
#define MtxSafeReadDiffUInt64(mtx, obj, val)         MtxSafeReadDiff(mtx, obj, static_cast<uint64_t>(val))
#define MtxSafeReadLessUInt64(mtx, obj, val)         MtxSafeReadLess(mtx, obj, static_cast<uint64_t>(val))
#define MtxSafeReadGreaterUInt64(mtx, obj, val)      MtxSafeReadGreater(mtx, obj, static_cast<uint64_t>(val))
#define MtxSafeReadLessEqualUInt64(mtx, obj, val)    MtxSafeReadLessEqual(mtx, obj, static_cast<uint64_t>(val))
#define MtxSafeReadGreaterEqualUInt64(mtx, obj, val) MtxSafeReadGreaterEqual(mtx, obj, static_cast<uint64_t>(val))

#define MtxSafeReadEqualInt(mtx, obj, val)        MtxSafeReadEqual(mtx, obj, static_cast<int>(val))
#define MtxSafeReadDiffInt(mtx, obj, val)         MtxSafeReadDiff(mtx, obj, static_cast<int>(val))
#define MtxSafeReadLessInt(mtx, obj, val)         MtxSafeReadLess(mtx, obj, static_cast<int>(val))
#define MtxSafeReadGreaterInt(mtx, obj, val)      MtxSafeReadGreater(mtx, obj, static_cast<int>(val))
#define MtxSafeReadLessEqualInt(mtx, obj, val)    MtxSafeReadLessEqual(mtx, obj, static_cast<int>(val))
#define MtxSafeReadGreaterEqualInt(mtx, obj, val) MtxSafeReadGreaterEqual(mtx, obj, static_cast<int>(val))

namespace gbs {
namespace osac {
class __DLL_DECLSPEC__ CMutex {
private:
    MUTEX_T m_stMtx;
    char *m_strMtxName;
    int32_t m_s32IsOpen;

public:
    CMutex();
    ~CMutex();

    int Create(const char *name = NULL, unsigned int recursive = 0);

    template <typename T>
    T SafeRead(T &obj, int *exp = NULL) {
        T ret;
        int err = Lock();
        ret = obj;
        UnLock();
        if (exp) *exp = err;
        return ret;
    }

    template <typename T>
    void SafeWrite(T &obj, T val, int *exp = NULL) {
        int err = Lock();
        obj = val;
        UnLock();
        if (exp) *exp = err;
    }

    int Lock(int timeout = 0);
    int TryLock();
    int UnLock();
    int Destroy();
};
}; // namespace osac
} // namespace gbs
#endif // __CMUTEX_H__