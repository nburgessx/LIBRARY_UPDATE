#pragma once

#include <memory>
#include <map>
#include <vector>
#include <string>

#include "Environment.h"

using etrading::Environment;

namespace google_test
{

    struct EnvironmentAttacker
    {
        static const std::string CTXT_TOBREAK;
        std::shared_ptr<Environment> wrkContextToCrack_;
        const int numberOfAttempts_;
        EnvironmentAttacker();
        void operator()();
    };

}