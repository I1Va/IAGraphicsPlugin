#include "Graphics.hpp"

extern "C" {
    dr4::DR4Backend* CreateDR4Backend() {
        ia::IAGraphicsBackEnd* backend = new ia::IAGraphicsBackEnd();
        return backend;
    }
}
