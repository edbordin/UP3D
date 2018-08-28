#include <stdio.h>
#include <Python.h>

#include "up3d.h"
#include "up3ddata.h"
#include "compat.h"
#include <time.h>
#include "printLink.h"


#define upl_error(s) { printf("ERROR: %s\n",s); }
#define logDebug(...) printf (__VA_ARGS__)
#define logWarning(...) printf (__VA_ARGS__)
#define logError(...) fprintf (stderr, __VA_ARGS__)
static PyObject *upError;

float steps[4]; // steps per mm for each axis from printer info
TT_tagPrinterInfoHeader pihdr;
TT_tagPrinterInfoName   piname;
TT_tagPrinterInfoData   pidata;
TT_tagPrinterInfoSet    pisets[8];

static bool up3d_updateData()
{
    if( !UP3D_GetPrinterInfo( &pihdr, &piname, &pidata, pisets ) )
    {
        upl_error( "UP printer info error\n" );
        UP3D_Close();
        return false;
    }

    steps[0] = pidata.f_steps_mm_x;
    steps[1] = pidata.f_steps_mm_y;
    steps[2] = pidata.f_steps_mm_z;
    steps[3] = pidata.f_steps_mm_x == 160.0 ? 236.0 : 854.0; // fix display for Cetus3D


    logDebug("PrinterId: %u\n", pihdr.u32_printerid);
    // logDebug("HwVersion: %u\n", pihdr.u32_hw_version);
    logDebug("RomVersion: %f\n", pihdr.f_rom_version);
    logDebug("serialNum: %u\n", pihdr.u32_printerserial);
    logDebug("nozzletype: %u\n", pihdr.u32_unk7);
    logDebug("printerName: %.63s\n", piname.printer_name);
    for (int i = 0; i < 8; i++)
    {
        logDebug("setName%i: %.16s\n", i, pisets[i].set_name);
        logDebug("nozzleDiam: %f\n", pisets[i].nozzle_diameter);
    }

    UP3D_SetParameter(0x94,999); //set best accuracy for reporting position
    return true;
}

// Module method definitions
static PyObject* up3d_open(PyObject *self, PyObject *args) {
    logDebug("up3d_open\n");
    if (UP3DCOMM_IsConnected())
    {
        UP3D_Close();
    }

    if( !UP3D_Open() )
    {
        return PyBool_FromLong(0L);
    }

    if( !UP3D_IsPrinterResponsive() )
    {
        logError( "UP printer is not responding\n" );
        UP3D_Close();
        return PyBool_FromLong(0L);
    }

    up3d_updateData();
    return PyBool_FromLong(1L);
}

static PyObject* up3d_close(PyObject *self, PyObject *args)
{
    logDebug("up3d_close\n");
    UP3D_Close();
    return PyBool_FromLong(1L);
}

static PyObject* up3d_init(PyObject *self, PyObject *args)
{
    logDebug("up3d_init\n");
    long ret = InitialPrinter();
    logDebug("up3d_init: %u", ret);
    return PyBool_FromLong(1L);
}

static PyObject* up3d_isIdle(PyObject *self, PyObject *args)
{
    logDebug("up3d_isIdle\n");
    bool isIdle;
    if (!IsSystemIdle(&isIdle))
    {
        logDebug("up3d_isIdle fail\n");
        return NULL;
    }

    return PyBool_FromLong(isIdle);
}

static PyObject* up3d_jog(PyObject *self, PyObject *args)
{
    int axis;
    float offset, speed;
    if (!PyArg_ParseTuple(args, "iff", &axis, &offset, &speed))
    {
        PyErr_SetString(upError, "wrong argument list, expect: i_axis, f_offset, f_speed");
        return NULL;
    }
    logDebug("up3d_jog %u %f %f\n", axis, offset, speed);

    axis -= 1;
    const bool ret = MotorJog(axis, offset, speed);
    return PyBool_FromLong(ret);
}

static PyObject* up3d_jogTo(PyObject *self, PyObject *args)
{
    int axis;
    float coord, speed;
    if (!PyArg_ParseTuple(args, "iff", &axis, &coord, &speed))
    {
        PyErr_SetString(upError, "wrong argument list, expect: i_axis, f_coord, f_speed");
        return NULL;
    }
    logDebug("up3d_jogTo %u %f %f\n", axis, coord, speed);

    axis -= 1;
    const bool ret = MotorJog(axis, coord, speed);
    return PyBool_FromLong(ret);
}

static PyObject* up3d_powerOn(PyObject *self, PyObject *args)
{
    if (!UP3DCOMM_IsConnected())
    {
        PyErr_SetString(upError, "printer is not connected\n");
        return NULL;
    }
    UP3D_BLK blk;
    UP3D_ClearProgramBuf();
    UP3D_PROG_BLK_Power(&blk,true);UP3D_WriteBlock(&blk);
    UP3D_PROG_BLK_Stop(&blk);UP3D_WriteBlock(&blk);
    UP3D_StartResumeProgram();
    return PyBool_FromLong(1L);
}

static PyObject* up3d_powerOff(PyObject *self, PyObject *args)
{
    if (!UP3DCOMM_IsConnected())
    {
        PyErr_SetString(upError, "printer is not connected\n");
        return NULL;
    }
    UP3D_BLK blk;
    UP3D_ClearProgramBuf();
    UP3D_PROG_BLK_Power(&blk,false);UP3D_WriteBlock(&blk);
    UP3D_PROG_BLK_Stop(&blk);UP3D_WriteBlock(&blk);
    UP3D_StartResumeProgram();
    return PyBool_FromLong(1L);
}

