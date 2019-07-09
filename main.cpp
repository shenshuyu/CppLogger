#include "Logger.h"
using namespace log;
int main()
{
    int counter = 0;
    LOG_DEBUG("Debug counter : %d \n", counter);
    
    LOG_DEBUG("Test Logger!! \n");

    getchar();
    return 0;
}
