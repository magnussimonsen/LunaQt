#include <Python.h>
#include <fcntl.h>

#include "eulerdll.h"

const char *pystart =
"import sys\n\
class CatchOutErr:\n\
    def __init__(self):\n\
        self.value = ''\n\
    def write(self, txt):\n\
        self.value += txt\n\
    def clear(self):\n\
        self.value = ''\n\
catchOutErr = CatchOutErr()\n\
sys.stdout = catchOutErr\n\
sys.stderr = catchOutErr\n\
import eumat\n\
sys.path.append('.')\n\
";

PyObject *pModule=0;

#define MAXLINE 32000
char line[MAXLINE];

typedef void (*f_print) (char *format, ...);
f_print print;
typedef void (*f_stack) (char **newram, char **ramend);
f_stack stack;
typedef int (*f_eval) (header *hd, char **newram);
f_eval eval;
typedef int (*f_testkey) ();
f_testkey testkey;
typedef int (*f_eulerprint) (header *hd, char **newram);
f_eulerprint eulerprint;

char errorline[MAXLINE];
int error=0;

extern "C" __declspec(dllexport) char *py_error ()
{
	if (error) return errorline;
	else return "";
}

static int numargs=5;

int pythontoeuler (PyObject *item, char *name, char **xnewram, char *xramend);
PyObject *eulertopython (header *hd);

PyObject *pyerror;

/* Return the number of arguments of the application command line */
static PyObject* euler_call (PyObject *self, PyObject *args)
{
	char *newram,*ramend;
	stack(&newram,&ramend);
	header *hd=(header *)newram;

	int n=(int)PyTuple_Size(args);
	for (int i=0; i<n; i++)
	{
		header *hd=(header *)newram;
		if (!pythontoeuler(PyTuple_GetItem(args,i),"",&newram,ramend))
		{
			PyErr_SetString(pyerror,"Cannot convert parameter for Euler");
			return PyErr_NewException("euler.call", NULL, NULL);
		}
	}
	if (!eval(hd,&newram))
	{
		PyErr_SetString(pyerror,"Euler returned an error");
		return PyErr_NewException("euler.call", NULL, NULL);
	}

	PyObject *res=eulertopython(hd);
	if (!res)
	{
		PyErr_SetString(pyerror,"Cannot convert Euler result to Python");
		return PyErr_NewException("euler.call", NULL, NULL);
	}
	
	return res;
}

/* Return the number of arguments of the application command line */
static PyObject* euler_print (PyObject *self, PyObject *args)
{
	char *newram,*ramend;
	stack(&newram,&ramend);
	header *hd=(header *)newram;

	int n=(int)PyTuple_Size(args);
	for (int i=0; i<n; i++)
	{
		header *hd=(header *)newram;
		if (!pythontoeuler(PyTuple_GetItem(args,i),"",&newram,ramend))
		{
			PyErr_SetString(pyerror,"Cannot convert parameter for Euler");
			return PyErr_NewException("euler.call", NULL, NULL);
		}
	}
	if (!eulerprint(hd,&newram))
	{
		PyErr_SetString(pyerror,"Euler returned an error");
		return PyErr_NewException("euler.call", NULL, NULL);
	}

	return Py_None;
}

static PyObject * euler_testkey (PyObject *self, PyObject *args)
{
	return PyLong_FromLong(testkey());
}

