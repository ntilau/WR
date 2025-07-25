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
      SUBROUTINE CMUMPS_MCAST2(DATA, LDATA, MPITYPE, ROOT, COMMW, TAG,
     &SLAVEF, KEEP)
      USE CMUMPS_BUF
      IMPLICIT NONE
      INCLUDE 'mpif.h'
      INTEGER IERR
      INTEGER LDATA, ROOT, COMMW, TAG, MPITYPE, SLAVEF
      INTEGER DEST
      INTEGER, INTENT(INOUT) :: KEEP(500)
      INTEGER DATA(LDATA)
      DO 10 DEST = 0, SLAVEF - 1
        IF (DEST .NE. ROOT) THEN
          IF ( LDATA .EQ. 1 .and. MPITYPE .EQ. MPI_INTEGER ) THEN
            CALL CMUMPS_BUF_SEND_1INT( DATA(1), DEST, TAG, 
     &                                COMMW, KEEP, IERR )
          ELSE
            WRITE(*,*) 'Error : bad argument to CMUMPS_MCAST2'
            CALL MUMPS_ABORT()
          END IF
        ENDIF
   10 CONTINUE
      RETURN
      END SUBROUTINE CMUMPS_MCAST2
      SUBROUTINE CMUMPS_BDC_ERROR( MYID, SLAVEF, COMM, KEEP )
      INTEGER MYID, SLAVEF, COMM
      INTEGER, INTENT(INOUT) :: KEEP(500)
      INCLUDE 'mpif.h'
      INCLUDE 'mumps_tags.h'
      INTEGER DUMMY (1)
      DUMMY(1) = -98765
      CALL CMUMPS_MCAST2( DUMMY, 1, MPI_INTEGER, MYID,
     &                 COMM, TERREUR, SLAVEF, KEEP )
      RETURN
      END SUBROUTINE CMUMPS_BDC_ERROR
