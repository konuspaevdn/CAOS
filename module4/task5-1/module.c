#include <math.h>
#include <Python.h>

static PyObject* factor_out(PyObject* self, PyObject* args_tuple) {
    int args_count = PyTuple_Size(args_tuple);
    if (1 != args_count) {
        PyErr_SetString(
                PyExc_TypeError, "Numbers of required and given args do not match");
        return NULL;
    }
    PyObject* arg = PyTuple_GetItem(args_tuple, 0);
    if (!PyLong_Check(arg)) {
        PyErr_SetString(PyExc_TypeError, "The argument must be an integer");
        return NULL;
    }
    int num = PyLong_AsLong(arg);
    if (1 >= num) {
        PyErr_SetString(PyExc_TypeError, "The argument must be greater than 1");
        return NULL;
    }
    PyObject* list = PyList_New(0);
    PyObject* one = PyLong_FromLong(1);
    PyList_Append(list, one);
    Py_DECREF(one);
    int square = (int)floor(sqrt(num));
    int old_num = num;
    for (int div = 2; div <= square; ++div) {
        PyObject* add = PyLong_FromLong(div);
        while (num % div == 0) {
            PyList_Append(list, add);
            num /= div;
        }
        Py_DECREF(add);
        if (1 == num) {
            break;
        }
    }
    if (1 != num && old_num != num) {
        PyObject* add = PyLong_FromLong(num);
        PyList_Append(list, add);
        Py_DECREF(add);
    }
    if (1 == PyList_Size(list)) {
        Py_DECREF(list);
        return PyUnicode_FromString("Prime!");
    } else {
        return list;
    }
}

static PyMethodDef module_methods[] = {
        {
                .ml_name = "factor_out",
                .ml_meth = factor_out,
                .ml_flags = METH_VARARGS,
                .ml_doc =
                "Выполняет разложение целого числа на простые множители и возвращает их в виде списка."
                "В случае, если число уже является простым, возвращает строку Prime!",
        },
        {NULL, NULL, 0, NULL}};

static struct PyModuleDef module_def = {
        .m_base = PyModuleDef_HEAD_INIT,
        .m_name = "primes",
        .m_doc = "Библиотека, реализующая единственный метод factor_out - "
                 "факторизацию числа",
        .m_methods = module_methods,
};

PyMODINIT_FUNC PyInit_primes()
{
    return PyModule_Create(&module_def);
}