static PyMethodDef EmbMethods[] = {
    {"call", euler_call, METH_VARARGS,
     "Call an Euler function by name with arguments"},
    {"testkey", euler_testkey, METH_NOARGS,
     "Test for a key press"},
    {"dump", euler_print, METH_VARARGS,
     "Print anything in Euler"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef moduledef = {
	PyModuleDef_HEAD_INIT,
	"eumat",
	"EMT Module",
	-1,
	EmbMethods,
	NULL,
	NULL,
	NULL,
	NULL,
};

extern "C" __declspec(dllexport) int py_init (
	f_print fprint, f_stack fstack, f_eval feval, 
		f_testkey ftestkey, f_eulerprint feulerprint)
{
	print=fprint;
	stack=fstack;
	eval=feval;
	testkey=ftestkey;
	eulerprint=feulerprint;
	error=0;

	Py_Initialize();

	pModule = PyImport_AddModule("__main__"); //create main module
	if (!pModule)
	{
		strcpy(errorline,"Could not create main module in Python.");
		error=1; return 0;
	}

	PyObject* eumat = PyImport_AddModule("eumat"); //create main module
	if (!eumat)
	{
		printf("Could not add the module eumat in Python.");
		return 0;
	}

	PyModule_AddFunctions(eumat, EmbMethods);

	if (PyRun_SimpleString(pystart)==-1)
	{
		strcpy(errorline,"Could not run the initial script.");
		error=1; return 0;
	}

	pyerror = PyErr_NewException("spam.error", NULL, NULL);
	Py_INCREF(pyerror);
	PyModule_AddObject(pModule, "error", pyerror);

	return 1;
}

extern "C" __declspec(dllexport) int py_exit ()
{
	// if (pModule) Py_Finalize();
	pModule=0;

	return 1;
}

char *py_result ()
{
	PyObject *catcher = PyObject_GetAttrString(pModule,"catchOutErr");
	if (!catcher)
	{
		strcpy(errorline,"Could not get the output catcher.");
		error=1; return 0;
	}
	PyObject *output = PyObject_GetAttrString(catcher,"value");
	if (!output)
	{
		strcpy(errorline,"Could not get the output object.");
		Py_DECREF(catcher);
		error=1; return 0;
	}
	else 
	{
		output = PyUnicode_AsEncodedString(output, "utf-8", "strict");
		if (!output)
		{
			strcpy(errorline, "Could not get the decoded string.");
			Py_DECREF(catcher);
			error = 1; return 0;
		}
		char *out=PyBytes_AsString(output);
		if (!out)
		{
			strcpy(errorline,"Could not get the out string.");
			Py_DECREF(catcher); Py_DECREF(output);
			error=1; return 0;
		}
		if (strlen(out)>=MAXLINE)
		{
			strcpy(errorline,"Python output too long.");
			Py_DECREF(catcher); Py_DECREF(output);
			error=1; return 0;
		}
		strcpy(line,out);
		int l=(int)strlen(line);
		if (l>0 && line[l-1]=='\n') line[l-1]=0;
	}

	Py_DECREF(catcher); Py_DECREF(output);
	return line;
}

extern "C" __declspec(dllexport) char *py_run (char *s)
{
	error=0;

	if (!pModule)
	{
		strcpy(errorline,"Python was not initialized.");
		error=1; return 0;
	}

	if (PyRun_SimpleString("catchOutErr.clear()\n")==-1)
	{
		//strcpy(errorline,"The command returned an error.");
		error=1; return 0;
	}
	
	strcpy(line,s);
	strcat(line,"\n");

	error=(PyRun_SimpleString(line)==-1);

	return py_result();
}

extern "C" __declspec(dllexport) char *py_run_file (char *filename)
{

	error=0;

	if (!pModule)
	{
		strcpy(errorline,"Python was not initialized.");
		error=1; return 0;
	}

	if (PyRun_SimpleString("catchOutErr.clear()\n")==-1)
	{
		strcpy(errorline,"The Python command returned an error.");
		error=1; return 0;
	}

	/*

	FILE *f = fopen(filename, "rb");
	if (!f)
	{
		sprintf(errorline, "C could not open %s", filename);
		error = 1; return 0;
	}
	if (PyRun_SimpleFile(f,filename)==-1) 
	{
		PyErr_Print();
		strcpy(errorline,py_result());
		PyErr_Clear();
		PyRun_SimpleString("catchOutErr.clear()\n");
		error=1; 
		fclose(f);
		return 0;
	}
	fclose(f);

	*/

	FILE *f = fopen(filename, "r");
	if (!f)
	{
		sprintf(errorline, "C could not open %s", filename);
		error = 1; return 0;
	}
	size_t n=fread(line, 1, MAXLINE - 8, f);
	if (n>MAXLINE-10)
	{
		sprintf(errorline, "Python file %s too long", filename);
		error = 1; return 0;
	}
	line[n] = 0;
	if (PyRun_SimpleString(line) == -1)
	{
		PyErr_Print();
		strcpy(errorline, py_result());
		PyErr_Clear();
		PyRun_SimpleString("catchOutErr.clear()\n");
		error = 1;
		fclose(f);
		return 0;
	}
	
	return py_result();
}

double getdouble (PyObject *item)
{
	if (PyFloat_Check(item)) return PyFloat_AsDouble(item);
	if (PyLong_Check(item)) return PyLong_AsLong(item);
	sprintf(errorline,"Element could not be converted to double.");
	error=1;
	return 0.0;
}

double *getcomplex (PyObject *item, double *x)
{
	if (PyComplex_Check(item))
	{
		*x++=(double)PyComplex_RealAsDouble(item);
		*x++=(double)PyComplex_ImagAsDouble(item);
		return x;
	}
	else
	{
		sprintf(errorline,"Element could not be converted to complex.");
		error=1;
	}
	return x;
}

char *putstring (PyObject *item, char *p)
{
	if (!PyUnicode_Check(item))
	{
		strcpy(errorline,"String vector contains non-string items");
		error=1; return p;
	}
	char *s=PyBytes_AsString(PyUnicode_AsEncodedString(item, "utf-8", "strict"));
	int l=(int)strlen(s)+1;
	if (p+l>=ramend)
	{
		strcpy(errorline,"Out of stack space in Euler");
		error=1; return p;
	}
	strcpy(p,s);
	p+=l;
	return p;
}

int pythontoeuler (PyObject *item, char *name, char **xnewram, char *xramend)
{
	newram=*xnewram; ramend=xramend;

	if (PyFloat_Check(item))
	{
		new_real(PyFloat_AsDouble(item),"");
	}
	else if (PyBytes_Check(item))
	{
		new_string(PyBytes_AsString(item),"");
	}
	else if (PyLong_Check(item))
	{
		new_real(PyLong_AsDouble(item),"");
	}
	else if (PyUnicode_Check(item))
	{
		new_string(PyBytes_AsString(PyUnicode_AsEncodedString(item, "utf-8", "strict")),"");
	}
	else if (PyComplex_Check(item))
	{
		new_complex((double)PyComplex_RealAsDouble(item),
			(double)PyComplex_ImagAsDouble(item),"");
	}
	else if (PyList_Check(item))
	{
		int n=(int)PyList_GET_SIZE(item);
		if (n==0) new_matrix(1,0,"");
		else
		{
			PyObject *o=PyList_GetItem(item,0);
			if (PyFloat_Check(o) || PyLong_Check(o))
			{
				header *hd=new_matrix(1,n,"");
				double *m=matrixof(hd);
				*m++=getdouble(o);
				if (error)
				{
					sprintf(errorline,"Vector %s contains non-real elements",name);
					error=1; return 0;
				}
				for (int i=1; i<n; i++)
				{
					PyObject *o=PyList_GetItem(item,i);
					*m++=getdouble(o);
					if (error)
					{
						sprintf(errorline,"Vector %s contains non-real elements",name);
						error=1; return 0;
					}
				}
			}
			else if (PyComplex_Check(o))
			{
				header *hd=new_cmatrix(1,n,"");
				double *m=matrixof(hd);
				m=getcomplex(o,m);
				if (error)
				{
					sprintf(errorline,"Vector %s contains non-complex elements",name);
					error=1; return 0;
				}
				for (int i=1; i<n; i++)
				{
					PyObject *o=PyList_GetItem(item,i);
					m=getcomplex(o,m);
					if (error)
					{
						sprintf(errorline,"Vector %s contains non-complex elements",name);
						error=1; return 0;
					}
				}
			}
			else if (PyUnicode_Check(o))
			{
				header *hd=new_smatrix("");
				char *p=stringof(hd);
				p=putstring(o,p);
				if (error) return 0;
				for (int i=1; i<n; i++)
				{
					PyObject *o=PyList_GetItem(item,i);
					p=putstring(o,p);
					if (error) return 0;
				}
				*p++=1;
				newram=p;
				setsizeto(hd,p);
			}
			else if (PyList_Check(o))
			{
				int m=(int)PyList_GET_SIZE(o);
				if (m==0)
				{
					strcpy(errorline,"Matrix row has length 0");
					error=1; return 0;
				}
				if (PyComplex_Check(PyList_GetItem(o,0)))
				{
					header *hd=new_cmatrix(n,m,"");
					double *x=matrixof(hd);
					for (int i=0; i<n; i++)
					{
						PyObject *o=PyList_GetItem(item,i);
						if (m!=PyList_GET_SIZE(o))
						{
							sprintf(errorline,"Matrix row of length %d not accepted",(int)PyList_GET_SIZE(o));
							error=1; return 0;
						}
						for (int j=0; j<m; j++)
						{
							PyObject *u=PyList_GetItem(o,j);
							x=getcomplex(u,x);
							if (error) return 0;
						}
					}
				}
				else
				{
					header *hd=new_matrix(n,m,"");
					double *x=matrixof(hd);
					for (int i=0; i<n; i++)
					{
						PyObject *o=PyList_GetItem(item,i);
						if (m!=PyList_GET_SIZE(o))
						{
							sprintf(errorline,"Matrix row of length %d not accepted",(int)PyList_GET_SIZE(o));
							error=1; return 0;
						}
						for (int j=0; j<m; j++)
						{
							PyObject *u=PyList_GetItem(o,j);
							*x++=getdouble(u);
							if (error) return 0;
						}
					}
				}
			}
			else
			{
				sprintf(errorline,"Could not translate list object %s to Euler",name);
				error=1; return 0;
			}
		}
	}
	else if (item==Py_None)
	{
		new_string("\x01","");
	}
	else
	{
		sprintf(errorline,"Could not translate object %s to Euler",name);
		error=1; return 0;
	}

	*xnewram=newram;

	return 1;
}

extern "C" __declspec(dllexport) int py_get (char *name, char **xnewram, char *xramend)
{
	error=0;

	if (!pModule)
	{
		strcpy(errorline,"Python was not initialized.");
		error=1; return 0;
	}

	PyObject *item = PyObject_GetAttrString(pModule,name);
	if (!item)
	{
		sprintf(errorline,"Could not find object %s",name);
		error=1; return 0;
	}
	Py_DECREF(item);

	return pythontoeuler(item,name,xnewram,xramend);
}

PyObject *eulertopython (header *hd)
{
	if (hd->type==s_real)
	{
		double x=*realof(hd);
		if ((long)x==x) return PyLong_FromLong((long)x);
		return PyFloat_FromDouble(*realof(hd));
	}
	else if (hd->type==s_complex)
	{
		return PyComplex_FromDoubles(*realof(hd),*(realof(hd)+1));
	}
	else if (hd->type==s_string)
	{
		char *s=stringof(hd);
		if (strncmp(s,"py$",3))
			return PyBytes_FromString(stringof(hd));
		else
		{	
			PyObject *f=PyObject_GetAttrString(pModule,s+3);
			if (!f) 
			{
				sprintf(errorline,"%s not found",s);
				error=1;
			}
			return f;
		}
	}
	else if (hd->type==s_smatrix)
	{
		int n=0;
		char *p=stringof(hd);
		while (*p!=1)
		{
			n++;
			p=p+strlen(p)+1;
		}
		PyObject *o=PyList_New(n);
		n=0;
		p=stringof(hd);
		while (*p!=1)
		{
			PyObject *item=PyBytes_FromString(p);
			PyList_SetItem(o,n,item);
			n++;
			p=p+strlen(p)+1;
		}
		return o;
	}
	else if (hd->type==s_matrix && dimsof(hd)->r==1)
	{
		int m=dimsof(hd)->c;
		PyObject *o=PyList_New(m);
		double *x=matrixof(hd);
		for (int i=0; i<m; i++)
		{
			PyObject *item;
			if ((long)(*x)==(*x)) item=PyLong_FromLong((long)(*x++));
			else item=PyFloat_FromDouble(*x++);
			PyList_SetItem(o,i,item);
		}
		return o;
	}
	else if (hd->type==s_matrix && dimsof(hd)->r>1)
	{
		int n=dimsof(hd)->r;
		double *x=matrixof(hd);
		PyObject *o=PyList_New(n);
		for (int i=0; i<n; i++)
		{
			int m=dimsof(hd)->c;
			PyObject *or=PyList_New(m);
			for (int j=0; j<m; j++)
			{
				PyObject *item;
				if ((long)(*x)==(*x)) item=PyLong_FromLong((long)(*x++));
				else item=PyFloat_FromDouble(*x++);
				PyList_SetItem(or,j,item);
			}
			PyList_SetItem(o,i,or);
		}
		return o;
	}
	else if (hd->type==s_cmatrix && dimsof(hd)->r==1)
	{
		int m=dimsof(hd)->c;
		PyObject *o=PyList_New(m);
		double *x=matrixof(hd);
		for (int i=0; i<m; i++)
		{
			PyObject *item=PyComplex_FromDoubles(*x,*(x+1));
			PyList_SetItem(o,i,item);
			x+=2;
		}
		return o;
	}
	else if (hd->type==s_cmatrix && dimsof(hd)->r>1)
	{
		int n=dimsof(hd)->r;
		double *x=matrixof(hd);
		PyObject *o=PyList_New(n);
		for (int i=0; i<n; i++)
		{
			int m=dimsof(hd)->c;
			PyObject *or=PyList_New(m);
			for (int j=0; j<m; j++)
			{
				PyObject *item=PyComplex_FromDoubles(*x,*(x+1));
				PyList_SetItem(or,j,item);
				x+=2;
			}
			PyList_SetItem(o,i,or);
		}
		return o;
	}
	else return 0;
}

extern "C" __declspec(dllexport) int py_set (char *name, header *hd)
{
	error=0;

	if (!pModule)
	{
		strcpy(errorline,"Python was not initialized.");
		error=1; return 0;
	}

	PyObject *o=eulertopython(hd);
	if (!o)
	{
		sprintf(errorline,"Could not set %s in Python",name);
		error=1; return 0;
	}
	PyObject_SetAttrString(pModule,name,o);

	return 1;
}

extern "C" __declspec(dllexport) int py_eval (char *name, header *hd[], 
	int n, char **xnewram, char *xramend)
{
	error=0;

	if (!pModule)
	{
		strcpy(errorline,"Python was not initialized.");
		error=1; return 0;
	}

	PyObject *module=pModule;
	static char line[MAXLINE];
	strcpy(line,name);
	char *pm=strstr(line,"$");
	char *pname=name;
	if (pm)
	{
		*pm=0;
		module=PyImport_AddModule(line);
		pname=pm+1;
	}	

	PyObject *f=PyObject_GetAttrString(module,pname);
	if (!f)
	{
		PyErr_Clear();
		sprintf(errorline,"Python function %s not found",name);
		error=1; return 0;
	}

	PyObject *args=PyTuple_New(n);
	for (int i=0; i<n; i++)
	{
		PyObject *o=eulertopython(hd[i]);
		if (error) { Py_DECREF(f); Py_DECREF(args); return 0; }
		PyTuple_SetItem(args,i,o);
	}

	PyObject *res=PyObject_CallObject(f,args);
	if (!res)
	{
		PyErr_Print();
		strcpy(errorline,py_result());
		PyErr_Clear();
		PyRun_SimpleString("catchOutErr.clear()\n");
		error=1; Py_DECREF(f); Py_DECREF(args); return 0;
	}

	Py_DECREF(f);
	Py_DECREF(args);

	int r=pythontoeuler(res,"",xnewram,xramend);
	Py_DECREF(res);

	return r;
}

