#ifndef __SINGLETON_H__
#define __SINGLETON_H__

#include "Typedef.h"

#define SINGLETON_CONSTRUCTOR(SingletonClass)   \
public:                                         \
    static SingletonClass *GetInstance();       \
    void Destroy();                             \
                                                \
private:                                        \
    static SingletonClass *Instance;            \
    SingletonClass(){};                         \
    ~SingletonClass(){};                        \
    SingletonClass(SingletonClass &) = delete;  \
    SingletonClass(SingletonClass &&) = delete; \
    SingletonClass operator=(SingletonClass const &) = delete;
#endif // __SINGLETON_H__