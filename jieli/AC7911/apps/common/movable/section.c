#include "app_config.h"

#ifdef CONFIG_RF_TRIM_CODE_MOVABLE
1:
.rf.text.cache.L2.rf_trim
#endif

#ifdef CONFIG_ASR_CODE_MOVABLE
2:
.libaisp.text
#endif

#ifdef CONFIG_MP3_DEC_CODE_MOVABLE
3:
.mp3_dec_sparse_code
.mp3_dec_code
#endif
