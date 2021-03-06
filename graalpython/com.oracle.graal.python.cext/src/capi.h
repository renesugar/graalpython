/*
 * Copyright (c) 2017, 2018, Oracle and/or its affiliates.
 *
 * The Universal Permissive License (UPL), Version 1.0
 *
 * Subject to the condition set forth below, permission is hereby granted to any
 * person obtaining a copy of this software, associated documentation and/or data
 * (collectively the "Software"), free of charge and under any and all copyright
 * rights in the Software, and any and all patent rights owned or freely
 * licensable by each licensor hereunder covering either (i) the unmodified
 * Software as contributed to or provided by such licensor, or (ii) the Larger
 * Works (as defined below), to deal in both
 *
 * (a) the Software, and
 * (b) any piece of software and/or hardware listed in the lrgrwrks.txt file if
 *     one is included with the Software (each a "Larger Work" to which the
 *     Software is contributed by such licensors),
 *
 * without restriction, including without limitation the rights to copy, create
 * derivative works of, display, perform, and distribute the Software and make,
 * use, sell, offer for sale, import, export, have made, and have sold the
 * Software and the Larger Work(s), and to sublicense the foregoing rights on
 * either these or other terms.
 *
 * This license is subject to the following condition:
 *
 * The above copyright notice and either this complete permission notice or at a
 * minimum a reference to the UPL must be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef CAPI_H
#define CAPI_H

#include "Python.h"

/* Private types are defined here because we need to declare the type cast. */
typedef struct {
    PyObject_HEAD
    PyObject *md_dict;
    struct PyModuleDef *md_def;
    void *md_state;
    PyObject *md_weaklist;
    PyObject *md_name;  /* for logging purposes after md_dict is cleared */
} PyModuleObject;

// taken from CPython "Objects/capsule.c"
typedef struct {
    PyObject_HEAD
    void *pointer;
    const char *name;
    void *context;
    PyCapsule_Destructor destructor;
} PyCapsule;

/* Declare Python structs/types for explicit polyglot typecasts. */
/* NOTE: Also add an appropriate case in 'PyTruffle_Explicit_Cast' ! */
POLYGLOT_DECLARE_STRUCT(_object);
POLYGLOT_DECLARE_TYPE(PyModuleObject);
POLYGLOT_DECLARE_TYPE(PyVarObject);
POLYGLOT_DECLARE_STRUCT(_typeobject);
POLYGLOT_DECLARE_TYPE(PyTupleObject);
POLYGLOT_DECLARE_TYPE(PyListObject);
POLYGLOT_DECLARE_TYPE(PyDictObject);
POLYGLOT_DECLARE_TYPE(PyUnicodeObject);
POLYGLOT_DECLARE_TYPE(PyBytesObject);
POLYGLOT_DECLARE_STRUCT(_longobject);
POLYGLOT_DECLARE_TYPE(PyCapsule);
POLYGLOT_DECLARE_TYPE(PyMemoryViewObject);
POLYGLOT_DECLARE_TYPE(PySetObject);


extern void* to_java(PyObject* obj);
extern void* to_java_type(PyTypeObject* cls);
extern PyObject* to_sulong(void *o);
#define as_char_pointer(obj) truffle_invoke(PY_TRUFFLE_CEXT, "to_char_pointer", to_java(obj))
#define as_long(obj) truffle_invoke(PY_TRUFFLE_CEXT, "to_long", obj)
#define as_int(obj) ((int)as_long(obj))
#define as_short(obj) ((short)as_long(obj))
#define as_uchar(obj) ((unsigned char)as_long(obj))
#define as_char(obj) ((char)as_long(obj))
#define as_double(obj) truffle_invoke_d(PY_TRUFFLE_CEXT, "to_double", to_java(obj))
#define as_float(obj) ((float)as_double(obj))

// defined in 'exceptions.c'
void initialize_exceptions();
// defined in 'pyhash.c'
void initialize_hashes();

