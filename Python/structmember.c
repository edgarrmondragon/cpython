
/* Map C struct members to Python object attributes */

#include "Python.h"


PyObject *
PyMember_GetOne(const char *obj_addr, PyMemberDef *l)
{
    PyObject *v;
    if (l->flags & Py_RELATIVE_OFFSET) {
        PyErr_SetString(
            PyExc_SystemError,
            "PyMember_GetOne used with Py_RELATIVE_OFFSET");
        return NULL;
    }

    const char* addr = obj_addr + l->offset;
    switch (l->type) {
    case Py_T_BOOL:
        v = PyBool_FromLong(*(char*)addr);
        break;
    case Py_T_BYTE:
        v = PyLong_FromLong(*(char*)addr);
        break;
    case Py_T_UBYTE:
        v = PyLong_FromUnsignedLong(*(unsigned char*)addr);
        break;
    case Py_T_SHORT:
        v = PyLong_FromLong(*(short*)addr);
        break;
    case Py_T_USHORT:
        v = PyLong_FromUnsignedLong(*(unsigned short*)addr);
        break;
    case Py_T_INT:
        v = PyLong_FromLong(*(int*)addr);
        break;
    case Py_T_UINT:
        v = PyLong_FromUnsignedLong(*(unsigned int*)addr);
        break;
    case Py_T_LONG:
        v = PyLong_FromLong(*(long*)addr);
        break;
    case Py_T_ULONG:
        v = PyLong_FromUnsignedLong(*(unsigned long*)addr);
        break;
    case Py_T_PYSSIZET:
        v = PyLong_FromSsize_t(*(Py_ssize_t*)addr);
        break;
    case Py_T_FLOAT:
        v = PyFloat_FromDouble((double)*(float*)addr);
        break;
    case Py_T_DOUBLE:
        v = PyFloat_FromDouble(*(double*)addr);
        break;
    case Py_T_STRING:
        if (*(char**)addr == NULL) {
            v = Py_NewRef(Py_None);
        }
        else
            v = PyUnicode_FromString(*(char**)addr);
        break;
    case Py_T_STRING_INPLACE:
        v = PyUnicode_FromString((char*)addr);
        break;
    case Py_T_CHAR:
        v = PyUnicode_FromStringAndSize((char*)addr, 1);
        break;
    case _Py_T_OBJECT:
        v = *(PyObject **)addr;
        if (v == NULL)
            v = Py_None;
        Py_INCREF(v);
        break;
    case Py_T_OBJECT_EX:
        v = *(PyObject **)addr;
        if (v == NULL) {
            PyObject *obj = (PyObject *)obj_addr;
            PyTypeObject *tp = Py_TYPE(obj);
            PyErr_Format(PyExc_AttributeError,
                         "'%.200s' object has no attribute '%s'",
                         tp->tp_name, l->name);
        }
        Py_XINCREF(v);
        break;
    case Py_T_LONGLONG:
        v = PyLong_FromLongLong(*(long long *)addr);
        break;
    case Py_T_ULONGLONG:
        v = PyLong_FromUnsignedLongLong(*(unsigned long long *)addr);
        break;
    case _Py_T_NONE:
        v = Py_NewRef(Py_None);
        break;
    default:
        PyErr_SetString(PyExc_SystemError, "bad memberdescr type");
        v = NULL;
    }
    return v;
}

#define WARN(msg)                                               \
    do {                                                        \
    if (PyErr_WarnEx(PyExc_RuntimeWarning, msg, 1) < 0)         \
        return -1;                                              \
    } while (0)

