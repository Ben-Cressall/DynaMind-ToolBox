/**
 * CityDrain3 is an open source software for modelling and simulating integrated 
 * urban drainage systems.
 * 
 * Copyright (C) 2012 Gregor Burger
 * 
 * This program is free software; you can redistribute it and/or modify it under 
 * the terms of the GNU General Public License as published by the Free Software 
 * Foundation; version 2 of the License.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY 
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with 
 * this program; if not, write to the Free Software Foundation, Inc., 51 Franklin 
 * Street, Fifth Floor, Boston, MA 02110-1301, USA.
 **/

#ifndef PYTHON_DISABLED
#include <Python.h>
#endif
#include "pythonexception.h"

PythonException::PythonException() {
#ifndef PYTHON_DISABLED
	if (!Py_IsInitialized() || !PyErr_Occurred()) {
		return;
	}


	PyObject *ptype, *pvalue, *ptraceback;
	PyObject *pystr, *module_name, *pyth_module, *pyth_func;


	PyErr_Fetch(&ptype, &pvalue, &ptraceback);


//	PyObject* repr = PyObject_Repr(pvalue);
	PyObject* str = PyUnicode_AsEncodedString(pvalue, "utf-8", "~E~");
	const char *bytes = PyBytes_AS_STRING(str);

	printf("REPR: %s\n", bytes);

	Py_XDECREF(pvalue);
	Py_XDECREF(str);


	PyErr_Print();
#endif
}
