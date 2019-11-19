#include "ManagerModule.h"
#include "Logger.h"
#include "IConfiguration.h"

int main()
{
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