// prototype of C landing function
PyObject *wrap_direct(PyCFunction fun, ...);
PyObject *wrap_varargs(PyCFunction fun, PyObject *module, PyObject *varargs);
PyObject *wrap_keywords(PyCFunctionWithKeywords fun, PyObject *module, PyObject *varargs, PyObject *kwargs);
PyObject *wrap_noargs(PyCFunction fun, PyObject *module, PyObject *pnone);
PyObject *wrap_fastcall(_PyCFunctionFast fun, PyObject *self, PyObject **args, Py_ssize_t nargs, PyObject *kwnames);
PyObject *wrap_unsupported(void *fun, ...);

#define write_struct_field(object, struct, fieldname, value)            \
    truffle_write(to_java(object),                                      \
                  #fieldname ,                                          \
                  to_java(value));                                      \
    truffle_write(to_java(object),                                      \
                  offsetof(struct, fieldname),                          \
                  to_java(value));


#define TDEBUG __asm__("int $3")
#define get_method_flags_wrapper(flags)                                 \
    (((flags) < 0) ?                                                    \
     truffle_read(PY_TRUFFLE_CEXT, "METH_DIRECT") :                     \
     (((flags) & METH_KEYWORDS) ?                                       \
      truffle_read(PY_TRUFFLE_CEXT, "METH_KEYWORDS") :                  \
      (((flags) & METH_VARARGS) ?                                       \
       truffle_read(PY_TRUFFLE_CEXT, "METH_VARARGS") :                  \
       (((flags) & METH_NOARGS) ?                                       \
        truffle_read(PY_TRUFFLE_CEXT, "METH_NOARGS") :                  \
        (((flags) & METH_O) ?                                           \
         truffle_read(PY_TRUFFLE_CEXT, "METH_O") :                      \
         (((flags) & METH_FASTCALL) ?                                   \
          truffle_read(PY_TRUFFLE_CEXT, "METH_FASTCALL") :              \
          truffle_read(PY_TRUFFLE_CEXT, "METH_UNSUPPORTED")))))))

#define get_method_flags_cwrapper(flags)                                \
    (void*)((((flags) < 0) ?                                                    \
     wrap_direct :                                                      \
     (((flags) & METH_KEYWORDS) ?                                       \
      wrap_keywords :                                                   \
      (((flags) & METH_VARARGS) ?                                       \
       wrap_varargs :                                                   \
       (((flags) & METH_NOARGS) ?                                       \
        wrap_noargs :                                                   \
        (((flags) & METH_O) ?                                           \
         wrap_direct :                                                  \
         (((flags) & METH_FASTCALL) ?                                   \
          wrap_fastcall :                                               \
          wrap_unsupported)))))))


#define PY_TRUFFLE_TYPE(__TYPE_NAME__, __SUPER_TYPE__, __FLAGS__) {\
    PyVarObject_HEAD_INIT((__SUPER_TYPE__), 0)\
    __TYPE_NAME__,                              /* tp_name */\
    0,                                          /* tp_basicsize */\
    0,                                          /* tp_itemsize */\
    0,                                          /* tp_dealloc */\
    0,                                          /* tp_print */\
    0,                                          /* tp_getattr */\
    0,                                          /* tp_setattr */\
    0,                                          /* tp_reserved */\
    0,                                          /* tp_repr */\
    0,                                          /* tp_as_number */\
    0,                                          /* tp_as_sequence */\
    0,                                          /* tp_as_mapping */\
    0,                                          /* tp_hash */\
    0,                                          /* tp_call */\
    0,                                          /* tp_str */\
    0,                                          /* tp_getattro */\
    0,                                          /* tp_setattro */\
    0,                                          /* tp_as_buffer */\
    (__FLAGS__),                                /* tp_flags */\
    0,                                          /* tp_doc */\
    0,                                          /* tp_traverse */\
    0,                                          /* tp_clear */\
    0,                                          /* tp_richcompare */\
    0,                                          /* tp_weaklistoffset */\
    0,                                          /* tp_iter */\
    0,                                          /* tp_iternext */\
    0,                                          /* tp_methods */\
    0,                                          /* tp_members */\
    0,                                          /* tp_getset */\
    0,                                          /* tp_base */\
    0,                                          /* tp_dict */\
    0,                                          /* tp_descr_get */\
    0,                                          /* tp_descr_set */\
    0,                                          /* tp_dictoffset */\
    0,                                          /* tp_init */\
    0,                                          /* tp_alloc */\
    0,                                          /* tp_new */\
    0,                                          /* tp_free */\
    0,                                          /* tp_is_gc */\
}


