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
      SUBROUTINE MUMPS_PRINT_IF_DEFINED(MPG)
      IMPLICIT NONE
      INTEGER, INTENT(IN) :: MPG
      IF (MPG.LE.0) RETURN
      write(MPG,*) "================================================="
#if defined(ZERO_TRIANGLE)
      write(MPG,*) "MUMPS compiled with option -DZERO_TRIANGLE"
#endif
#if defined(GEMMT_AVAILABLE)
      write(MPG, *) "MUMPS compiled with option -DGEMMT_AVAILABLE"
#endif
#if defined(DETERMINISTIC_PARALLEL_GRAPH)
      write(MPG,*) "MUMPS compiled with option"
     &     ," -DDETERMINISTIC_PARALLEL_GRAPH"
#endif
#if defined(metis)
      write(MPG,*) "MUMPS compiled with option -Dmetis"
#endif
#if defined(metis4)
      write(MPG,*) "MUMPS compiled with option -Dmetis4"
#endif
#if defined(MUMPS_F2003)
      write(MPG,*) "MUMPS compiled with option -DMUMPS_F2003"
#endif
#if defined(OLD_OOC_NOPANEL)
      write(MPG,*) "MUMPS compiled with option -DOLD_OOC_NOPANEL"
#endif
#if defined(parmetis)
      write(MPG,*) "MUMPS compiled with option -Dparmetis"
#endif
#if defined(parmetis3)
      write(MPG,*) "MUMPS compiled with option -Dparmetis3"
#endif
#if defined(ptscotch)
      write(MPG,*) "MUMPS compiled with option -Dptscotch"
#endif
#if defined(scotch)
      write(MPG,*) "MUMPS compiled with option -Dscotch"
#endif
#if defined(MUMPS_USE_BLAS2)
      write(MPG,*) "MUMPS compiled with option -DMUMPS_USE_BLAS2"
#endif
#if defined(BLR_MT)
      write(MPG,*) "MUMPS compiled with option -DBLR_MT"
#endif
      write(MPG,*) "This MUMPS version includes code for SAVE_RESTORE"
      write(MPG,*) "This MUMPS version includes code for DIST_RHS"
#if defined(NODYNAMICCB)
      write(MPG,*) "MUMPS compiled with option -DNODYNAMICCB"
#endif
      write(MPG,*) "================================================="
      RETURN
      END SUBROUTINE MUMPS_PRINT_IF_DEFINED
