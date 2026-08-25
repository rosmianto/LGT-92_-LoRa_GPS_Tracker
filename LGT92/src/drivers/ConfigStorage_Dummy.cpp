#include "ConfigStorage_Dummy.h"

bool ConfigStorage_Dummy::init() {
    return true;
}

bool ConfigStorage_Dummy::write(const ConfigData &data) {
    return true;
}

bool ConfigStorage_Dummy::read(const ConfigData *data) {
    return true;
}
