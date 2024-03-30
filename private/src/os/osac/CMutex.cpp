#include "CMutex.h"
#include "dbg/Debug.h"
#include "osal/Mutex.h"
#include <string.h>

namespace gbs {
namespace osac {
/**
 * @brief Construct a new CMutex::CMutex object
 *
 */
CMutex::CMutex() :
    m_strMtxName(NULL),
    m_s32IsOpen(-1) {}

/**
 * @brief Destroy the CMutex::CMutex object
 *
 */
CMutex::~CMutex() {
    if (m_strMtxName) delete[] m_strMtxName;
    Destroy();
}

/**
 * @fn Create
 * @brief Create Mutex
 *
 * @param name
 * @param recursive
 * @return int
 */
int CMutex::Create(const char *name, unsigned int recursive) {
    int len = 0;
    if (name) {
        len = strlen(name);
        m_strMtxName = new char[len];
        assert(m_strMtxName);
        strncpy(m_strMtxName, name, len);
    }

    m_s32IsOpen = osal::MUTEX_Create(m_stMtx, name, recursive);
    return m_s32IsOpen;
}

/**
 * @fn Lock
 * @brief Lock Mutex
 *
 * @return int
 */
int CMutex::Lock(int timeout) {
    if (m_s32IsOpen < 0) return -1;
    return osal::MUTEX_Lock(m_stMtx, timeout);
}

/**
 * @fn TryLock
 * @brief Trylock Mutex
 *
 * @return int
 */
int CMutex::TryLock() {
    if (m_s32IsOpen < 0) return -1;
    return osal::MUTEX_TryLock(m_stMtx);
}

/**
 * @fn UnLock
 * @brief Unlock Mutex
 *
 * @return int
 */
int CMutex::UnLock() {
    if (m_s32IsOpen < 0) return -1;
    return osal::MUTEX_UnLock(m_stMtx);
}

/**
 * @fn Destroy
 * @brief Destroy Mutex
 *
 * @return int
 */
int CMutex::Destroy() {
    if (m_s32IsOpen == -1) return -1;
    m_s32IsOpen = -1;
    return osal::MUTEX_Destroy(m_stMtx);
}
} // namespace osac
} // namespace gbs
