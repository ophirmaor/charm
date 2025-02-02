/*
 * Copyright (c) 2019, Mellanox Technologies. All rights reserved.
 * See LICENSE in this directory.
 *
 * Support for the Ncpy Entry Method API
 */
#ifndef UCX_MACHINE_ONESIDED_H
#define UCX_MACHINE_ONESIDED_H

#define UCX_MAX_PACKED_RKEY_SIZE CMK_NOCOPY_DIRECT_BYTES - sizeof(ucp_mem_h)


// Nocopy Direct API
typedef struct UcxRdmaInfo
{
    ucp_mem_h  memh;
    char       packedRkey[UCX_MAX_PACKED_RKEY_SIZE];
} UcxRdmaInfo;

void UcxRmaOp(NcpyOperationInfo *ncpyOpInfo, int op);

void UcxRmaReqCompleted(void *request, ucs_status_t status);


#endif
