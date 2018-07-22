#include <stdio.h>
#include <Python.h>

#include "up3d.h"
#include "compat.h"
#include <time.h>


#define upl_error(s) { printf("ERROR: %s\n",s); }
#define logDebug(...) printf (__VA_ARGS__)
#define logWarning(...) printf (__VA_ARGS__)
#define logError(...) fprintf (stderr, __VA_ARGS__)
static PyObject *upError;

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
    return PyBool_FromLong(1L);
}

static PyObject* up3d_close(PyObject *self, PyObject *args)
{
    logDebug("up3d_close\n");
    UP3D_Close();
    return PyBool_FromLong(1L);
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
    UP3D_BLK blk;
    UP3D_ClearProgramBuf();
    UP3D_PROG_BLK_Beeper(&blk,true);UP3D_WriteBlock(&blk);
    usleep(500000);
    UP3D_PROG_BLK_Beeper(&blk,false);UP3D_WriteBlock(&blk);
    UP3D_StartResumeProgram();

    return PyBool_FromLong(1);
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
    {NULL, NULL, 0, NULL}
};

// Module definition
// The arguments of this structure tell Python what to call your extension,
// what it's methods are and where to look for it's method definitions
static struct PyModuleDef pyUP3D_definition = {
    PyModuleDef_HEAD_INIT,
    "pyUP3D",
    "A Python module to control your TierTime printer",
    -1,
    up_methods
};

// Module initialization
// Python calls this function when importing your extension. It is important
// that this function is named PyInit_[[your_module_name]] exactly, and matches
// the name keyword argument in setup.py's setup() call.
PyMODINIT_FUNC PyInit_pyUP3D(void) {
    Py_Initialize();
    printf("v1.7");

    PyObject *module = PyModule_Create(&pyUP3D_definition);
    upError = PyErr_NewException("up3d.error", NULL, NULL);
    Py_INCREF(upError);
    PyModule_AddObject(module, "error", upError);
    return module;
}