/*
 *
 *  This file is part of MUMPS 5.3.1, released
 *  on Fri Apr 10 13:52:30 UTC 2020
 *
 *
 *  Copyright 1991-2020 CERFACS, CNRS, ENS Lyon, INP Toulouse, Inria,
 *  Mumps Technologies, University of Bordeaux.
 *
 *  This version of MUMPS is provided to you free of charge. It is
 *  released under the CeCILL-C license:
 *  http://www.cecill.info/licences/Licence_CeCILL-C_V1-en.html
 *
 */
#ifndef MUMPS_SCOTCH_INT_H
#define MUMPS_SCOTCH_INT_H
#include "mumps_common.h" /* includes mumps_compat.h and mumps_c_types.h */
#define MUMPS_SCOTCH_INTSIZE \
  F_SYMBOL(scotch_intsize,SCOTCH_INTSIZE)
void MUMPS_CALL
MUMPS_SCOTCH_INTSIZE(MUMPS_INT *scotch_int_size);
#endif
