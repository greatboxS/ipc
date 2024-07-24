#ifndef SOCKET_LOCAL_H
#define SOCKET_LOCAL_H
#include <memory>
#include <string>

namespace ipc::core {

class accept_client {
private:
    class impl;
    std::unique_ptr<impl> m_impl{nullptr};

public:
    accept_client();

    

};

class socket_local_server {
private:
    class impl;
    std::unique_ptr<impl> m_impl{nullptr};

public:
    socket_local_server(const std::string &path);
    ~socket_local_server();

    int open();
    int close();
    int bind();
    int listen(size_t max);
    int 
};

class socket_local_client {
private:
    class impl;
    std::unique_ptr<impl> m_impl{nullptr};

public:
    socket_local_client();
    ~socket_local_client();

    int open();
    int close();
    int connect(const std::string &path);
    int disconnect();

    int send(const char *data, size_t size);
    int recv(char *buff, size_t size);
};

} // namespace ipc::core

#endif // SOCKET_LOCAL_H