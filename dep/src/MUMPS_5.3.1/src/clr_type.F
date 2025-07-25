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
      MODULE CMUMPS_LR_TYPE
      IMPLICIT NONE
      TYPE LRB_TYPE
        COMPLEX,POINTER,DIMENSION(:,:) :: Q => null()
        COMPLEX,POINTER,DIMENSION(:,:) :: R => null()
        INTEGER :: K,M,N
        LOGICAL :: ISLR
      END TYPE LRB_TYPE
      CONTAINS
      SUBROUTINE DEALLOC_LRB(LRB_OUT,KEEP8)
        TYPE(LRB_TYPE), INTENT(INOUT) :: LRB_OUT
        INTEGER(8) :: KEEP8(150)
        INTEGER :: MEM
        IF (LRB_OUT%M.EQ.0) RETURN
        IF (LRB_OUT%N.EQ.0) RETURN
        MEM = 0
        IF (LRB_OUT%ISLR) THEN
           IF(associated(LRB_OUT%Q)) MEM = MEM + size(LRB_OUT%Q)
           IF(associated(LRB_OUT%R)) MEM = MEM + size(LRB_OUT%R)
        ELSE
           IF(associated(LRB_OUT%Q)) MEM = MEM + size(LRB_OUT%Q)
        ENDIF
!$OMP   ATOMIC UPDATE
        KEEP8(69) = KEEP8(69) - int(MEM,8)
!$OMP   END ATOMIC
!$OMP   ATOMIC UPDATE
        KEEP8(71) = KEEP8(71) - int(MEM,8)
!$OMP   END ATOMIC
!$OMP   ATOMIC UPDATE
        KEEP8(73) = KEEP8(73) - int(MEM,8)
!$OMP   END ATOMIC
        IF (LRB_OUT%ISLR) THEN
          IF (associated(LRB_OUT%Q)) THEN
            DEALLOCATE (LRB_OUT%Q)
            NULLIFY(LRB_OUT%Q)
          ENDIF
          IF (associated(LRB_OUT%R)) THEN
            DEALLOCATE (LRB_OUT%R)
            NULLIFY(LRB_OUT%R)
          ENDIF
        ELSE
          IF (associated(LRB_OUT%Q)) THEN
            DEALLOCATE (LRB_OUT%Q)
            NULLIFY(LRB_OUT%Q)
          ENDIF
        ENDIF
      END SUBROUTINE DEALLOC_LRB
      SUBROUTINE DEALLOC_BLR_PANEL(BLR_PANEL, NB_BLR, KEEP8)
        INTEGER, INTENT(IN)           :: NB_BLR
        TYPE(LRB_TYPE), INTENT(INOUT) :: BLR_PANEL(:) 
        INTEGER(8) :: KEEP8(150)
        INTEGER :: I
        IF (NB_BLR.GT.0) THEN
          IF (BLR_PANEL(1)%M.NE.0) THEN
            DO I=1, NB_BLR
              CALL DEALLOC_LRB(BLR_PANEL(I), KEEP8)
            ENDDO
          ENDIF
        ENDIF
      END SUBROUTINE DEALLOC_BLR_PANEL
      END MODULE CMUMPS_LR_TYPE
