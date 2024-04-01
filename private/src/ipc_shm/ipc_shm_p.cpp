#include "include/ipc_shm_p.h"

namespace ipc {

void ipc_shm_private::lock() {
}
void ipc_shm_private::unlock() {
}
void ipc_shm_private::try_lock() {
}
void ipc_shm_private::try_unlock() {
}
void ipc_shm_private::try_shared_lock() {
}
void ipc_shm_private::try_shared_unlock() {
}
size_t ipc_shm_private::size() const {
}
void ipc_shm_private::read(uint8_t *data, size_t size) {
}
void ipc_shm_private::read(char *data, size_t size) {
}
void ipc_shm_private::write(const uint8_t *data, size_t size) {
}
void ipc_shm_private::write(const char *data, size_t size) {
}
void ipc_shm_private::write(const std::string &data) {
}

} // namespace ipc
