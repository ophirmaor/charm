/* -*- Mode: C; c-basic-offset:4 ; -*- */
/* 
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpioimpl.h"
#include "adio_extern.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_Register_datarep = PMPI_Register_datarep
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Register_datarep MPI_Register_datarep
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Register_datarep as PMPI_Register_datarep
/* end of weak pragmas */
#endif

/* Include mapping from MPI->PMPI */
#define MPIO_BUILD_PROFILING
#include "mpioprof.h"
#endif

/*@
  MPI_Register_datarep - Register functions for user-defined data 
                         representations

  Input Parameters:
+ name - data representation name (string)
. read_conv_fn - function invoked to convert from file representation to
                 native representation (function)
. write_conv_fn - function invoked to convert from native representation to
                  file representation (function)
. extent_fn - function invoked to get the exted of a datatype as represented
                  in the file (function)
- extra_state - pointer to extra state that is passed to each of the
                three functions

 Notes:
 This function allows the user to provide routines to convert data from
 an external representation, used within a file, and the native representation,
 used within the CPU.  There is one predefined data representation, 
 'external32'.  Please consult the MPI-2 standard for details on this
 function.

.N fortran
  
  @*/
int MPI_Register_datarep(char *name,
			 MPI_Datarep_conversion_function *read_conv_fn,
			 MPI_Datarep_conversion_function *write_conv_fn,
			 MPI_Datarep_extent_function *extent_fn,
			 void *state)
{
    int error_code;
    ADIOI_Datarep *datarep;
    static char myname[] = "MPI_REGISTER_DATAREP";

    MPIU_THREAD_CS_ENTER(ALLFUNC,);

    /* --BEGIN ERROR HANDLING-- */
    /* check datarep name (use strlen instead of strnlen because
       strnlen is not portable) */
    if (name == NULL ||
	strlen(name) < 1 || 
	strlen(name) > MPI_MAX_DATAREP_STRING)
    {
	error_code = MPIO_Err_create_code(MPI_SUCCESS,
					  MPIR_ERR_RECOVERABLE,
					  myname, __LINE__,
					  MPI_ERR_ARG,
					  "**datarepname", 0);
	error_code = MPIO_Err_return_file(MPI_FILE_NULL, error_code);
	goto fn_exit;
    }
    /* --END ERROR HANDLING-- */

    MPIR_MPIOInit(&error_code);
    if (error_code != MPI_SUCCESS) goto fn_exit;

    /* --BEGIN ERROR HANDLING-- */
    /* check datarep isn't already registered */
    for (datarep = CtvAccess(ADIOI_Datarep_head); datarep; datarep = datarep->next) {
	if (!strncmp(name, datarep->name, MPI_MAX_DATAREP_STRING)) {
	    error_code = MPIO_Err_create_code(MPI_SUCCESS,
					      MPIR_ERR_RECOVERABLE,
					      myname, __LINE__,
					      MPI_ERR_DUP_DATAREP,
					      "**datarepused",
					      "**datarepused %s",
					      name);
	    error_code = MPIO_Err_return_file(MPI_FILE_NULL, error_code);
	    goto fn_exit;
	}
    }

    /* check extent function pointer */
    if (extent_fn == NULL)
    {
	error_code = MPIO_Err_create_code(MPI_SUCCESS,
					  MPIR_ERR_RECOVERABLE,
					  myname, __LINE__,
					  MPI_ERR_ARG,
					  "**datarepextent", 0);
	error_code = MPIO_Err_return_file(MPI_FILE_NULL, error_code);
	goto fn_exit;
    }
    /* --END ERROR HANDLING-- */

    datarep = ADIOI_Malloc(sizeof(ADIOI_Datarep));
/* need to ifdef MPICH2 because if it is MPICH2 in memory tracing mode, it will complain 
   about the use of strdup instead of MPIU_Strdup. (mpiimpl.h is being included in mpioimpl.h 
   ifdef MPICH2 */
#ifdef MPICH2
    datarep->name = MPIU_Strdup(name);
#else
    datarep->name = ADIOI_Strdup(name);
#endif
    datarep->state         = state;
    datarep->read_conv_fn  = read_conv_fn;
    datarep->write_conv_fn = write_conv_fn;
    datarep->extent_fn     = extent_fn;
    datarep->next          =  CtvAccess(ADIOI_Datarep_head);

     CtvAccess(ADIOI_Datarep_head) = datarep;

    error_code = MPI_SUCCESS;

fn_exit:
    MPIU_THREAD_CS_EXIT(ALLFUNC,);

    return error_code;
}
