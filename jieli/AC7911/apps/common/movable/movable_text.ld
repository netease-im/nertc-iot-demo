
#if defined CONFIG_RF_TRIM_CODE_MOVABLE
        *(.movable.stub.1)
        __rf_trim_movable_region_start = .;
        *(.movable.region.1);
        __rf_trim_movable_region_end = .;
#endif

#ifdef CONFIG_ASR_CODE_MOVABLE
        *(.movable.stub.2)
        __asr_movable_region_start = .;
        *(.movable.region.2);
        __asr_movable_region_end = .;
#endif

#ifdef CONFIG_MP3_DEC_CODE_MOVABLE
        *(.movable.stub.3)
        __mp3_dec_movable_region_start = .;
        *(.movable.region.3);
        __mp3_dec_movable_region_end = .;
#endif
