/***

PyABI_URKLE; a PyAPI module implementing https://github.com/chjj/liburkel 

***/

/***

//
//  MIT License
//
//  Copyright(c) 2020 - 2020, Scott McCallum (https github.com scott91e1)
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this softwareand associated documentation files(the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions :
//
//  The above copyright noticeand this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.
//

***/

/***

instruct the compiler to actually complile the source

***/

#define PyABI_INCLUDE 1

#include <string>
#include <sstream>
#include <iostream>

/*

by default we are going to have a small number of workers

its also possible to match the number of cores on the host with:

auto PyABI_threads = std::thread::hardware_concurrency();

*/

auto PyABI_threads = 4;

#include "PyABI_URKLE.hpp"

#include "PyABI_singleton.hpp"

#include "PyABI_body.hpp"


/*
 * Helpers
 */

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

 /*
  * Base16
  */

static const char* urkel_base16_charset = "0123456789abcdef";

static const int8_t urkel_base16_table[256] = {
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
   0,  1,  2,  3,  4,  5,  6,  7,
   8,  9, -1, -1, -1, -1, -1, -1,
  -1, 10, 11, 12, 13, 14, 15, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, 10, 11, 12, 13, 14, 15, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1
};

static void
urkel_base16_encode(char* dst, const uint8_t* src, size_t srclen) {
  size_t i, j;

  for (i = 0, j = 0; i < srclen; i++) {
    dst[j++] = urkel_base16_charset[src[i] >> 4];
    dst[j++] = urkel_base16_charset[src[i] & 15];
  }

  dst[j] = '\0';
}

static int
urkel_base16_decode(uint8_t* dst, size_t* dstlen, const char* src) {
  size_t srclen = strlen(src);
  uint8_t z = 0;
  size_t i, j;

  if (srclen & 1)
    return 0;

  if (srclen / 2 > * dstlen)
    return 0;

  for (i = 0, j = 0; i < srclen / 2; i++) {
    uint8_t hi = urkel_base16_table[(uint8_t)src[j++]];
    uint8_t lo = urkel_base16_table[(uint8_t)src[j++]];

    z |= hi | lo;

    dst[i] = (hi << 4) | lo;
  }

  if (z & 0x80)
    return 0;

  *dstlen = i;

  return 1;
}

static int
urkel_base16_decode32(uint8_t* dst, const char* src) {
  size_t dstlen = 32;

  if (!urkel_base16_decode(dst, &dstlen, src))
    return 0;

  return dstlen == 32;
}

static void
urkel_base16_print(const uint8_t* src, size_t srclen) {
  char tmp[1024 + 1];
  char* dst = tmp;

  if (srclen > 512) {
    dst = (char*)malloc(srclen * 2 + 1);

    if (dst == NULL) {
      abort();
      return;
    }
  }

  urkel_base16_encode(dst, src, srclen);

  printf("%s\n", dst);

  if (dst != tmp)
    free(dst);
}



// Module method definitions
static PyObject* hello_world(PyObject* module, PyObject* args, PyObject* kwargs) {

  PY_DEFAULT_ARGUMENT_INIT(encoding, PyUnicode_FromString("utf-8"), NULL);
  PY_DEFAULT_ARGUMENT_INIT(the_id, PyLong_FromLong(0L), NULL);
  PY_DEFAULT_ARGUMENT_INIT(must_log, PyBool_FromLong(1L), NULL);

  static const char* kwlist[] = { "encoding", "the_id", "must_log", NULL };
  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|Oip", const_cast<char**>(kwlist), &encoding, &the_id, &must_log)) {
    return NULL;
  }

  PY_DEFAULT_ARGUMENT_SET(encoding);
  PY_DEFAULT_ARGUMENT_SET(the_id);
  PY_DEFAULT_ARGUMENT_SET(must_log);

  PyObject* defaultargs = PyList_New(3);
  PyList_SetItem(defaultargs, 0, encoding);
  PyList_SetItem(defaultargs, 1, the_id);
  PyList_SetItem(defaultargs, 2, must_log);
  auto dispatch_success = false;
  try {
    hello_world__(args, kwargs, defaultargs);
    //auto callback = std::bind(&Singleton::hello, singleton, std::placeholders::_1);
    //singleton.PyABI_dispatch(module, args, kwargs, defaultargs, "hello_world", callback);
    dispatch_success = true;
  }
  catch (...) {

  };
  Py_DECREF(defaultargs);

  Py_DECREF(encoding);
  Py_DECREF(the_id);
  Py_DECREF(must_log);

  Py_INCREF(Py_None);
  return Py_None;
}


static PyObject* hash(PyObject* module, PyObject* args, PyObject* kwargs) {
  const char* name;
  if (!PyArg_ParseTuple(args, "s", &name)) {
    return NULL;
  }

  PyObject* defaultargs = PyList_New(0);
  auto dispatch_success = false;
  try {
    //auto callback = std::bind(&Singleton::hello, singleton, std::placeholders::_1);
    //singleton.PyABI_dispatch(module, args, kwargs, defaultargs, "hello", callback);
    dispatch_success = true;
  }
  catch (...) {

  };
  Py_DECREF(defaultargs);

  printf("Hello, %s!\n", name);

  Py_INCREF(Py_None);
  return Py_None;
}

/***


***/

static PyMethodDef urkle_methods[] = {
    { "create",    (PyCFunction)hello_world, METH_VARARGS | METH_KEYWORDS, "create a new database" },
    { "destroy",   (PyCFunction)hello_world, METH_VARARGS | METH_KEYWORDS, "destroy database" },
    { "info",      (PyCFunction)hello_world, METH_VARARGS | METH_KEYWORDS, "print database information" },
    { "root",      (PyCFunction)hello_world, METH_VARARGS | METH_KEYWORDS, "print root hash" },
    { "get",       (PyCFunction)hello_world, METH_VARARGS | METH_KEYWORDS, "retrieve value <key>" },
    { "insert",    (PyCFunction)hello_world, METH_VARARGS | METH_KEYWORDS, "insert value <key> <value>" },
    { "remove",    (PyCFunction)hello_world, METH_VARARGS | METH_KEYWORDS, "remove value <key>" },
    { "list",      (PyCFunction)hello_world, METH_VARARGS | METH_KEYWORDS, "list all keys <key>" },
    { "prove",     (PyCFunction)hello_world, METH_VARARGS | METH_KEYWORDS, "create proof <key> <proof>" },
    { "verify",    (PyCFunction)hello_world, METH_VARARGS | METH_KEYWORDS, "verify proof **kwarg=root" },
    { "hash",      (PyCFunction)hash,        METH_VARARGS | METH_KEYWORDS, "hash key with BLAKE2b-256" },
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef abi_definition = {
    PyModuleDef_HEAD_INIT,
    "PyABI_URKLE_pyd",
    "PyABI C++ Implimiation of https://github.com/chjj/liburkel",
    -1,
    urkle_methods
};

PyMODINIT_FUNC PyInit_PyABI_URKLE_pyd(void) {
  Py_Initialize();
  return PyModule_Create(&abi_definition);
}

int main(int argc, char** argv) {


  return EXIT_SUCCESS;

}

#include "PyABI_footer.hpp"
