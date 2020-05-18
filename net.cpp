//
// Created by volund on 5/17/20.
//

#include "net.h"

const char *NetworkSubsystem::name() const {
    return nullptr;
}

void NetworkSubsystem::initialize(Poco::Util::Application &app) {

}

void NetworkSubsystem::uninitialize() {

}

void NetworkSubsystem::reinitialize(Poco::Util::Application &app) {
    Subsystem::reinitialize(app);
}

void NetworkSubsystem::defineOptions(Poco::Util::OptionSet &options) {
    Subsystem::defineOptions(options);
}
