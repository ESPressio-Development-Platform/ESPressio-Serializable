#pragma once

#include <functional>

#include "ESPressio_ICommand.hpp"

namespace ESPressio {

    namespace Serializable {

        class ICommandFactory {
            public:
                virtual ICommand* GetCommand(const char* commandName) = 0;
                virtual bool HasCommand(const char* commandName) = 0;
                virtual uint16_t WithCommands(std::function<void(ICommand*)> commandFunction) = 0;
        };

    }

}