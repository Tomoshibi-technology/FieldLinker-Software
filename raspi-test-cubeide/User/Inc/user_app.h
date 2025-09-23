#ifndef _USER_APP_H_
#define _USER_APP_H_

#include "telemetry.h"

#ifdef __cplusplus
extern "C" {
#endif

void UserApp_Init(void);
void UserApp_Run(void);

// Telemetry data access functions
TelemetryData_t UserApp_GetLatestTelemetryData(void);
uint32_t UserApp_GetFrameCount(void);

#ifdef __cplusplus
}
#endif

#endif /* __USER_APP_H */
