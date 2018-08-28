#include <stdint.h>
#include "compat.h"
#include "up3d.h"
#include <time.h>
#include "up3dconf.h"
#include "printLink.h"

#ifdef _WINDOWS
#include <windows.h>
#else
#include <unistd.h>
#define Sleep(x) usleep((x)*1000)
#endif

#define PDEBUG printf
#define PWARN printf
#define PERR printf

#define MIN(a,b) (((a)<(b))?(a):(b))


bool GetPrinterResponse(uint8_t *apCmd, uint8_t *apRespBuff, int aCmdLen, int *apRespBuffLen)
{
  uint8_t resp[2048];
  int respBuffLen = 0;
  volatile int written = UP3DCOMM_Write( apCmd, aCmdLen);
  // PDEBUG("GetPrinterResponse:write(%X): write %x; %X\n", *apCmd, aCmdLen, written);
  if (apRespBuffLen)
  {
    respBuffLen = *apRespBuffLen;
  }

  if (aCmdLen == written)
  {
    int read = sizeof(resp);
    read = UP3DCOMM_Read(resp, read);
    if (apRespBuffLen)
    {
      *apRespBuffLen = read;
    }
    if (0 == read)
    {
      PERR("GetPrinteResponse(%X): read fail\n", *apCmd);
    }
    else if (apRespBuff)
    {
      memmove(apRespBuff, &resp, MIN(respBuffLen, read));
    }
    return (read >= 1);
  }
  else
  {
    PERR("GetPrinterResponse(%X): write fail %i != %i\n", *apCmd, aCmdLen, written);
  }
  return false;
}

bool GetSystemVar(char var, int *response)
{
  uint8_t v3[2]; // [rsp+30h] [rbp+8h]
  int v5; // [rsp+40h] [rbp+18h]

  v5 = 4;
  v3[0] = 0x76;
  v3[1] = var;
  return GetPrinterResponse(&v3, (uint8_t *)response, 2, &v5);
}

bool SetSystemVar(char var, int value)
{
  uint8_t cmd[6]; // [rsp+30h] [rbp+8h]
  int* val = (int*)&cmd[2];
  int v5; // [rsp+40h] [rbp+18h]

  v5 = 4;
  cmd[0] = 0x56;
  cmd[1] = var;
  *val = value;
  return GetPrinterResponse(cmd, 0L, sizeof(cmd), &v5);
}

bool ClearProgramBuf(void)
{
  uint16_t apCmd; // [rsp+30h] [rbp+8h]
  int apRespBuffLen; // [rsp+38h] [rbp+10h]

  apRespBuffLen = 2048;
  apCmd = 0x43;
  return GetPrinterResponse(&apCmd, 0, 1, &apRespBuffLen);
}

/**
 * motor: X=0,Y=1,Z=2
 * offset: positive: up, negative: down
 * speed
 */
bool MotorJog(uint8_t motor, float offset, float speed)
{
  uint8_t CMD_JOG[10];
  uint8_t *pCmd = &CMD_JOG[0];
  uint8_t *pMotor = &CMD_JOG[1];
  float *p1 = (float*)&CMD_JOG[2];
  float *p2 = (float*)&CMD_JOG[2+4];
  int read, i;

  if (motor > E_axis)
    return false;
  *pCmd = 0x6A;
  *pMotor = motor;
  *p1 = speed;
  *p2 = offset;

  // printf("motorJog %u %f %f, %f %f\n", motor, offset, speed, *p1, *p2);
  // _print_buffer(CMD_JOG, 10);
  return GetPrinterResponse(CMD_JOG, 0L, sizeof(CMD_JOG), &read);
}


/**
 * motor: X=0,Y=1,Z=2
 * coord: negative
 * speed
 */
bool MotorJogTo(uint8_t motor, float coord, float speed)
{
  uint8_t CMD_JOG[10];
  uint8_t *pCmd = &CMD_JOG[0];
  uint8_t *pMotor = &CMD_JOG[1];
  float *p1 = (float*)&CMD_JOG[2];
  float *p2 = (float*)&CMD_JOG[2+4];
  int read, i;

  PDEBUG("MotorJogTo(%u, %f, %f)\n", motor, coord, speed);
  if (motor > E_axis)
    return false;
  *pCmd = 0x4A;
  *pMotor = motor;
  *p1 = speed;
  *p2 = coord;
  return GetPrinterResponse(CMD_JOG, 0L, sizeof(CMD_JOG), &read);
}

bool PauseProgram(void)
{
  int v2 = 1;
  uint8_t v1 = 0x50;
  return GetPrinterResponse(&v1, 0, 1, &v2);
}

bool StartResumeProgram(void)
{
  uint8_t v1; // [rsp+30h] [rbp+8h]
  int v2; // [rsp+38h] [rbp+10h]

  v2 = 1;
  v1 = 0x58;
  return GetPrinterResponse(&v1, 0, 1, &v2);
}

bool SetZPrecision(int val/* = 999*/)
{
  UP3D_SetParameter( 0x94, val & 0x3e7 );             //SET_Z_PRECISION
}

bool StopAllMove(void)
{
  signed int v0; // ebx
  uint8_t apCmd[2]; // [rsp+30h] [rbp+8h]
  char v3; // [rsp+32h] [rbp+Ah]
  int apRespBuffLen; // [rsp+38h] [rbp+10h]

  v0 = 0;
  PDEBUG("StopAllMove");

  do
  {
    apCmd[0] = 0x30 + v0;
    apCmd[1] = 0x73;
    v3 = 0;
    apRespBuffLen = 1;
    if (!GetPrinterResponse(apCmd, 0, 2, &apRespBuffLen))
    {
      PERR("StopAllMove(%u) fail", v0);
    }
    ++v0;
  }
  while ( v0 < 4 );

  apRespBuffLen = 1;
  apCmd[0] = 0x53;
  return GetPrinterResponse(apCmd, 0, 1, &apRespBuffLen);
}

