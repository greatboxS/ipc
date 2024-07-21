#ifndef IPC_FILE_H
#define IPC_FILE_H

#include "osal.h"

namespace ipc::core {
__dll_declspec__ int file_open(FILE_T &file, const char *name, int mode);
__dll_declspec__ int file_create(FILE_T &file, const char *name);
__dll_declspec__ int file_close(FILE_T &file);
__dll_declspec__ int file_seek(FILE_T &file, long pos);
__dll_declspec__ int file_read(FILE_T &file, char *buff, size_t size);
__dll_declspec__ int file_write(FILE_T &file, const char *buff, size_t size);

__dll_declspec__ int file_exist(const char *file);
__dll_declspec__ int file_is_directory(const char *file);
} // namespace ipc::core
#endif // IPC_FILE_H