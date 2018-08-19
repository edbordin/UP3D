
#include <stdint.h>
#include "compat.h"
#include "up3d.h"
#include <time.h>
#include "up3dconf.h"

typedef enum Axis {
    X_axis = 0,
    Y_axis = 1,
    Z_axis = 2,
    E_axis = 3
} Axis;


// static char UP3D_STR_MACHINE_STATE[][32] = {"System Error","Jogging","Running Program","Idle","Unknown Status","<UNK>"};

// static char UP3D_STR_PROGRAM_STATE[][32] = {"Program Stop","Program Running","Program Pause","Program Have Errors","<UNK>"};

// static char UP3D_STR_SYSTEM_STATE[][32]  = {"Unknown","System Ready","Running","Paused","Nozzle Error","Motion Error",
//                                             "Print Finished","User Canceled","Nozzle To Hot","Nozzle To Cool","Platform To Hot",
//                                             "DIP ERROR","SWEEP ERROR","SCAN ERROR","SCAN CALIB ERROR","System Power On",
//                                             "SD Card Error","SD Card Write Error","SD Card Read Error","Save To SD Card",
//                                             "JogCmd Error","Invalid Copyright","Over Usage Restriction","<UNK>"};

typedef enum MACHINE_STATE {
    MS_SYSTEM_ERROR = 0,
    MS_JOGGING = 1,
    MS_RUNNING_PROGRAM = 2,
    MS_IDLE = 3,
    MS_UNKNOWN_STATUS
} MACHINE_STATE;

typedef enum PROGRAM_STATE {
    PS_STOP = 0,
    PS_RUNNING = 1,
    PS_PAUSE = 2,
    PS_HAVE_ERRORS
} PROGRAM_STATE;

typedef enum SYSTEM_STATE {
    SS_UNKNOWN = 0,
    SS_SYSTEM_READY = 1,
    SS_RUNNING = 2,
    SS_PAUSED = 3,
    SS_NOZZLE_ERROR,
    SS_MOTION_ERROR,
    SS_PRINT_FINISHED,
    SS_USER_CANCELED,
    SS_NOZZLE_TO_HOT,
    SS_NOZZLE_TO_COOL,
    SS_PLATFORM_TO_HOT,
    SS_DIP_ERROR,
    SS_SWEEP_ERROR,
    SS_SCAN_ERROR,
    SS_SCAN_CALIB_ERROR,
    SS_SYSTEM_POWER_ON,
    SD_CARD_ERROR,
    SS_SD_CARD_WRITE_ERROR,
    SS_SD_CARD_READ_ERROR,
    SS_SAVE_TO_SD_CARD,
    SS_JOGCMD_ERROR,
    SS_INVALID_COPYRIGHT,
    SS_OVER_USAGE_RESTRICTION
} SYSTEM_STATE;

bool GetPrinterResponse(uint8_t *apCmd, uint8_t *apRespBuff, int aCmdLen, int *apRespBuffLen);
bool GetSystemVar(char var, int *response);
bool SetSystemVar(char a1, int value);

bool ClearProgramBuf(void);

/**
 * motor: X=0,Y=1,Z=2
 * offset: positive: up, negative: down
 * speed
 */
bool MotorJog(uint8_t motor, float offset, float speed);

/**
 * motor: X=0,Y=1,Z=2
 * coord: negative
 * speed
 */
bool MotorJogTo(uint8_t motor, float coord, float speed);

bool PauseProgram(void);
bool StartResumeProgram(void);
bool SetZPrecision(int val/* = 999*/);
bool StopAllMove(void);
bool IsSystemIdle(bool *a1);
bool SetPrinterStatus(SYSTEM_STATE status);
bool GetPrinterStatus(SYSTEM_STATE* status);
bool UpWaitUntilIdle(uint32_t timeout);
bool LoadRunRomProg(char prog);