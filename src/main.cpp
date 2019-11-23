#include "ManagerModule.h"
#include "Logger.h"
#include "IConfiguration.h"
#include <sys/time.h>
#include <sys/resource.h>

constexpr rlim_t CORE_SIZE = (rlim_t)(2 * 1024 * 1024 * 1024L);

void enableCoreDump()
{
    struct rlimit rli;
    if (getrlimit(RLIMIT_CORE, &rli) == -1) {
        LOG_WARN << "getrlimit failed";
    }
    else {
        rli.rlim_cur = CORE_SIZE;
        if (setrlimit(RLIMIT_CORE, &rli) == -1) {
            LOG_WARN << "setrlimit failed";
        }
    }
}

int main()
{
    enableCoreDump();

    common::InitLogger();
    
    if (IConfiguration::Get().Init() < 0) {
        return -1;
    }

    ManagerModule mgrModule;
    mgrModule.Start();
    mgrModule.Run();
    mgrModule.Stop();
    
    IConfiguration::Get().UnInit();
    common::UnInitLogger();

    return 0;
}
