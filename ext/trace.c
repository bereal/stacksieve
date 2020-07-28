#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include <stdlib.h>
#include <stdbool.h>
#include <frameobject.h>


static Py_tss_t trace_ctx_tss;


typedef struct trace_ctx {
    int fd;
    bool seen_call;
} trace_ctx;

static trace_ctx* get_ctx() {
    return (trace_ctx*)PyThread_tss_get(&trace_ctx_tss);
}

static trace_ctx* init_ctx(int fd) {
    trace_ctx *ctx = calloc(sizeof(trace_ctx), 1);
    if (ctx) {
        ctx->fd = fd;
        PyThread_tss_set(&trace_ctx_tss, ctx);
    }

    return ctx;
}

static void del_ctx() {
    trace_ctx *ctx = get_ctx();
    if (ctx) {
        free(ctx);
        PyThread_tss_set(&trace_ctx_tss, NULL);
    }
}


int trace(PyObject *obj, PyFrameObject *frame, int what, PyObject *arg) {
    trace_ctx *ctx = get_ctx();
    if (!ctx) {
        return 0;
    }

    switch (what) {
    case PyTrace_CALL:
        ;
        PyCodeObject *code = frame->f_code;
        PyObject *name_obj = PyUnicode_FromFormat(
            "%U:%d:%U", code->co_filename, code->co_firstlineno, code->co_name
        );
        const char *name = PyUnicode_AsUTF8(name_obj);
        dprintf(ctx->fd, ">%s\n", name);
        ctx->seen_call = true;
        Py_DecRef(name_obj);

        break;
    case PyTrace_RETURN:
        if (ctx->seen_call) {
            dprintf(ctx->fd, "<\n");
        }
        break;
    default:
        return 0;
    }

    return 0;
}


static PyObject* trace_thread(PyObject *self, PyObject *args) {
    int fd;
    PyArg_ParseTuple(args, "i", &fd);
    init_ctx(fd);

    PyEval_SetProfile(trace, NULL);
    Py_RETURN_NONE;
}


static PyObject* cleanup_thread(PyObject *self, PyObject *args) {
    del_ctx();
    PyEval_SetProfile(NULL, NULL);

    Py_RETURN_NONE;
}


static struct PyMethodDef methods[] = {
    {"trace_thread", trace_thread, METH_VARARGS},
    {"cleanup_thread", cleanup_thread, METH_NOARGS},
};


static struct PyModuleDef module = {
    PyModuleDef_HEAD_INIT,
    .m_name="stacksieve_c",
    .m_size=-1,
    .m_methods=methods,
};


PyMODINIT_FUNC PyInit_stacksieve_c(void) {
    return PyModule_Create(&module);
}
