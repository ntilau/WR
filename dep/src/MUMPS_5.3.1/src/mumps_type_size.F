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
      SUBROUTINE MUMPS_GET_INT_DBL_SIZES( IS, DS )
      INTEGER IS, DS
#if defined(t3e)
      IS = 8
      DS = 16
#else
      IS = 4
      DS = 8
#endif
      END SUBROUTINE MUMPS_GET_INT_DBL_SIZES