bool IsSystemIdle(bool *isIdle)
{
  bool result; // rax
  int v3; // [rsp+30h] [rbp+8h]

  result = GetSystemVar(0, &v3);
  *isIdle = (v3 == MS_IDLE);
  // PDEBUG("IsSystemIdle: %u\n", v3);
  return result;
}

bool SetPrinterStatus(SYSTEM_STATE status)
{
  PDEBUG("SetPrinterStatus %x\n", status);
  return SetSystemVar(0x10, status);
}

bool GetPrinterStatus(SYSTEM_STATE* status)
{
  PDEBUG("SetPrinterStatus %x\n", status);
  return GetSystemVar(0x10, status);
}

#define EXECUTE(command) \
if (!(command)) {\
  PERR("%s %s fail\n", __FUNCTION__, #command ); \
  return false;\
}

/**
 * isTempReached = 1 if heater has reached the target temp
 * heater: 1 or 2?
 */
bool GetTempReached(int heater, int* isTempReached)
{
  if ((heater != 1) && (heater != 2))
  {
    return false;
  }
  return GetSystemVar((0x12 + heater - 1), isTempReached);
}

bool UpWaitUntilIdle(uint32_t timeout)
{
  bool isIdle;
  for (; timeout>0; --timeout)
  {
    if (!IsSystemIdle(&isIdle))
    {
      printf("IsSystemIdle: fail\n");
      return false;
    }

    if (isIdle)
    {
      return true;
    }
    Sleep(100);
  }
  return false;
}

bool ExtrudeMaterialFromSS_Pause(bool WithdrawMaterial)
{
  int isTempReached;
  float amount = 1000.0 * ((WithdrawMaterial) ? -1 : 1);
  EXECUTE(GetTempReached(1, isTempReached))
  if (isTempReached)
  {

    MotorJog(E_axis, amount, 50.0);
    EXECUTE(UpWaitUntilIdle(5000));
  }
  else
  {
    PWARN("ExtrudeMaterial: fail, temperature is not reached\n");
    return false;
  }
  return true;
}

bool ExtrudeMaterial(int extruder, int temperature)
{
  int program; // ebx
  int v4; // ecx
  int v5; // ecx
  int v6; // ecx

  if ( extruder == 1 )
  {
    program = 8;
  }
  else
  {
    if ( extruder != 2 )
      return false;
    program = 9;
  }
  v4 = extruder - 1;
  if ( v4 )
  {
    v5 = v4 - 1;
    if ( !v5 )
    {
      SetSystemVar(0x3A, temperature);
      return LoadRunRomProg(program);
    }
    v6 = v5 - 9;
    if ( !v6 || v6 == 1 )
    {
      SetSystemVar(0x3B, temperature);
      return LoadRunRomProg(program);
    }
  }
  else
  {
    SetSystemVar(0x39, temperature);
  }
  return LoadRunRomProg(program);
}

bool UpExtrudeMaterial(bool WithdrawMaterial)
{
  SYSTEM_STATE status;
  EXECUTE(GetPrinterStatus(&status));
  if (status == SS_PAUSED)
  {
    EXECUTE(ExtrudeMaterialFromSS_Pause(WithdrawMaterial));
    return true;
  }
  else if (status == SS_SYSTEM_READY)
  {
    EXECUTE(ExtrudeMaterial(1, 200)); // TODO: check temperature
    return true;
  }
  else
  {
    PWARN("");
    return false;
  }
}

bool InitialPrinter(void)
{
  SetSystemVar(16, 0);
  return LoadRunRomProg(0);
}

bool LoadRomProg(char prog, uint32_t* ret)
{
  char cmd[2]; // [rsp+30h] [rbp+8h]
  int retLen; // [rsp+40h] [rbp+18h]

  retLen = sizeof(*ret);
  cmd[0] = 0x4C;
  cmd[1] = prog + 48;
  return GetPrinterResponse(cmd, ret, 2, &retLen);
}

bool UseSDProgramBuf(int prog, char enableWrite)
{
  uint8_t apCmd[3]; // [rsp+30h] [rbp+8h]
  int apRespBuffLen; // [rsp+40h] [rbp+18h]

  if ( prog >= 0 )
  {
    apCmd[0] = 0x6C;
    apCmd[1] = prog;
    apCmd[2] = enableWrite;
    apRespBuffLen = 4;
    return GetPrinterResponse(apCmd, 0, 3, &apRespBuffLen);
  }
  else
  {
    SetSystemVar(55, 0);
    SetSystemVar(56, 0);
    return true;
  }
}

bool LoadRunRomProg(char prog)
{
  uint8_t apCmd[3]; // [rsp+38h] [rbp+10h]
  char v4; // [rsp+3Ah] [rbp+12h]
  int apRespBuffLen; // [rsp+40h] [rbp+18h]
  int v6; // [rsp+48h] [rbp+20h]

  EXECUTE(ClearProgramBuf());// ClearProgramBuffer
  // if ( dword_7FFB5FAE718C < 0x2710 )
  {
    GetSystemVar(0x68, &v6);
    EXECUTE(UseSDProgramBuf(9, 0));
    SetSystemVar(0x68, v6);
    SetSystemVar(0x38, 0);
    Sleep(10);
    EXECUTE(ClearProgramBuf());
  }
  int loadRomRes;
  EXECUTE(LoadRomProg(prog, &loadRomRes));
  EXECUTE(StartResumeProgram());
  return true;
}