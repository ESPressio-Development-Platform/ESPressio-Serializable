#pragma once

#include <functional>
#include <unordered_map>

#include "ESPressio_ICommand.hpp"
#include "ESPressio_ICommandFactory.hpp"

namespace ESPressio {

    namespace Serializable {

        class CommandFactory : public ICommandFactory {
            private:
                // Registered commands are borrowed. The caller retains ownership.
                std::unordered_map<std::string, ICommand*> _commands;
            protected:
                void RegisterCommand(ICommand* command) {
                    if (command == nullptr) { return; }
                    _commands[std::string(command->GetName())] = command;
                }

                void RegisterCommands(std::initializer_list<ICommand*> commands) {
                    for (auto command : commands) {
                        if (command == nullptr) { continue; }
                        _commands[std::string(command->GetName())] = command;
                    }
                }

                void UnregisterCommand(ICommand* command) {
                    if (command == nullptr) { return; }
                    _commands.erase(command->GetName());
                }

                void UnregisterCommand(const char* commandName) {
                    if (commandName == nullptr) { return; }
                    _commands.erase(std::string(commandName));
                }

                void ClearCommands() {
                    _commands.clear();
                }
            public:
                CommandFactory() {

                }

                ~CommandFactory() {
                    ClearCommands();
                }

                ICommand* GetCommand(const char* commandName) override {
                    if (commandName == nullptr) { return nullptr; }
                    auto command = _commands.find(std::string(commandName));
                    return command == _commands.end() ? nullptr : command->second;
                }


                bool HasCommand(const char* commandName) override {
                    return GetCommand(commandName) != nullptr;
                }


                uint16_t WithCommands(std::function<void(ICommand*)> commandFunction) override {
                    uint32_t count = 0;

                    for (auto command : _commands) {
                        count++;
                        commandFunction(command.second);
                    }

                    return count;
                }
        };

    }

}
