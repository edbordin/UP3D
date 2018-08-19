///////////////////////////////////////////////////////
// Author: K.Scheffer
//
// Date: 27.Sep.2016
//
// tool to control and print out current printer status
//
//////////////////////////////////////////////////////

#include <stdlib.h>
//#ifdef _WIN32
//#include <ncurses/curses.h>
//#else
//#include <curses.h>
//#endif
//#include <signal.h>
#include <string.h>
//#include <sys/time.h>

#include <stdbool.h>
#include <stdint.h>

#include <stdio.h>
//#include <math.h>
#include <unistd.h>

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

void print_usage_and_exit(char *name)
{
  printf("Usage: %s [stop | on | off| pause]\n\n", name);
  printf("          stop:  stops current print and reports status\n");
  printf("          on:    switch printer on and reports status\n");
  printf("          off:   switch printer off and reports status\n");
  printf("          no parameter reports current printer status:\n");
  printf("          <machine>;<program>;<system>;<temp>;<layer>;<height>;<percent>;<time>\n");
  printf("          <machine>: machine state \n");
  printf("          <program>: program state \n");
  printf("          <system>:  numeric value machine state \n");
  printf("          <temp>:    current nozzle temperture in C\n");
  printf("          <layer>:   reported printing layer\n");
  printf("          <height>:  reported printing height in mm\n");
  printf("          <percent>: reported completion in percent\n");
  printf("          <time>:    reported time remaining in seconds\n");
  printf("          <pause>:   pause current job\n");
  printf("\n");
  exit(0);
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool UpPauseProgram()
{
  bool isIdle;
  IsSystemIdle(&isIdle);
  if (!PauseProgram())
  {
      printf("PauseProgram: fail\n");
      return false;
  }
  if (!UpWaitUntilIdle(500))
  {
      printf("UpWaitUntilIdle: fail\n");
      return false;
  }
  SetPrinterStatus(3);
  if (!StopAllMove())
  {
      printf("StopAllMove: fail\n");
      return false;
  }
  Sleep(1000);
  if (!MotorJog(Z_axis, 40.0, 10.0))
  // if(!MotorJogTo(2, 15.0, -22.0))
  {
      printf("MotorJog: fail\n");
      return false;
  }
  if (!UpWaitUntilIdle(500))
  {
      printf("UpWaitUntilIdle 2: fail\n");
      return false;
  }
  printf("Pause done\n");
  return true;
}



//////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  if (argc > 2)
    print_usage_and_exit(argv[0]);

  if( !UP3D_Open() )
    return -1;

  if (UP3D_IsPrinterResponsive())
  {

  }

  if (argc == 2)
  {
    if ( !strcmp(argv[1], "stop") )
    {
      UP3D_BLK blk;
      UP3D_ClearProgramBuf();
      UP3D_PROG_BLK_SetParameter(&blk,0x1C,0); UP3D_WriteBlock(&blk); //not printing...
      UP3D_PROG_BLK_Stop(&blk);UP3D_WriteBlock(&blk);
      UP3D_StartResumeProgram();
    }
    else if ( !strcmp(argv[1], "on") )
    {
      UP3D_BLK blk;
      UP3D_ClearProgramBuf();
      UP3D_PROG_BLK_Power(&blk,true);UP3D_WriteBlock(&blk);
      UP3D_PROG_BLK_Stop(&blk);UP3D_WriteBlock(&blk);
      UP3D_StartResumeProgram();
    }
    else if ( !strcmp(argv[1], "off") )
    {
      UP3D_BLK blk;
      UP3D_ClearProgramBuf();
      UP3D_PROG_BLK_Power(&blk,false);UP3D_WriteBlock(&blk);
      UP3D_PROG_BLK_Stop(&blk);UP3D_WriteBlock(&blk);
      UP3D_StartResumeProgram();
    }
    else if ( !strcmp(argv[1], "beep") )
    {
      UP3D_BLK blk;
      UP3D_ClearProgramBuf();
      UP3D_PROG_BLK_Beeper(&blk,true);UP3D_WriteBlock(&blk);
      usleep(1000000);
      UP3D_PROG_BLK_Beeper(&blk,false);UP3D_WriteBlock(&blk);
      UP3D_StartResumeProgram();
    }
    else if ( !strcmp(argv[1], "home") )
    {
      UP3D_BLK blk;
      UP3D_ClearProgramBuf();

      UP3D_PROG_BLK_Home( &blk, UP3DAXIS_Z, settings.z_dir, settings.z_hofs_hi, settings.z_hspeed_hi );
      UP3D_WriteBlock(&blk);
      UP3D_PROG_BLK_Home( &blk, UP3DAXIS_Z, settings.z_dir, settings.z_hofs_lo, settings.z_hspeed_lo );
      UP3D_WriteBlock(&blk);

      UP3D_StartResumeProgram();
    }
    else if ( !strcmp(argv[1], "move") )
    {
      //UP3D_ClearProgramBuf();
      MotorJog(2, 40.0, 10.0);
      // MotorJogTo(2, 15.0, -22.0);

      //UP3D_StartResumeProgram();
    }
    else if ( !strcmp(argv[1], "pause") )
    {
      // PauseProgram();
      // MotorJogTo(2, 15.0, -22.0);
      UpPauseProgram();
    }
    else if ( !strcmp(argv[1], "resume") )
    {
      StartResumeProgram();
    }
    else if ( !strcmp(argv[1], "extrude") )
    {
      // MotorJog(E_axis, 1200.0, 40.0);
      UpExtrudeMaterial(true);
    }
    else
    {
      UP3D_Close();
      print_usage_and_exit(argv[0]);
    }
  }

  int32_t mstat = UP3D_GetMachineState();
  int32_t pstat = UP3D_GetProgramState();
  int32_t sstat = UP3D_GetSystemState();
  float   temp  = UP3D_GetHeaterTemp(1);
  int32_t layer = UP3D_GetLayer();
  float   height = UP3D_GetHeight();
  int32_t percent = UP3D_GetPercent();
  int32_t tr = UP3D_GetTimeRemaining();


  printf("%s;%s;%s;%0.2f;%d;%0.2f;%d;%d\n",
         UP3D_STR_MACHINE_STATE[mstat],
         UP3D_STR_PROGRAM_STATE[pstat],
         UP3D_STR_SYSTEM_STATE[sstat],
         temp,
         layer,
         height,
         percent,
         tr );

  printf("sizeof float %u", sizeof(float));
  UP3D_Close();
  return 0;
}

