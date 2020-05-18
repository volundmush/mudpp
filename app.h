//
// Created by volund on 5/17/20.
//

#ifndef MUDPP_APP_H
#define MUDPP_APP_H

#include <Poco/Util/Application.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionCallback.h>
#include <Poco/Util/IntValidator.h>
#include <Poco/Util/HelpFormatter.h>

#include <iostream>

class GameApplication : public Poco::Util::Application {
public:
    void initialize(Application &self) override;
    void uninitialize() override;
    void reinitialize(Application &self) override;
    void defineOptions(Poco::Util::OptionSet &options) override;
    void handleOption(const std::string &name, const std::string &value) override;
    int main(const std::vector<std::string> &args) override;
private:
    void handleHelp(const std::string& name, const std::string& value);

    void displayHelp();

    bool _helpRequested = false;
};


#endif //MUDPP_APP_H
