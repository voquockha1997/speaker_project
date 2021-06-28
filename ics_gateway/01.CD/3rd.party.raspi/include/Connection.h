#pragma once
#include<event.h>
#include<memory>
#include<stdexcept>
#include <event2/bufferevent.h>

namespace Net {
    namespace TCP {
        class Connection
        {
        public:
            class Exception : public std::exception
            {
            public:
                explicit Exception(const std::string &msg) noexcept: err_msg_(msg)
                {

                }
                char const* what() const throw() { return err_msg_.c_str(); }
            private:
                std::string err_msg_;
            };            
        public:
            void onRead();
            virtual void SetUpBufferEvent(bufferevent *bev);
            virtual void onError() {};
            virtual void onClose() = 0;
            virtual bool decodeFrame() = 0;
            virtual void onConnect(){};
            virtual void onConnectSucess(){}
        protected:
            struct bufferevent* getBufferEvent()
            {
                return bev_.get();
            }
            std::unique_ptr<struct bufferevent, void(*)(struct bufferevent*)> bev_ = { nullptr, bufferevent_free };
        };
    }
}

