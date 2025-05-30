#pragma once

namespace proxy
{

    class EchoProxy
    {
    public:
        explicit EchoProxy(unsigned short port);
        void run();

    private:
        unsigned short port_;
    };

}
