/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Copyright (C) 2012 Evgeniy Andreev (gsomix)
 */

#ifdef SWIGPYTHON

%include "protocols_helper.i"

/* Numeric operators for SGVector */
%define NUMERIC_SGVECTOR(class_name, type_name, format_str, operator_name, operator)

PyObject* class_name ## _inplace ## operator_name ## (PyObject *self, PyObject *o2)
{
	SGVector< type_name >* arg1=(SGVector< type_name >*) 0; // self in c++ repr

	void* argp1=0; // pointer to self
	int res1=0; // result for self's casting
	int res2=0; // result for checking buffer
	int res3=0; // result for getting buffer

	PyObject* resultobj=0;
	Py_buffer view;
	SGVector< type_name > buf; // internal buffer of self

	int vlen=0; // shape of buffer of self
	Py_ssize_t shape[1];
	Py_ssize_t strides[1];

	type_name* lhs;
	char* rhs;

	res1=SWIG_ConvertPtr(self, &argp1, SWIG_TypeQuery("shogun::SGVector<type_name>"), 0 |  0 );
	arg1=reinterpret_cast< SGVector< type_name >* >(argp1);

	res2=PyObject_CheckBuffer(o2);
	if (!res2)
	{
		SWIG_exception_fail(SWIG_ArgError(res2), "this object don't support buffer protocol");
	}

	res3=PyObject_GetBuffer(o2, &view, PyBUF_F_CONTIGUOUS | PyBUF_ND | PyBUF_STRIDES | 0);
	if (res3!=0 || view.buf==NULL)
	{
		SWIG_exception_fail(SWIG_ArgError(res3), "bad buffer");
	}

	// checking that buffer is right
	if (view.ndim!=1)
	{
		SWIG_exception_fail(SWIG_ArgError(view.ndim), "wrong dimension");
	}

	if (view.itemsize!=sizeof(type_name))
	{
		SWIG_exception_fail(SWIG_ArgError(view.itemsize), "wrong type");
	}

	if (view.shape==NULL)
	{
		SWIG_exception_fail(SWIG_ArgError(0), "wrong shape");
	}

	shape[0]=view.shape[0];
	if (shape[0]!=arg1->vlen)
		SWIG_exception_fail(SWIG_ArgError(0), "wrong size");

	strides[0]=view.strides[0];

	if (view.len!=shape[0]*view.itemsize)
		SWIG_exception_fail(SWIG_ArgError(view.len), "bad buffer");

	// calculation
	buf=*arg1;
	vlen=arg1->vlen;

	lhs=buf.vector;
	rhs=(char*) view.buf;

	for (int i=0; i<vlen; i++)
	{
		lhs[i] ## operator ## = (*(type_name*) (rhs + strides[0]*i));
	}

	resultobj=self;
	PyBuffer_Release(&view);

	Py_INCREF(resultobj);
	return resultobj;

fail:
	return NULL;
}

%enddef // NUMERIC_SGVECTOR

/* Python protocols for SGVector */
%define PROTOCOLS_SGVECTOR(class_name, type_name, format_str, typecode)

