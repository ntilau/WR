C
C  This file is part of MUMPS 5.3.1, released
C  on Fri Apr 10 13:52:30 UTC 2020
C
C
C  Copyright 1991-2020 CERFACS, CNRS, ENS Lyon, INP Toulouse, Inria,
C  Mumps Technologies, University of Bordeaux.
C
C  This version of MUMPS is provided to you free of charge. It is
C  released under the CeCILL-C license:
C  http://www.cecill.info/licences/Licence_CeCILL-C_V1-en.html
C
       MODULE SMUMPS_FAC_S_IS_POINTERS_M
C      ----------------------------------
C      This module defines a type used in 
C      SMUMPS_FAC_DRIVER and SMUMPS_FAC_B
C      ----------------------------------
       TYPE S_IS_POINTERS_T
          REAL, POINTER, DIMENSION(:) :: A
          INTEGER, POINTER, DIMENSION(:) :: IW
       END TYPE S_IS_POINTERS_T
       END MODULE SMUMPS_FAC_S_IS_POINTERS_M
