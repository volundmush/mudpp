//
// Created by volund on 5/17/20.
//

#ifndef MUDPP_NET_H
#define MUDPP_NET_H

#include <Poco/Util/Subsystem.h>

class NetworkSubsystem : public Poco::Util::Subsystem {
public:
    const char* name() const override;
    void initialize(Poco::Util::Application &app) override;
    void uninitialize() override;
    void reinitialize(Poco::Util::Application &app) override;
    void defineOptions(Poco::Util::OptionSet &options) override;
};


#endif //MUDPP_NET_H