%wrapper
%{

/* used by PyObject_GetBuffer */
static int class_name ## _getbuffer(PyObject *self, Py_buffer *view, int flags)
{
	SGVector< type_name >* arg1=(SGVector< type_name >*) 0; // self in c++ repr
	void* argp1=0; // pointer to self
	int res1=0; // result for self's casting

	int num_labels=0;
	Py_ssize_t* shape=NULL;
	Py_ssize_t* strides=NULL;
	
	buffer_vector_ ## type_name ## _info* info=NULL;

	static char* format=(char *) format_str; // http://docs.python.org/dev/library/struct.html#module-struct

	res1 = SWIG_ConvertPtr(self, &argp1, SWIG_TypeQuery("shogun::SGVector<type_name>"), 0 |  0 );
	if (!SWIG_IsOK(res1))
	{
		SWIG_exception_fail(SWIG_ArgError(res1),
					"in method '" "getbuffer" "', argument " "1"" of type '" "SGVector<type_name> *""'");
	}

	if ((flags & PyBUF_C_CONTIGUOUS)==PyBUF_C_CONTIGUOUS)
	{
		PyErr_SetString(PyExc_ValueError, "class_name is not C-contiguous");
		goto fail;
	}

	if ((flags & PyBUF_STRIDES)!=PyBUF_STRIDES &&
		(flags & PyBUF_ND)==PyBUF_ND)
	{
		PyErr_SetString(PyExc_ValueError, "class_name is not C-contiguous");
		goto fail;
	}

	arg1=reinterpret_cast< SGVector< type_name >* >(argp1);

	info=new buffer_vector_ ## type_name ## _info;

	info->buf=*arg1;
	num_labels=arg1->vlen;

	view->buf=info->buf.vector;

	shape=new Py_ssize_t[1];
	shape[0]=num_labels;

	strides=new Py_ssize_t[1];
	strides[0]=sizeof( type_name );

	info->shape=shape;
	info->strides=strides;

	view->ndim=1;

	view->format=(char*) format_str;
	view->itemsize=sizeof( type_name );

	view->len=shape[0]*view->itemsize;
	view->shape=shape;
	view->strides=strides;

	view->readonly=0;
	view->suboffsets=NULL;
	view->internal=(void*) info;

	view->obj=(PyObject*) self;
	Py_INCREF(self);

	return 0;

fail:
	view->obj=NULL;
	return -1;
}

/* used by PyBuffer_Release */
static void class_name ## _releasebuffer(PyObject *self, Py_buffer *view)
{
	buffer_vector_ ## type_name ## _info* temp=NULL;
	if (view->obj!=NULL && view->internal!=NULL)
	{
		temp=(buffer_vector_ ## type_name ## _info*) view->internal;
		if (temp->shape!=NULL)
			delete[] temp->shape;

		if (temp->strides!=NULL)
			delete[] temp->strides;

		temp->buf=SGVector< type_name >();
		delete temp;
	}
}

/* used by PySequence_GetItem */
static PyObject* class_name ## _getitem(PyObject *self, Py_ssize_t idx, bool get_scalar=true)
{
	SGVector< type_name >* arg1=(SGVector< type_name >*) 0; // self in c++ repr
	void* argp1=0; // pointer to self
	int res1=0; // result for self's casting

	char* data=0; // internal data of self
	int vlen=0;

	SGVector< type_name > temp;

	Py_ssize_t* shape;
	Py_ssize_t* strides;

	PyObject* ret;
	PyArray_Descr* descr=PyArray_DescrFromType(typecode);

	res1 = SWIG_ConvertPtr(self, &argp1, SWIG_TypeQuery("shogun::SGVector<type_name>"), 0 |  0 );
	if (!SWIG_IsOK(res1))
	{
		SWIG_exception_fail(SWIG_ArgError(res1),
					"in method '" "getitem" "', argument " "1"" of type '" "SGVector<type_name> *""'");
	}

	arg1=reinterpret_cast< SGVector< type_name >* >(argp1);
	
	temp=*arg1;
	vlen=arg1->vlen;

	data=(char*) temp.vector;

	idx=get_idx_in_bounds(idx, vlen);
	if (idx < 0)
	{
		goto fail;
	}

	data+=idx * sizeof( type_name );

	shape=new Py_ssize_t[1];
	shape[0]=1;

	strides=new Py_ssize_t[1];
	strides[0]=sizeof( type_name );

	if (get_scalar)
	{
		ret=(PyObject *) PyArray_Scalar(data, descr, (PyObject *) self);
	}
	else
	{
		ret=(PyObject *) PyArray_NewFromDescr(&PyArray_Type, descr,
						0, shape,
						strides, data,
	 					NPY_FARRAY | NPY_WRITEABLE,
	 					(PyObject *) self);
	}

	if (ret==NULL)
		goto fail;

	Py_INCREF(self);
	return ret;

fail:
	return NULL;
}

/* used by PySequence_SetItem */
static int class_name ## _setitem(PyObject *self, Py_ssize_t idx, PyObject *v)
{
	PyArrayObject* tmp=NULL;
	int ret=0;

	if (v==NULL)
	{
		// TODO error message
		goto fail;
	}

	tmp=(PyArrayObject *) class_name ## _getitem(self, idx, false);
	if(tmp==NULL)
	{
		goto fail;
	}
	ret=PyArray_CopyObject(tmp, v);
	Py_DECREF(tmp);
	return ret;

fail:
	return -1;
}


/* used by PySequence_GetSlice */
static PyObject* class_name ## _getslice(PyObject *self, Py_ssize_t ilow, Py_ssize_t ihigh)
{
	SGVector< type_name >* arg1=(SGVector< type_name >*) 0; // self in c++ repr
	void* argp1=0; // pointer to self
	int res1=0 ; // result for self's casting

	int vlen=0;
	char* data=0; // internal data of self

	SGVector< type_name > temp;

	Py_ssize_t* shape;
	Py_ssize_t* strides;

	PyArrayObject* ret;
	PyArray_Descr* descr=PyArray_DescrFromType(typecode);

	res1=SWIG_ConvertPtr(self, &argp1, SWIG_TypeQuery("shogun::SGVector<type_name>*"), 0 |  0 );
	if (!SWIG_IsOK(res1))
	{
		SWIG_exception_fail(SWIG_ArgError(res1),
					"in method '" "slice" "', argument " "1"" of type '" "SGVector<type_name> *""'");
	}

	arg1=reinterpret_cast< SGVector< type_name >* >(argp1);

	temp=*arg1;
	vlen=arg1->vlen;

	data=(char*) temp.vector;

	get_slice_in_bounds(&ilow, &ihigh, vlen);
	if (ilow < ihigh)
	{
		data+=ilow * sizeof( type_name );
	}

	shape=new Py_ssize_t[1];
	shape[0]=ihigh - ilow;

	strides=new Py_ssize_t[1];
	strides[0]=sizeof( type_name );

	ret=(PyArrayObject *) PyArray_NewFromDescr(&PyArray_Type, descr,
					1, shape,
 					strides, data,
 					NPY_FARRAY | NPY_WRITEABLE,
 					(PyObject *) self);
	if (ret==NULL)
		goto fail;

	Py_INCREF(self);
	return (PyObject *) ret;

fail:
	return NULL;
}

/* used by PySequence_SetSlice */
static int class_name ## _setslice(PyObject *self, Py_ssize_t ilow, Py_ssize_t ihigh, PyObject* v)
{
	PyArrayObject* tmp=NULL;
	int ret=0;

	if (v==NULL)
	{
		// TODO error message
		goto fail;
	}

	tmp=(PyArrayObject *) class_name ## _getslice(self, ilow, ihigh);
	if(tmp==NULL)
	{
		goto fail;
	}
	ret = PyArray_CopyObject(tmp, v);
	Py_DECREF(tmp);
	return ret;

fail:
	return -1;
}

NUMERIC_SGVECTOR(class_name, type_name, format_str, add, +)
NUMERIC_SGVECTOR(class_name, type_name, format_str, sub, -)
NUMERIC_SGVECTOR(class_name, type_name, format_str, mul, *)

static long class_name ## _flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_NEWBUFFER | Py_TPFLAGS_BASETYPE;
%}

%init
%{
SwigPyBuiltin__shogun__SGVectorT_ ## type_name ## _t_type.ht_type.tp_flags = class_name ## _flags;
%}

%feature("python:bf_getbuffer") SGVector< type_name > #class_name "_getbuffer"
%feature("python:bf_releasebuffer") SGVector< type_name > #class_name "_releasebuffer"

%feature("python:nb_inplace_add") SGVector< type_name > #class_name "_inplaceadd"
%feature("python:nb_inplace_subtract") SGVector< type_name > #class_name "_inplacesub"
%feature("python:nb_inplace_multiply") SGVector< type_name > #class_name "_inplacemul"

%feature("python:sq_item") SGVector< type_name > #class_name "_getitem"
%feature("python:sq_ass_item") SGVector< type_name > #class_name "_setitem"
%feature("python:sq_slice") SGVector< type_name > #class_name "_getslice"
%feature("python:sq_ass_slice") SGVector< type_name > #class_name "_setslice"

%enddef /* PROTOCOLS_SGVECTOR */
#endif /* SWIG_PYTHON */
