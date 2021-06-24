#include "dynx.hpp"

namespace dynx {
    thread_local std::vector<std::function<WeakListener(const std::shared_ptr<void>&)>> DynxBase::childStack;
    thread_local std::vector<std::pair<std::weak_ptr<void>, WeakListeners*>> DynxBase::listenerQueue;

    void DynxBase::runWeakListeners(std::weak_ptr<void> ptr, WeakListeners& listeners) {
        if(!listenerQueue.empty()){
            listenerQueue.emplace_back(std::move(ptr), &listeners);
            return;
        }
        listenerQueue.emplace_back(std::weak_ptr<void>{}, nullptr);
        listenerQueue.emplace_back(std::move(ptr), &listeners);
        runWeakListenerQueue();
    }

    void DynxBase::runWeakListenerQueue(){
        while(listenerQueue.size() > 1){
            auto [weak_body_ptr, listeners] = std::move(listenerQueue.back());
            listenerQueue.pop_back();
            auto body_ptr = weak_body_ptr.lock(); //keeps listeners alive if it works
            if(!body_ptr)
                continue;
            auto it = listeners->begin();
            auto end = listeners->end();
            while(it != end){
                {
                    auto ptr = it->lock();
                    if(!ptr){
                        it = listeners->erase(it);
                        continue;
                    }
                    (*ptr)();
                }
                if(it->expired()){
                    it = listeners->erase(it);
                    continue;
                }
                ++it;
            }
        }
        listenerQueue.pop_back();
    }

    DynxBase::Token::Token(std::variant<Listeners::iterator, WeakListeners::iterator> it) :
        it(it)
    { }
}
