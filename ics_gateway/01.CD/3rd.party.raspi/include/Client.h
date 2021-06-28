#pragma once
#include "Connection.h"
#include "EventBase.h"
#include "Utils.hpp"
#ifndef WIN32
#include <arpa/inet.h>
#endif // WIN32

#include "Poco/Logger.h"
#include "Poco/Util/Application.h"

namespace Net {
    namespace TCP {
        class Client :
            public Connection
        {
        protected:
            std::string host;
            int         port;

        public:
            explicit Client(const std::string &host, int port, int msMaxReconnect);

            explicit Client(const std::string &host, int port, int msMaxReconnect, Poco::Logger* pLogger);
            
            virtual ~Client() {}
            
        public:
            virtual void onClose();
            void doConnect();
            bool checkConnection();
            void onConnectSucess();

            template<typename T>
            inline bool writeToBufferEvent(const T &t)
            {
                return bufferevent_write(bev_.get(), &t, sizeof(T)) ? false : true;
            }

            inline bool writeToBufferEvent(const char *str)
            {
                return bufferevent_write(bev_.get(), str, strlen(str)) ? false : true;
            }
        protected:
            struct sockaddr_in addr_ = { 0 };
            int msMaxReconnect_ = -1;
            int msReconnect_ = 50;
        public:
            struct
            {
                uint32_t closed_ : 1 ;
                uint32_t reconnecting_ : 1;
            }state_ = { 0 };
                        
            Poco::Logger* _pLogger = NULL;
        };
    }
}

