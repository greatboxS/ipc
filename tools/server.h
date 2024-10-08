
/*
 * This file was automatically generated by sdbus-c++-xml2cpp; DO NOT EDIT!
 */

#ifndef __sdbuscpp__server_h__adaptor__H__
#define __sdbuscpp__server_h__adaptor__H__

#include <sdbus-c++/sdbus-c++.h>
#include <string>
#include <tuple>

namespace com {
namespace example {

class MyInterface_adaptor {
public:
    static constexpr const char *INTERFACE_NAME = "com.example.MyInterface";

protected:
    MyInterface_adaptor(sdbus::IObject &object) :
        object_(object) {
        object_.registerSignal("clientError").onInterface(INTERFACE_NAME).withParameters<std::string, int32_t>("name", "code");
        object_.registerSignal("serverError").onInterface(INTERFACE_NAME).withParameters<std::string, int32_t>("name", "code");
        object_.registerMethod("sendMessage").onInterface(INTERFACE_NAME).withInputParamNames("dest", "message").withOutputParamNames("response").implementedAs([this](const std::string &dest, const std::string &message) {
            return this->sendMessage(dest, message);
        });
        object_.registerProperty("messageSize").onInterface(INTERFACE_NAME).withGetter([this]() { return this->messageSize(); });
        object_.registerProperty("messageCount").onInterface(INTERFACE_NAME).withGetter([this]() { return this->messageCount(); });
    }

    ~MyInterface_adaptor() = default;

public:
    void emitClientError(const std::string &name, const int32_t &code) {
        object_.emitSignal("clientError").onInterface(INTERFACE_NAME).withArguments(name, code);
    }

    void emitServerError(const std::string &name, const int32_t &code) {
        object_.emitSignal("serverError").onInterface(INTERFACE_NAME).withArguments(name, code);
    }

private:
    virtual int32_t sendMessage(const std::string &dest, const std::string &message) = 0;

private:
    virtual uint64_t messageSize() = 0;
    virtual uint64_t messageCount() = 0;

private:
    sdbus::IObject &object_;
};

} // namespace example
} // namespace com

#endif
