#include "concurrent/task.h"
#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>
#include <thread>

using namespace ipc::core;

// Mock function for testing tasks with return values
int add(int a, int b) {
    return a + b;
}

// Mock function for testing tasks with void return type
void print_message(const std::string &message) {
    // This is just a placeholder; in real tests, you may want to verify output using a mock or a spy
}

// Mock callback to verify callback functionality
void callback(std::shared_ptr<ipc::core::task_base> task) {
    // Use this callback to change some external state in more complex tests
}

// Test fixture for task
class TaskTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

// Test that tasks with return types execute correctly and return expected results
TEST_F(TaskTest, TaskWithReturnTypeExecutesCorrectly) {
    auto t = make_task(add, callback, 3, 5);
    try {
        t->execute();
    } catch (...) {}
    const auto *result = t->get();
    ASSERT_TRUE(result != nullptr);
    EXPECT_EQ(result->data<int>(0), 8);
    EXPECT_TRUE(t->finished());
    EXPECT_FALSE(t->error());
}

// Test that tasks without return types execute correctly
TEST_F(TaskTest, TaskWithVoidReturnTypeExecutesCorrectly) {
    auto t = make_task(print_message, callback, "Hello, World!");
    try {
        t->execute();
    } catch (...) {}
    EXPECT_TRUE(t->finished());
    EXPECT_FALSE(t->error());
}

// Test task state transitions
TEST_F(TaskTest, TaskStateTransitionsCorrectly) {
    auto t = make_task(add, callback, 1, 2);
    EXPECT_EQ(t->state(), static_cast<int>(ipc::core::task_base::State::Created));

    try {
        t->execute();
    } catch (...) {}
    EXPECT_EQ(t->state(), static_cast<int>(ipc::core::task_base::State::Finished));
}

// Test task exception handling
TEST_F(TaskTest, TaskExceptionHandling) {
    auto t = make_task([]() -> int { throw std::runtime_error("Test error"); }, callback);
    try {
        t->execute();
    } catch (...) {}

    EXPECT_TRUE(t->error());
    EXPECT_FALSE(t->finished());
    auto exc_ptr = t->exception_ptr();
    ASSERT_TRUE(exc_ptr != nullptr);

    try {
        std::rethrow_exception(exc_ptr);
    } catch (const std::runtime_error &e) {
        EXPECT_STREQ(e.what(), "Test error");
    }
}

// Test task callback invocation
TEST_F(TaskTest, TaskCallbackInvocation) {
    bool callback_invoked = false;
    auto t = make_task(add, [&](std::shared_ptr<task_base> task) { callback_invoked = true; }, 4, 5);

    try {
        t->execute();
    } catch (...) {}
    EXPECT_TRUE(callback_invoked);
    EXPECT_TRUE(t->finished());
}

// Test task result retrieval timeout
TEST_F(TaskTest, TaskGetTimeout) {
    auto t = make_task([]() -> int {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        return 42;
    },
                       callback);

    auto result = t->get(100);  // Timeout of 100 ms
    EXPECT_EQ(result, nullptr); // Should be nullptr due to timeout
}

// Basic test for task that returns a result
TEST_F(TaskTest, TaskReturnsCorrectResult) {
    auto func = [](int a, int b) -> int {
        return a + b;
    };

    auto callback = [](std::shared_ptr<task_base>) {
        // Callback function (can be left empty for basic tests)
    };

    auto t = make_task(func, callback, 3, 4);
    try {
        t->execute();
    } catch (...) {}
    const auto *result = t->get(1000);
    ASSERT_EQ(result->get<int>(0), 7); // Expect 3 + 4 = 7
}

// Test that verifies the callback is called
TEST_F(TaskTest, CallbackIsInvoked) {
    bool callback_called = false;

    auto func = [](int a, int b) -> int {
        return a * b;
    };

    auto callback = [&callback_called](std::shared_ptr<task_base>) {
        callback_called = true;
    };

    auto t = make_task(func, callback, 3, 5);
    try {
        t->execute();
    } catch (...) {}

    EXPECT_TRUE(callback_called); // Check that callback was invoked
}

// Test with lambda that throws an exception
TEST_F(TaskTest, TaskThrowsException) {
    auto func = [](int) -> int {
        throw std::runtime_error("Intentional Exception");
    };

    auto callback = [](std::shared_ptr<task_base>) {
        // Callback function can remain empty for this test
    };

    auto t = make_task(func, callback, 1);
    try {
        t->execute();
    } catch (...) {}

    EXPECT_TRUE(t->error());                // Check that task reported an error
    ASSERT_NE(t->exception_ptr(), nullptr); // Ensure an exception was caught
}

// Test with std::bind for a function with multiple arguments
TEST_F(TaskTest, TaskWithBindFunction) {
    auto bound_func = std::bind([](int a, int b) { return a - b; }, 10, 3);

    auto callback = [](std::shared_ptr<task_base>) {
        // Callback can be left empty for this test
    };

    auto t = make_task(bound_func, callback);
    try {
        t->execute();
    } catch (...) {}
    const auto *result = t->get(1000);
    ASSERT_EQ(result->get<int>(0), 7); // Expect 10 - 3 = 7
}

// Test task with void return type and lambda function
TEST_F(TaskTest, TaskWithVoidReturn) {
    bool executed = false;

    auto func = [&executed](int x) {
        if (x == 5) {
            executed = true;
        }
    };

    auto callback = [](std::shared_ptr<task_base>) {
        // Callback can be left empty for this test
    };

    auto t = make_task(func, callback, 5);
    try {
        t->execute();
    } catch (...) {}

    EXPECT_TRUE(executed); // Check if the lambda function was executed
}

// Test task with a function pointer
void mock_function(int a, int b, int &result) {
    result = a + b;
}

TEST_F(TaskTest, TaskWithFunctionPointer) {
    int result = 0;

    auto callback = [](std::shared_ptr<task_base>) {
        // Callback can be left empty for this test
    };

    auto t = make_task(mock_function, callback, 7, 8, std::ref(result));
    try {
        t->execute();
    } catch (...) {}

    EXPECT_EQ(result, 15); // Expect 7 + 8 = 15
}

// Test for task that verifies wait_for with timeout
TEST_F(TaskTest, TaskWaitForTimeout) {
    auto func = [](int x) -> int {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        return x * 2;
    };

    auto callback = [](std::shared_ptr<task_base>) {
        // Callback can be left empty for this test
    };

    auto t = make_task(func, callback, 10);
    std::thread t1([t]() {
        try {
            t->execute();
        } catch (...) {}
    });
    t1.detach();

    const auto *result = t->get(100); // Set timeout to 100ms
    EXPECT_EQ(result, nullptr);       // Expect no result because it should timeout
}
