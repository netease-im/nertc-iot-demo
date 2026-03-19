
        *(*.text.cache.L1)
        *(*.text.cache.L2)

#if defined CONFIG_RF_TRIM_CODE_MOVABLE
        . = ALIGN(4);
        __rf_trim_movable_slot_start = .;
        *(.movable.slot.1);
        __rf_trim_movable_slot_end = .;
#endif

#ifdef CONFIG_ASR_CODE_MOVABLE
        . = ALIGN(4);
        __asr_movable_slot_start = .;
        *(.movable.slot.2);
        __asr_movable_slot_end = .;
#endif

#ifdef CONFIG_MP3_DEC_CODE_MOVABLE
        . = ALIGN(4);
        __mp3_dec_movable_slot_start = .;
        *(.movable.slot.3);
        __mp3_dec_movable_slot_end = .;
#endif
