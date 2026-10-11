#include <utility>

/// An exception guard is a helper class for writing code with the strong exception guarantee. 
/// When writing code that can throw an exception, we can store rollback instructions in an exception guard,
// so it will be rolled back automatically. When an exception guard is done, we must mark it as complete, 
/// so that the transaction is not rolled back, when the  guard is destroyed.
template <typename Rollback>
struct exception_guard{
    exception_guard() = delete;
    exception_guard(Rollback&& rollback) 
    : rollback_{ std::move(rollback) }
    , complete_{ false }
    {}

    exception_guard(exception_guard&& other) noexcept
    : rollback_{ std::move(other.rollback_) }
    , complete_{ std::exchange(other.complete_, true) }
    {}

    private:
    Rollback rollback_;
    bool complete_;
};