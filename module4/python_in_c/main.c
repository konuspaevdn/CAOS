#include <Python.h>


int main() {
    Py_Initialize();
    PyObject *globals = PyDict_New();
    PyObject *locals = PyDict_New();
    PyObject *a_key = PyUnicode_FromString("a");
    PyObject *a_value = PyLong_FromLongLong(10);
    PyDict_SetItem(locals, a_key, a_value);
    Py_DECREF(a_key);
    Py_DECREF(a_value);

    PyObject *eval_result = PyRun_String("(1+2) * a", Py_eval_input, globals, locals);
    Py_DECREF(globals);
    Py_DECREF(locals);
    PyObject_Print(eval_result, stdout, 0);
    Py_DECREF(eval_result);
    Py_Finalize();
}
