
#include "fsm.h"

const uint8_t fsm[STATE_COUNT] =
{
	/*                IN_00    IN_01_EF    IN_F0        IN_F1        IN_F2        IN_F3_FE    IN_FF      */
	/* GOOD        */ GOOD_00, GOOD_01_FE, GOOD_01_FE,  GOOD_01_FE,  GOOD_01_FE,  GOOD_01_FE, GOOD_FF,
	/* GOOD_00     */ GOOD,    GOOD,       GOOD,        GOOD,        GOOD,        GOOD,       GOOD,
	/* GOOD_01_FE  */ GOOD,    GOOD,       GOOD,        GOOD,        GOOD,        GOOD,       BAD_FF,
	/* GOOD_FF     */ GOOD,    GOOD,       GOOD,        GOOD,        GOOD,        GOOD,       GOOD_FFFF,
	/* GOOD_FFFF   */ BAD,     BAD,        BAD,         BAD,         BAD,         BAD,        GOOD_FFFFFF,
	/* GOOD_FFFFFF */ BAD,     BAD,        GOOD_RETURN, ROUTE,       TIMING,      BAD,        BAD_FFFFFF,
	/* BAD         */ BAD,     BAD,        BAD,         BAD,         BAD,         BAD,        BAD_FF,
	/* BAD_FF      */ BAD,     BAD,        BAD,         BAD,         BAD,         BAD,        BAD_FFFF,
	/* BAD_FFFF    */ BAD,     BAD,        BAD,         BAD,         BAD,         BAD,        BAD_FFFFFF,
	/* BAD_FFFFFF  */ BAD,     BAD,        BAD_RETURN,  BAD_RETURN,  BAD_RETURN,  BAD,        BAD_FFFFFF,
};

