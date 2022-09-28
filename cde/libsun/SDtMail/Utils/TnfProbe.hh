/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

#ifndef _SDM_TNF_PROBE_H
#define _SDM_TNF_PROBE_H

#pragma ident "@(#)TnfProbe.hh	1.1 96/05/10 SMI"

#ifdef SDM_TNF_PROBE

#include <tnf/probe.h>

#else /* SDM_TNF_PROBE */

/*
 * Define TNF_PROBE_? to null statements
 */

#define	TNF_PROBE_0(namearg, keysarg, detail) \
		((void)0)

#define	TNF_PROBE_1(namearg, keysarg, detail, type_1, namearg_1, valarg_1) \
		((void)0)

#define	TNF_PROBE_2(namearg, keysarg, detail, type_1, namearg_1, valarg_1, type_2, namearg_2, valarg_2) \
		((void)0)

#define	TNF_PROBE_3(namearg, keysarg, detail, type_1, namearg_1, valarg_1, type_2, namearg_2, valarg_2, type_3, namearg_3, valarg_3) \
		((void)0)

#define	TNF_PROBE_4(namearg, keysarg, detail, type_1, namearg_1, valarg_1, type_2, namearg_2, valarg_2, type_3, namearg_3, valarg_3, type_4, namearg_4, valarg_4) \
		((void)0)

#define	TNF_PROBE_5(namearg, keysarg, detail, type_1, namearg_1, valarg_1, type_2, namearg_2, valarg_2, type_3, namearg_3, valarg_3, type_4, namearg_4, valarg_4, type_5, namearg_5, valarg_5) \
		((void)0)

#endif /* SDM_TNF_PROBE */

#endif /* _SDM_TNF_PROBE_H */
