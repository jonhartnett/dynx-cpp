#ifndef DYNX_DYNX_CPP
#error "Do not include dynx/dynx.cpp.hpp directly. Use dynx/dynx.hpp."
#else

#include <variant>
#include <cassert>

namespace dynx {
    struct DynxBase::Token {
        template<class>
        friend class Dynx;
    private:
        std::variant<Listeners::iterator, WeakListeners::iterator> it;

        explicit Token(std::variant<Listeners::iterator, WeakListeners::iterator> iter);
    public:
        Token(const Token&) = delete;
        Token(Token&&) noexcept = default;

        Token& operator =(const Token&) = delete;
        Token& operator =(Token&&) noexcept = default;
    };

    template<class T>
    struct Dynx<T>::Body {
        T value;
        std::function<T()> expression;
        std::shared_ptr<std::function<void()>> handle;
        Listeners listeners;
        WeakListeners weakListeners;

        template<class... Args>
        explicit Body(emplace_t, Args&&... args) :
            value(std::forward<Args>(args)...),
            expression(nullptr),
            handle(nullptr),
            listeners(),
            weakListeners()
        {}

        explicit Body(std::function<T()> exp) :
            expression(std::move(exp)),
            handle(nullptr),
            listeners(),
            weakListeners()
        {
            //this doesn't init value! caller must inplace-construct value
        }
    };

    template<class T>
    Dynx<T>::Dynx(const T& value) :
        body(std::make_shared<Body>(emplace, value))
    { }

    template<class T>
    Dynx<T>::Dynx(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>) :
        body(std::make_shared<Body>(emplace, std::move(value)))
    { }

    template<class T>
    template<class... Args>
    Dynx<T>::Dynx(emplace_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) :
        body(std::make_shared<Body>(emplace, std::forward<Args>(args)...))
    { }

    template<class T>
    Dynx<T>::Dynx(std::function<T()> exp) :
        body(std::make_shared<Body>(std::move(exp)))
    {
        new (&body->value) T(evalExpression());
    }

    template<class T>
    Dynx<T>::operator const T&() const {
        return value();
    }

    template<class T>
    auto Dynx<T>::operator =(const T& val) -> Dynx& {
        value(val);
        return *this;
    }

    template<class T>
    auto Dynx<T>::operator =(T&& val) noexcept(std::is_nothrow_move_assignable_v<T>) -> Dynx& {
        value(std::move(val));
        return *this;
    }

    template<class T>
    auto Dynx<T>::operator =(std::function<T()> exp) -> Dynx& {
        expression(std::move(exp));
        return *this;
    }

    template<class T>
    const T& Dynx<T>::value() const {
        // this annoying hack is required because the c++ standard didn't bother to mandate an
        //  is_empty for weak_ptr
        auto is_empty = [](const WeakListener& listener){
            const WeakListener empty{};
            return !listener.owner_before(empty) && !empty.owner_before(listener);
        };
        if(!childStack.empty()){
            auto handle = childStack.back()(body);
            if(!is_empty(handle))
                const_cast<Dynx<T>*>(this)->on(std::move(handle));
        }
        return body->value;
    }

    template<class T>
    void Dynx<T>::value(const T& val) {
        if(val != body->value)
            value(emplace, val);
    }

    template<class T>
    void Dynx<T>::value(T&& val) noexcept(std::is_nothrow_move_assignable_v<T>) {
        if(val != body->value)
            value(emplace, std::move(val));
    }

    template<class T>
    template<class... Args>
    void Dynx<T>::value(emplace_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
        if constexpr(std::is_nothrow_constructible_v<T, Args...>){
            body->value.~T();
            new (&body->value) T(std::forward<Args>(args)...);
        }else{
            body->value = T(std::forward<Args>(args)...);
        }
        body->expression = nullptr;
        update();
    }

    template<class T>
    const std::function<T()>& Dynx<T>::expression() const {
        return body->expression;
    }

    template<class T>
    void Dynx<T>::expression(std::function<T()> exp) {
        body->expression = std::move(exp);
        update();
    }

    template<class T>
    const Listeners& Dynx<T>::listeners() const {
        return body->listeners;
    }

    template<class T>
    const WeakListeners& Dynx<T>::weakListeners() const {
        return body->weakListeners;
    }

    template<class T>
    auto Dynx<T>::on(std::function<void()> listener) -> Token {
        auto it = body->listeners.insert(body->listeners.begin(), std::move(listener));
        return Token(it);
    }

    template<class T>
    auto Dynx<T>::on(std::weak_ptr<std::function<void()>> listener) -> Token {
        auto it = body->weakListeners.insert(body->weakListeners.begin(), std::move(listener));
        return Token(it);
    }

    template<class T>
    void Dynx<T>::off(Token token) {
        std::visit([&](auto&& it){
            using It = std::decay_t<decltype(it)>;
            if constexpr(std::is_same_v<It, Listeners::iterator>){
                body->listeners.erase(it);
            }else{
                body->weakListeners.erase(it);
            }
        }, token.it);
    }

    template<class T>
    T Dynx<T>::evalExpression() {
        childStack.push_back([&](const std::shared_ptr<void>& otherBody) -> WeakListener  {
            if(otherBody == body)
                return {};
            if(!body->handle)
                body->handle = std::make_shared<std::function<void()>>([*this]() mutable { update(); });
            return body->handle;
        });
        auto value = body->expression();
        assert(!childStack.empty());
        childStack.pop_back();
        return value;
    }

    template<class T>
    void Dynx<T>::update() {
        body->handle = nullptr;
        if(body->expression){
            auto value = evalExpression();
            if(value == body->value)
                return;
            body->value = std::move(value);
        }
        for(auto& listener : body->listeners)
            listener();
        runWeakListeners(body, body->weakListeners);
    }
}

#endif //DYNX_DYNX_CPP