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
      MODULE SMUMPS_STATIC_PTR_M
      PUBLIC :: SMUMPS_TMP_PTR, SMUMPS_GET_TMP_PTR
      REAL, DIMENSION(:), POINTER, SAVE :: SMUMPS_TMP_PTR
      CONTAINS
      SUBROUTINE SMUMPS_SET_STATIC_PTR(ARRAY)
      REAL, DIMENSION(:), TARGET :: ARRAY
      SMUMPS_TMP_PTR => ARRAY
      RETURN
      END SUBROUTINE SMUMPS_SET_STATIC_PTR
      SUBROUTINE SMUMPS_GET_TMP_PTR(PTR)
#if defined(MUMPS_F2003)
      REAL, DIMENSION(:), POINTER, INTENT(OUT) :: PTR
#else
      REAL, DIMENSION(:), POINTER :: PTR
#endif
      PTR => SMUMPS_TMP_PTR
      RETURN
      END SUBROUTINE SMUMPS_GET_TMP_PTR
      END MODULE SMUMPS_STATIC_PTR_M
