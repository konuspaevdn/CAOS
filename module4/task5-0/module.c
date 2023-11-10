#include <Python.h>

static PyObject* dot(PyObject* self, PyObject* args_tuple)
{
    int args_count = PyTuple_Size(args_tuple);
    if (args_count != 3) {
        PyErr_SetString(
            PyExc_TypeError, "Numbers of required and given args do not match");
        return NULL;
    }
    if (!PyLong_Check(PyTuple_GetItem(args_tuple, 0))) {
        PyErr_SetString(PyExc_TypeError, "first argument must be int");
        return NULL;
    }
    int size = PyLong_AsLong(PyTuple_GetItem(args_tuple, 0));

    PyObject* left_matrix = PyTuple_GetItem(args_tuple, 1);
    if (!PyList_Check(left_matrix)) {
        PyErr_SetString(PyExc_TypeError, "second argument must be list");
        return NULL;
    }
    PyObject* null = PyFloat_FromDouble(0);
    PyObject* left_rows = PyList_New(0);
    for (int i = 0; i < size; ++i) {
        PyObject* row = PyList_New(0);
        PyList_Append(left_rows, row);
        Py_DECREF(row);
        PyObject* sublist = (i < PyList_Size(left_matrix)) ? PyList_GetItem(left_matrix, i) : PyList_New(0);
        for (int j = 0; j < size; ++j) {
            if (PyList_Size(sublist) <= j) {
                PyList_Append(row, null);
            } else {
                PyObject* elem = PyList_GetItem(sublist, j);
                PyList_Append(row, elem);
                Py_DECREF(elem);
            }
        }
        if (i >= PyList_Size(left_matrix)) {
            Py_DECREF(sublist);
        }
    }

    PyObject* right_matrix = PyTuple_GetItem(args_tuple, 2);
    if (!PyList_Check(right_matrix)) {
        PyErr_SetString(PyExc_TypeError, "third argument must be list");
        return NULL;
    }
    PyObject* right_rows = PyList_New(0);
    for (int i = 0; i < size; ++i) {
        PyObject* row = PyList_New(0);
        PyList_Append(right_rows, row);
        Py_DECREF(row);
        PyObject* sublist = (i < PyList_Size(right_matrix)) ? PyList_GetItem(right_matrix, i) : PyList_New(0);
        for (int j = 0; j < size; ++j) {
            if (PyList_Size(sublist) <= j) {
                PyList_Append(row, null);
            } else {
                PyObject* elem = PyList_GetItem(sublist, j);
                PyList_Append(row, elem);
                Py_DECREF(elem);
            }
        }
        if (i >= PyList_Size(left_matrix)) {
            Py_DECREF(sublist);
        }
    }
    PyObject* result = PyList_New(0);
    for (int i = 0; i < size; ++i) {
        PyObject* result_row = PyList_New(0);
        PyList_Append(result, result_row);
        Py_DECREF(result_row);
        PyObject* left_row = PyList_GetItem(left_rows, i);
        for (int j = 0; j < size; ++j) {
            double elem = 0;
            for (int k = 0; k < size; ++k) {
                PyObject* right_row = PyList_GetItem(right_rows, k);
                elem += PyFloat_AsDouble(PyList_GetItem(left_row, k)) *
                        PyFloat_AsDouble(PyList_GetItem(right_row, j));
            }
            PyObject* result_elem = PyFloat_FromDouble(elem);
            PyList_Append(result_row, result_elem);
            Py_DECREF(result_elem);
        }
    }
    Py_DECREF(null);
    Py_DECREF(left_rows);
    Py_DECREF(right_rows);
    return result;
}

static PyMethodDef module_methods[] = {
        {
                .ml_name = "dot",
                .ml_meth = dot,
                .ml_flags = METH_VARARGS,
                .ml_doc =
                "Выполняет алгебраическое умножение двух квадратных "
                "вещественных матриц "
                "заданного размера, представленных в виде списка списков.\n"
                "В случае, если в качестве аргументов переданы матрицы, в "
                "которых количество "
                "строк или столбцов меньше указанного размера, то оставшаяся "
                "часть матриц "
                "дополняется нулями. Если больше указанного размера - то "
                "лишние строки или "
                "столбцы не используются.",
        },
        {NULL, NULL, 0, NULL}};

static struct PyModuleDef module_def = {
        .m_base = PyModuleDef_HEAD_INIT,
        .m_name = "matrix",
        .m_doc = "Библиотека, реализующая единственный метод dot - "
                 "перемножение двух [квадратных] "
                 "вещественных матриц",
        .m_methods = module_methods,
};

PyMODINIT_FUNC PyInit_matrix()
{
    return PyModule_Create(&module_def);
}