int PyTruffle_Debug(void *arg);
PyTypeObject* PyObjectHandle_ForJavaType(void* jobj);

extern PyObject* ReadShortMember(PyObject* object, PyObject* offset);
extern PyObject* ReadIntMember(PyObject* object, PyObject* offset);
extern PyObject* ReadLongMember(PyObject* object, PyObject* offset);
extern PyObject* ReadFloatMember(PyObject* object, PyObject* offset);
extern PyObject* ReadDoubleMember(PyObject* object, PyObject* offset);
extern PyObject* ReadStringMember(PyObject* object, PyObject* offset);
extern PyObject* ReadObjectMember(PyObject* object, PyObject* offset);
extern PyObject* ReadCharMember(PyObject* object, PyObject* offset);
extern PyObject* ReadByteMember(PyObject* object, PyObject* offset);
extern PyObject* ReadUByteMember(PyObject* object, PyObject* offset);
extern PyObject* ReadUShortMember(PyObject* object, PyObject* offset);
extern PyObject* ReadUIntMember(PyObject* object, PyObject* offset);
extern PyObject* ReadULongMember(PyObject* object, PyObject* offset);
extern PyObject* ReadBoolMember(PyObject* object, PyObject* offset);
extern PyObject* ReadObjectExMember(PyObject* object, PyObject* offset);
extern PyObject* ReadLongLongMember(PyObject* object, PyObject* offset);
extern PyObject* ReadULongLongMember(PyObject* object, PyObject* offset);
extern PyObject* ReadPySSizeT(PyObject* object, PyObject* offset);

extern PyObject* WriteShortMember(PyObject* object, PyObject* offset, PyObject* value);
extern PyObject* WriteIntMember(PyObject* object, PyObject* offset, PyObject* value);
extern PyObject* WriteLongMember(PyObject* object, PyObject* offset, PyObject* value);
extern PyObject* WriteFloatMember(PyObject* object, PyObject* offset, PyObject* value);
extern PyObject* WriteDoubleMember(PyObject* object, PyObject* offset, PyObject* value);
extern PyObject* WriteStringMember(PyObject* object, PyObject* offset, PyObject* value);
extern PyObject* WriteObjectMember(PyObject* object, PyObject* offset, PyObject* value);
extern PyObject* WriteCharMember(PyObject* object, PyObject* offset, PyObject* value);
extern PyObject* WriteByteMember(PyObject* object, PyObject* offset, PyObject* value);
extern PyObject* WriteUByteMember(PyObject* object, PyObject* offset, PyObject* value);
extern PyObject* WriteUShortMember(PyObject* object, PyObject* offset, PyObject* value);
extern PyObject* WriteUIntMember(PyObject* object, PyObject* offset, PyObject* value);
extern PyObject* WriteULongMember(PyObject* object, PyObject* offset, PyObject* value);
extern PyObject* WriteBoolMember(PyObject* object, PyObject* offset, PyObject* value);
extern PyObject* WriteObjectExMember(PyObject* object, PyObject* offset, PyObject* value);
extern PyObject* WriteLongLongMember(PyObject* object, PyObject* offset, PyObject* value);
extern PyObject* WriteULongLongMember(PyObject* object, PyObject* offset, PyObject* value);
extern PyObject* WritePySSizeT(PyObject* object, PyObject* offset, PyObject* value);

extern PyObject marker_struct;
#define ERROR_MARKER &marker_struct

/* internal functions to avoid unnecessary managed <-> native conversions */

/* UNICODE */

void* PyTruffle_Unicode_FromString(const char* o);

/* DICT */

void* PyTruffle_Tuple_GetItem(void* jtuple, Py_ssize_t position);

#endif
