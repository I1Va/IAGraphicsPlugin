#include "Plugin.hpp"

namespace ia {

    extern "C" {
        cum::Plugin *CreatePlugin() {
            ia::IAGraphicsBackEnd *backend = new ia::IAGraphicsBackEnd();
            return backend;
        }
    }

}