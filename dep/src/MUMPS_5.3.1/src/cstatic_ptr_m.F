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
      MODULE CMUMPS_STATIC_PTR_M
      PUBLIC :: CMUMPS_TMP_PTR, CMUMPS_GET_TMP_PTR
      COMPLEX, DIMENSION(:), POINTER, SAVE :: CMUMPS_TMP_PTR
      CONTAINS
      SUBROUTINE CMUMPS_SET_STATIC_PTR(ARRAY)
      COMPLEX, DIMENSION(:), TARGET :: ARRAY
      CMUMPS_TMP_PTR => ARRAY
      RETURN
      END SUBROUTINE CMUMPS_SET_STATIC_PTR
      SUBROUTINE CMUMPS_GET_TMP_PTR(PTR)
#if defined(MUMPS_F2003)
      COMPLEX, DIMENSION(:), POINTER, INTENT(OUT) :: PTR
#else
      COMPLEX, DIMENSION(:), POINTER :: PTR
#endif
      PTR => CMUMPS_TMP_PTR
      RETURN
      END SUBROUTINE CMUMPS_GET_TMP_PTR
      END MODULE CMUMPS_STATIC_PTR_M
