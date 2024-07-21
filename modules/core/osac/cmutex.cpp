#include "cmutex.h"
#include "osal/ipc_mutex.h"
#include <string.h>

namespace ipc::core {
/**
 * @brief Construct a new cmutex::cmutex object
 *
 */
cmutex::cmutex() :
    m_strMtxName(NULL),
    m_s32IsOpen(-1) {}

/**
 * @brief destroy the cmutex::cmutex object
 *
 */
cmutex::~cmutex() {
    if (m_strMtxName) delete[] m_strMtxName;
    destroy();
}

/**
 * @fn create
 * @brief create Mutex
 *
 * @param name
 * @param recursive
 * @return int
 */
int cmutex::create(const char *name, unsigned int recursive) {
    int len = 0;
    if (name) {
        len = strlen(name);
        m_strMtxName = new char[len];
        strncpy(m_strMtxName, name, len);
    }

    m_s32IsOpen = mutex_create(m_stMtx, name, recursive);
    return m_s32IsOpen;
}

/**
 * @fn lock
 * @brief lock Mutex
 *
 * @return int
 */
int cmutex::lock(int timeout) {
    if (m_s32IsOpen < 0) return -1;
    return mutex_lock(m_stMtx, timeout);
}

/**
 * @fn try_lock
 * @brief Trylock Mutex
 *
 * @return int
 */
int cmutex::try_lock() {
    if (m_s32IsOpen < 0) return -1;
    return mutex_try_lock(m_stMtx);
}

/**
 * @fn unlock
 * @brief Unlock Mutex
 *
 * @return int
 */
int cmutex::unlock() {
    if (m_s32IsOpen < 0) return -1;
    return mutex_unlock(m_stMtx);
}

/**
 * @fn destroy
 * @brief destroy Mutex
 *
 * @return int
 */
int cmutex::destroy() {
    if (m_s32IsOpen == -1) return -1;
    m_s32IsOpen = -1;
    return mutex_destroy(m_stMtx);
}
} // namespace ipc::core