int
PyMember_SetOne(char *addr, PyMemberDef *l, PyObject *v)
{
    PyObject *oldv;
    if (l->flags & Py_RELATIVE_OFFSET) {
        PyErr_SetString(
            PyExc_SystemError,
            "PyMember_SetOne used with Py_RELATIVE_OFFSET");
        return -1;
    }

    addr += l->offset;

    if ((l->flags & Py_READONLY))
    {
        PyErr_SetString(PyExc_AttributeError, "readonly attribute");
        return -1;
    }
    if (v == NULL) {
        if (l->type == Py_T_OBJECT_EX) {
            /* Check if the attribute is set. */
            if (*(PyObject **)addr == NULL) {
                PyErr_SetString(PyExc_AttributeError, l->name);
                return -1;
            }
        }
        else if (l->type != _Py_T_OBJECT) {
            PyErr_SetString(PyExc_TypeError,
                            "can't delete numeric/char attribute");
            return -1;
        }
    }
    switch (l->type) {
    case Py_T_BOOL:{
        if (!PyBool_Check(v)) {
            PyErr_SetString(PyExc_TypeError,
                            "attribute value type must be bool");
            return -1;
        }
        if (v == Py_True)
            *(char*)addr = (char) 1;
        else
            *(char*)addr = (char) 0;
        break;
        }
    case Py_T_BYTE:{
        long long_val = PyLong_AsLong(v);
        if ((long_val == -1) && PyErr_Occurred())
            return -1;
        *(char*)addr = (char)long_val;
        /* XXX: For compatibility, only warn about truncations
           for now. */
        if ((long_val > CHAR_MAX) || (long_val < CHAR_MIN))
            WARN("Truncation of value to char");
        break;
        }
    case Py_T_UBYTE:{
        long long_val = PyLong_AsLong(v);
        if ((long_val == -1) && PyErr_Occurred())
            return -1;
        *(unsigned char*)addr = (unsigned char)long_val;
        if ((long_val > UCHAR_MAX) || (long_val < 0))
            WARN("Truncation of value to unsigned char");
        break;
        }
    case Py_T_SHORT:{
        long long_val = PyLong_AsLong(v);
        if ((long_val == -1) && PyErr_Occurred())
            return -1;
        *(short*)addr = (short)long_val;
        if ((long_val > SHRT_MAX) || (long_val < SHRT_MIN))
            WARN("Truncation of value to short");
        break;
        }
    case Py_T_USHORT:{
        long long_val = PyLong_AsLong(v);
        if ((long_val == -1) && PyErr_Occurred())
            return -1;
        *(unsigned short*)addr = (unsigned short)long_val;
        if ((long_val > USHRT_MAX) || (long_val < 0))
            WARN("Truncation of value to unsigned short");
        break;
        }
    case Py_T_INT:{
        long long_val = PyLong_AsLong(v);
        if ((long_val == -1) && PyErr_Occurred())
            return -1;
        *(int *)addr = (int)long_val;
        if ((long_val > INT_MAX) || (long_val < INT_MIN))
            WARN("Truncation of value to int");
        break;
        }
    case Py_T_UINT: {
        /* XXX: For compatibility, accept negative int values
           as well. */
        int overflow;
        long long_val = PyLong_AsLongAndOverflow(v, &overflow);
        if (long_val == -1 && PyErr_Occurred()) {
            return -1;
        }
        if (overflow < 0) {
            PyErr_SetString(PyExc_OverflowError,
                            "Python int too large to convert to C long");
            return -1;
        }
        else if (!overflow) {
            *(unsigned int *)addr = (unsigned int)(unsigned long)long_val;
            if (long_val < 0) {
                WARN("Writing negative value into unsigned field");
            }
            else if ((unsigned long)long_val > UINT_MAX) {
                WARN("Truncation of value to unsigned short");
            }
        }
        else {
            unsigned long ulong_val = PyLong_AsUnsignedLong(v);
            if (ulong_val == (unsigned long)-1 && PyErr_Occurred()) {
                return -1;
            }
            *(unsigned int*)addr = (unsigned int)ulong_val;
            if (ulong_val > UINT_MAX) {
                WARN("Truncation of value to unsigned int");
            }
        }
        break;
    }
    case Py_T_LONG:{
        *(long*)addr = PyLong_AsLong(v);
        if ((*(long*)addr == -1) && PyErr_Occurred())
            return -1;
        break;
        }
    case Py_T_ULONG: {
        /* XXX: For compatibility, accept negative int values
           as well. */
        int overflow;
        long long_val = PyLong_AsLongAndOverflow(v, &overflow);
        if (long_val == -1 && PyErr_Occurred()) {
            return -1;
        }
        if (overflow < 0) {
            PyErr_SetString(PyExc_OverflowError,
                            "Python int too large to convert to C long");
            return -1;
        }
        else if (!overflow) {
            *(unsigned long *)addr = (unsigned long)long_val;
            if (long_val < 0) {
                WARN("Writing negative value into unsigned field");
            }
        }
        else {
            unsigned long ulong_val = PyLong_AsUnsignedLong(v);
            if (ulong_val == (unsigned long)-1 && PyErr_Occurred()) {
                return -1;
            }
            *(unsigned long*)addr = ulong_val;
        }
        break;
    }
    case Py_T_PYSSIZET:{
        *(Py_ssize_t*)addr = PyLong_AsSsize_t(v);
        if ((*(Py_ssize_t*)addr == (Py_ssize_t)-1)
            && PyErr_Occurred())
                        return -1;
        break;
        }
    case Py_T_FLOAT:{
        double double_val = PyFloat_AsDouble(v);
        if ((double_val == -1) && PyErr_Occurred())
            return -1;
        *(float*)addr = (float)double_val;
        break;
        }
    case Py_T_DOUBLE:
        *(double*)addr = PyFloat_AsDouble(v);
        if ((*(double*)addr == -1) && PyErr_Occurred())
            return -1;
        break;
    case _Py_T_OBJECT:
    case Py_T_OBJECT_EX:
        oldv = *(PyObject **)addr;
        *(PyObject **)addr = Py_XNewRef(v);
        Py_XDECREF(oldv);
        break;
    case Py_T_CHAR: {
        const char *string;
        Py_ssize_t len;

        string = PyUnicode_AsUTF8AndSize(v, &len);
        if (string == NULL || len != 1) {
            PyErr_BadArgument();
            return -1;
        }
        *(char*)addr = string[0];
        break;
        }
    case Py_T_STRING:
    case Py_T_STRING_INPLACE:
        PyErr_SetString(PyExc_TypeError, "readonly attribute");
        return -1;
    case Py_T_LONGLONG:{
        long long value;
        *(long long*)addr = value = PyLong_AsLongLong(v);
        if ((value == -1) && PyErr_Occurred())
            return -1;
        break;
        }
    case Py_T_ULONGLONG:{
        unsigned long long value;
        /* ??? PyLong_AsLongLong accepts an int, but PyLong_AsUnsignedLongLong
            doesn't ??? */
        if (PyLong_Check(v))
            *(unsigned long long*)addr = value = PyLong_AsUnsignedLongLong(v);
        else
            *(unsigned long long*)addr = value = PyLong_AsLong(v);
        if ((value == (unsigned long long)-1) && PyErr_Occurred())
            return -1;
        break;
        }
    default:
        PyErr_Format(PyExc_SystemError,
                     "bad memberdescr type for %s", l->name);
        return -1;
    }
    return 0;
}
