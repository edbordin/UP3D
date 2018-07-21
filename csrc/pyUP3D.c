#include <stdio.h>
#include <Python.h>

#include "up3d.h"
#include "compat.h"


#define upl_error(s) { printf("ERROR: %s\n",s); }
// Module method definitions
static PyObject* up3d_open(PyObject *self, PyObject *args) {
    printf("up3d_open");
    if( !UP3D_Open() )
    {
        return PyBool_FromLong(0L);
    }

    if( !UP3D_IsPrinterResponsive() )
    {
        upl_error( "UP printer is not responding\n" );
        UP3D_Close();
        return PyBool_FromLong(0L);
    }
    return PyBool_FromLong(1L);
}

static PyObject* up3d_close(PyObject *self, PyObject *args) {
    printf("up3d_close\n");
    UP3D_Close();
    return PyBool_FromLong(1L);
}

static PyObject* up3d_close(PyObject *self, PyObject *args) {
{
    UP3D_BLK blk;
    UP3D_ClearProgramBuf();
    UP3D_PROG_BLK_Power(&blk,true);UP3D_WriteBlock(&blk);
    UP3D_PROG_BLK_Stop(&blk);UP3D_WriteBlock(&blk);
    UP3D_StartResumeProgram();
}

static PyObject* up3d_close(PyObject *self, PyObject *args) {
{
    UP3D_BLK blk;
    UP3D_ClearProgramBuf();
    UP3D_PROG_BLK_Power(&blk,false);UP3D_WriteBlock(&blk);
    UP3D_PROG_BLK_Stop(&blk);UP3D_WriteBlock(&blk);
    UP3D_StartResumeProgram();
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
    return PyModule_Create(&pyUP3D_definition);
}