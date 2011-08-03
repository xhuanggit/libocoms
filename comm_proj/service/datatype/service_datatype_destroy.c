/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 * Copyright (c) 2004-2006 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2009 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2006 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2006 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2009      Oak Ridge National Labs.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include "ccs_config.h"
#include "service/include/service/constants.h"
#include "opal/datatype/ccs_datatype.h"
#include "opal/datatype/service_datatype_internal.h"

int32_t service_datatype_destroy( service_datatype_t** dt )
{
    service_datatype_t* pData = *dt;

    if( (pData->flags & CCS_DATATYPE_FLAG_PREDEFINED) &&
        (pData->super.obj_reference_count <= 1) )
        return CCS_ERROR;

    OBJ_RELEASE( pData );
    *dt = NULL;
    return CCS_SUCCESS;
}
