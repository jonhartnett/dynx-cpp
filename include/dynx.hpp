#ifndef DYNX_DYNX
#define DYNX_DYNX

#include <memory>
#include <functional>
#include <list>
#include <vector>

namespace dynx {
    constexpr struct emplace_t{} emplace;

    using Listener = std::function<void()>;
    using WeakListener = std::weak_ptr<Listener>;
    using Listeners = std::list<Listener>;
    using WeakListeners = std::list<WeakListener>;

    class DynxBase {
        template<class>
        friend class Dynx;
    protected:
        static thread_local std::vector<std::function<WeakListener(const std::shared_ptr<void>&)>> childStack;
        static thread_local std::vector<std::pair<std::weak_ptr<void>, WeakListeners*>> listenerQueue;

        static void runWeakListeners(std::weak_ptr<void> ptr, WeakListeners& listeners);
        static void runWeakListenerQueue();
    public:
        struct Token;

        [[nodiscard]] virtual const Listeners& listeners() const = 0;
        [[nodiscard]] virtual const WeakListeners& weakListeners() const = 0;
        virtual Token on(std::function<void()> listener) = 0;
        virtual Token on(std::weak_ptr<std::function<void()>> listener) = 0;
        virtual void off(Token token) = 0;

        virtual void update() = 0;
    };

    template<class T>
    class Dynx final : public DynxBase {
    private:
        using DynxBase::childStack;
        using DynxBase::listenerQueue;

        struct Body;
        std::shared_ptr<Body> body;
    public:
        Dynx(const T& value); // NOLINT(google-explicit-constructor)
        Dynx(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>); // NOLINT(google-explicit-constructor)
        template<class... Args>
        explicit Dynx(emplace_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>);
        explicit Dynx(std::function<T()> exp);
        Dynx(const Dynx&) = default;
        Dynx(Dynx&&) noexcept = default;

        [[nodiscard]] operator const T&() const; // NOLINT(google-explicit-constructor)
        Dynx& operator =(const T& val);
        Dynx& operator =(T&& val) noexcept(std::is_nothrow_move_assignable_v<T>);
        Dynx& operator =(std::function<T()> exp);
        Dynx& operator =(const Dynx&) = default;
        Dynx& operator =(Dynx&&) noexcept = default;

        [[nodiscard]] const T& value() const;

        void value(const T& val);
        void value(T&& val) noexcept(std::is_nothrow_move_assignable_v<T>);
        template<class... Args>
        void value(emplace_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>);

        [[nodiscard]] const std::function<T()>& expression() const;
        void expression(std::function<T()> exp);
    private:
        T evalExpression();
    public:

        [[nodiscard]] const Listeners& listeners() const override;
        [[nodiscard]] const WeakListeners& weakListeners() const override;
        Token on(std::function<void()> listener) override;
        Token on(std::weak_ptr<std::function<void()>> listener) override;
        void off(Token token) override;

        void update() override;
    };
}

#define DYNX_DYNX_CPP
#include "dynx.cpp.hpp"
#undef DYNX_DYNX_CPP

#endif //DYNX_DYNX