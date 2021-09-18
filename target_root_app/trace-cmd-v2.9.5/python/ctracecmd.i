// tracecmd.i
%module ctracecmd
%include "typemaps.i"
%include "constraints.i"

%nodefaultctor record;
%nodefaultdtor record;

%apply Pointer NONNULL { struct tracecmd_input *handle };
%apply Pointer NONNULL { struct tep_handle *pevent };
%apply Pointer NONNULL { struct tep_format_field * };
%apply unsigned long long *OUTPUT {unsigned long long *}
%apply int *OUTPUT {int *}


%{
#include "trace-cmd.h"
#include "event-parse.h"
#include "event-utils.h"
#include <Python.h>
%}


%typemap(in) PyObject *pyfunc {
	if (!PyCallable_Check($input)) {
		PyErr_SetString(PyExc_TypeError, "Need a callable object!");
		return NULL;
	}
	$1 = $input;
}

%ignore python_callback;

%inline %{
static int python_callback(struct trace_seq *s,
			   struct tep_record *record,
			   struct tep_event *event,
			   void *context);

static int skip_output = 0;

static void py_supress_trace_output(void)
{
	skip_output = 1;
}

void pr_info(const char *fmt, ...)
{
        va_list ap;

	if (skip_output)
		return;
	va_start(ap, fmt);
	__vpr_info(fmt, ap);
	va_end(ap);
}

void warning(const char *fmt, ...)
{
	va_list ap;

	if (skip_output)
		return;

	va_start(ap, fmt);
	__vwarning(fmt, ap);
	va_end(ap);
}

PyObject *convert_pevent(unsigned long pevent)
{
	void *pev = (void *)pevent;
	return SWIG_NewPointerObj(SWIG_as_voidptr(pev), SWIGTYPE_p_tep_handle, 0);
}

void py_pevent_register_event_handler(struct tep_handle *pevent, int id,
				      char *subsys, char *evname,
				      PyObject *pyfunc)
{
	Py_INCREF(pyfunc);
	tep_register_event_handler(pevent, id, subsys, evname,
				   python_callback, pyfunc);
}

static PyObject *py_field_get_stack(struct tep_handle *pevent,
				    struct tep_record *record,
				    struct tep_event *event,
				    int long_size)
{
	PyObject *list;
	struct tep_format_field *field;
	void *data = record->data;
	const char *func = NULL;
	unsigned long addr;

	field = tep_find_any_field(event, "caller");
	if (!field) {
		PyErr_SetString(PyExc_TypeError,
				"Event doesn't have caller field");
		return NULL;
	}

	list = PyList_New(0);

	for (data += field->offset; data < record->data + record->size;
	     data += long_size) {
		addr = tep_read_number(event->tep, data, long_size);

		if ((long_size == 8 && addr == (unsigned long long)-1) ||
		    ((int)addr == -1))
			break;
		func = tep_find_function(event->tep, addr);
		if (PyList_Append(list, PyUnicode_FromString(func))) {
			Py_DECREF(list);
			return NULL;
		}
	}

	return list;
}

#if PY_MAJOR_VERSION >= 3
static PyObject *fromMemory(void *buf, size_t len)
{
		return PyMemoryView_FromMemory(buf, len, PyBUF_READ);
}
#define PY_INT_AS_LONG PyLong_AsLong
#else
static PyObject *fromMemory(void *buf, size_t len)
{
		return PyBuffer_FromMemory(buf, len);
}
#define PY_INT_AS_LONG PyInt_AS_LONG
#endif



static PyObject *py_field_get_data(struct tep_format_field *f, struct tep_record *r)
{
	if (!strncmp(f->type, "__data_loc ", 11)) {
		unsigned long long val;
		int len, offset;

		if (tep_read_number_field(f, r->data, &val)) {
			PyErr_SetString(PyExc_TypeError,
					"Field is not a valid number");
			return NULL;
		}

		/*
		 * The actual length of the dynamic array is stored
		 * in the top half of the field, and the offset
		 * is in the bottom half of the 32 bit field.
		 */
		offset = val & 0xffff;
		len = val >> 16;

		return fromMemory(r->data + offset, len);
	}

	return fromMemory(r->data + f->offset, f->size);
}

static PyObject *py_field_get_str(struct tep_format_field *f, struct tep_record *r)
{
	if (!strncmp(f->type, "__data_loc ", 11)) {
		unsigned long long val;
		int offset;

		if (tep_read_number_field(f, r->data, &val)) {
			PyErr_SetString(PyExc_TypeError,
					"Field is not a valid number");
			return NULL;
		}

		/*
		 * The actual length of the dynamic array is stored
		 * in the top half of the field, and the offset
		 * is in the bottom half of the 32 bit field.
		 */
		offset = val & 0xffff;

		return PyUnicode_FromString((char *)r->data + offset);
	}

	return PyUnicode_FromStringAndSize((char *)r->data + f->offset,
				strnlen((char *)r->data + f->offset, f->size));
}

static PyObject *py_format_get_keys(struct tep_event *ef)
{
	PyObject *list;
	struct tep_format_field *f;

	list = PyList_New(0);

	for (f = ef->format.fields; f; f = f->next) {
		if (PyList_Append(list, PyUnicode_FromString(f->name))) {
			Py_DECREF(list);
			return NULL;
		}
	}

	return list;
}
%}


%wrapper %{
static int python_callback(struct trace_seq *s,
			   struct tep_record *record,
			   struct tep_event *event,
			   void *context)
{
	PyObject *arglist, *result;
	int r = 0;

	record->ref_count++;

	arglist = Py_BuildValue("(OOO)",
		SWIG_NewPointerObj(SWIG_as_voidptr(s),
				   SWIGTYPE_p_trace_seq, 0),
		SWIG_NewPointerObj(SWIG_as_voidptr(record),
				   SWIGTYPE_p_tep_record, 0),
		SWIG_NewPointerObj(SWIG_as_voidptr(event),
				   SWIGTYPE_p_tep_event, 0));

	result = PyEval_CallObject(context, arglist);
	Py_XDECREF(arglist);
	if (result && result != Py_None) {
		if (!PyInt_Check(result)) {
			PyErr_SetString(PyExc_TypeError,
					"callback must return int");
			PyErr_Print();
			Py_XDECREF(result);
			return 0;
		}
		r = PY_INT_AS_LONG(result);
	} else if (result == Py_None)
		r = 0;
	else
		PyErr_Print();

	Py_XDECREF(result);

	return r;
}
%}


%ignore trace_seq_vprintf;
%ignore vpr_info;

/* SWIG can't grok these, define them to nothing */
#define __trace
#define __attribute__(x)
#define __thread

%include "trace-cmd.h"
%include "trace-seq.h"
%include "event-parse.h"
