#include "misc/dr4_ifc.hpp"
#include "Plugin.hpp"

namespace ia {

    extern "C" {
        dr4::DR4Backend* CreateDR4Backend() {
            ia::IAGraphicsBackEnd* backend = new ia::IAGraphicsBackEnd();
            return backend;
        }
    }

}