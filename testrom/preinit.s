
.import copydata
.import __PREINIT_RUN__

.segment "PREINIT"

        jsr     copydata

.segment "VECTORS"

.addr   __PREINIT_RUN__ ; reset vector
.addr   __PREINIT_RUN__ ; interrupt vector