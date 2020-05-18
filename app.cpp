//
// Created by volund on 5/17/20.
//

#include "app.h"

void GameApplication::initialize(Poco::Util::Application &self) {
    Application::initialize(self);
}

void GameApplication::uninitialize() {
    Application::uninitialize();
}

void GameApplication::reinitialize(Poco::Util::Application &self) {
    Application::reinitialize(self);
}

void GameApplication::defineOptions(Poco::Util::OptionSet &options) {
    Application::defineOptions(options);

    options.addOption(
            Poco::Util::Option("help", "h", "display help information")
            .required(false)
            .repeatable(false)
            .callback(Poco::Util::OptionCallback<GameApplication>(this, &GameApplication::handleHelp))
            );

    options.addOption(
            Poco::Util::Option("port", "p", "choose telnet port")
            .required(false)
            .repeatable(false)
            .validator(new Poco::Util::IntValidator(1,65535))
            );


}

void GameApplication::handleOption(const std::string &name, const std::string &value) {
    Application::handleOption(name, value);
}

int GameApplication::main(const std::vector<std::string> &args) {
    return Application::main(args);
}

void GameApplication::handleHelp(const std::string& name, const std::string& value) {
    _helpRequested = true;
    displayHelp();
    stopOptionsProcessing();
}

void GameApplication::displayHelp() {
    Poco::Util::HelpFormatter helpFormatter(options());
    helpFormatter.setCommand(commandName());
    helpFormatter.setUsage("OPTIONS");
    helpFormatter.setHeader(
            "A sample application that demonstrates some of the features "
            "of the Poco::Util::Application class.");
    helpFormatter.format(std::cout);
}