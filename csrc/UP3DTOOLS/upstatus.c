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
  SetPrinterStatus(SS_PAUSED);
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


float steps[4]; // steps per mm for each axis from printer info

static bool up3d_updateData()
{
    TT_tagPrinterInfoHeader pihdr;
    TT_tagPrinterInfoName   piname;
    TT_tagPrinterInfoData   pidata;
    TT_tagPrinterInfoSet    pisets[8];
    if( !UP3D_GetPrinterInfo( &pihdr, &piname, &pidata, pisets ) )
    {
        printf( "UP printer info error\n" );
        UP3D_Close();
        return false;
    }

    steps[0] = pidata.f_steps_mm_x;
    steps[1] = pidata.f_steps_mm_y;
    steps[2] = pidata.f_steps_mm_z;
    steps[3] = pidata.f_steps_mm_x == 160.0 ? 236.0 : 854.0; // fix display for Cetus3D


    printf("PrinterId: %u\n", pihdr.u32_printerid);
    // logDebug("HwVersion: %u\n", pihdr.u32_hw_version);
    printf("RomVersion: %f\n", pihdr.f_rom_version);
    printf("serialNum: %u\n", pihdr.u32_printerserial);
    printf("nozzletype: %u\n", pihdr.u32_unk7);
    printf("printerName: %.63s\n", piname.printer_name);
    // for (int i = 0; i < 8; i++)
    // {
    //     printf("setName%i: %.16s\n", i, pisets[i].set_name);
    //     printf("nozzleDiam: %f\n", pisets[i].nozzle_diameter);
    // }

    UP3D_SetParameter(0x94,999); //set best accuracy for reporting position
    return true;
}

void print_motors_pos()
{
  for (int axis = 1; axis < 5; ++axis)
  {
    int32_t apos = UP3D_GetAxisPosition(axis);
    float pos = (float)apos / steps[axis-1];
    printf("Motor%u: %7.3f == %i\n", axis, pos, apos);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  // if (argc > 2)
  //   print_usage_and_exit(argv[0]);

  if( !UP3D_Open() )
    return -1;

  up3d_updateData();

  if (UP3D_IsPrinterResponsive())
  {

  }

  if (argc >= 2)
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
      if (argc > 4)
      {
        int axis = atoi(argv[2]) - 1;
        float offset = atof(argv[3]);
        float speed = atof(argv[4]);
        MotorJog(axis, offset, speed);
      }
      else
      {
        printf("move i8_axis f_offset f_speed\n");
      }
    }
    else if ( !strcmp(argv[1], "moveto") )
    {
      if (argc > 4)
      {
        int axis = atoi(argv[2]) - 1;
        float coord = atof(argv[3]);
        float speed = atof(argv[4]);
        MotorJogTo(axis, coord, speed);
      }
      else
      {
        printf("moveto i8_axis f_coord f_speed\n");
      }
    }
    else if ( !strcmp(argv[1], "pause") )
    {
      UpPauseProgram();
    }
    else if ( !strcmp(argv[1], "resume") )
    {
      StartResumeProgram();
    }
    else if ( !strcmp(argv[1], "extrude") )
    {
      UpExtrudeMaterial(true);
    }
    else if ( !strcmp(argv[1], "init") )
    {
      InitialPrinter();
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


  print_motors_pos();
  printf("\n%s;%s;%s;%0.2f;%d;%0.2f;%d;%d\n",
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