static PyObject* up3d_getParameter(PyObject *self, PyObject *args)
{
    if (!UP3DCOMM_IsConnected())
    {
        PyErr_SetString(upError, "printer is not connected\n");
        return NULL;
    }
    uint32_t parameter = 0x0B;
    if (!PyArg_ParseTuple(args, "l", &parameter))
    {
        PyErr_SetString(upError, "wrong argument list");
        return NULL;
    }
    uint32_t res = UP3D_GetParameter(parameter & 0xFF);
    // return PyLong_AsUnsignedLong(res);
    return PyLong_FromLong(res);
}

static PyObject* up3d_beep(PyObject *self, PyObject *args)
{
    if (!UP3DCOMM_IsConnected())
    {
        PyErr_SetString(upError, "printer is not connected\n");
        return NULL;
    }
    int time;
    if (!PyArg_ParseTuple(args, "i", &time))
    {
        PyErr_SetString(upError, "wrong argument list, expect i_time(ms)");
        return NULL;
    }
    UP3D_BLK blk;
    UP3D_ClearProgramBuf();
    UP3D_PROG_BLK_Beeper(&blk,true);UP3D_WriteBlock(&blk);
    usleep(time * 1000);
    UP3D_PROG_BLK_Beeper(&blk,false);UP3D_WriteBlock(&blk);
    UP3D_StartResumeProgram();

    return PyBool_FromLong(1);
}

static PyObject* up3d_getFwVersion(PyObject *self, PyObject *args)
{
    if (!UP3DCOMM_IsConnected())
    {
        PyErr_SetString(upError, "printer is not connected\n");
        return NULL;
    }
    return PyLong_FromLong(get_fw_version());
}

static PyObject* up3d_getAxisPos(PyObject *self, PyObject *args)
{
    if (!UP3DCOMM_IsConnected())
    {
        PyErr_SetString(upError, "printer is not connected\n");
        return NULL;
    }

    int axis = 0;
    if(!PyArg_ParseTuple(args, "i", &axis))
    {
        PyErr_SetString(upError, "expected one int\n");
        return NULL;
    }

    if ((axis < 0) || (axis > 4))
    {
        PyErr_SetString(upError, "expecing 0 < axis < 5\n");
        return NULL;
    }

    int32_t apos = UP3D_GetAxisPosition(axis);
    float pos = (float)apos / steps[axis-1];
    return PyFloat_FromDouble(pos);
}

// Method definition object for this extension, these argumens mean:
// ml_name: The name of the method
// ml_meth: Function pointer to the method implementation
// ml_flags: Flags indicating special features of this method, such as
//          accepting arguments, accepting keyword arguments, being a
//          class method, or being a static method of a class.
// ml_doc:  Contents of this method's docstring
static PyMethodDef up_methods[] = {
    {
        "open", up3d_open, METH_NOARGS,
        "try to open usb device, 0 if success"
    },
    {
        "close", up3d_close, METH_NOARGS,
        "close usb"
    },
    {
        "get_fw_version", up3d_getFwVersion, METH_NOARGS,
        "return firmware version"
    },
    {
        "get_axis_pos", up3d_getAxisPos, METH_VARARGS,
        "return position of an axis in mm, get_axis_pos(1) -> 100.0"
    },
    {
        "powerOn", up3d_powerOn, METH_NOARGS,
        "power on, turn on the main relay"
    },
    {
        "powerOff", up3d_powerOff, METH_NOARGS,
        "power off, turn off the main relay"
    },
    {
        "getParam", up3d_getParameter, METH_VARARGS,
        "read a Parameter from the printer, see UP3D_PARAMS"
    },
    {
        "beep", up3d_beep, METH_NOARGS,
        "beep for 0.5sec"
    },
    {
        "init", up3d_init, METH_NOARGS,
        "initialize printer"
    },
    {
        "isIdle", up3d_isIdle, METH_NOARGS,
        "return true if printer is idle"
    },
    {
        "jogTo", up3d_jog, METH_VARARGS,
        "jogTo(axis, coord, speed). Move one of the axis to the coordinate"
    },
    {
        "jog", up3d_jog, METH_VARARGS,
        "jog(axis, offset, speed). Move one of the axis to the offset of current pos"
    },
    {NULL, NULL, 0, NULL}
};

// Module definition
// The arguments of this structure tell Python what to call your extension,
// what it's methods are and where to look for it's method definitions
static struct PyModuleDef pyUP3D_definition = {
    PyModuleDef_HEAD_INIT,
    "pyUP3D_com",
    "A Python module to control your TierTime printer",
    -1,
    up_methods
};

// Module initialization
// Python calls this function when importing your extension. It is important
// that this function is named PyInit_[[your_module_name]] exactly, and matches
// the name keyword argument in setup.py's setup() call.
PyMODINIT_FUNC PyInit_pyUP3D_com(void) {
    Py_Initialize();
    printf("v1.7");

    PyObject *module = PyModule_Create(&pyUP3D_definition);
    upError = PyErr_NewException("up3d.error", NULL, NULL);
    Py_INCREF(upError);
    PyModule_AddObject(module, "error", upError);
    return module;
}