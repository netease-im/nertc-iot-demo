#include "app_config.h"
#include "generic/typedef.h"
#include "asm/sdram.h"

#ifdef CONFIG_MP3_DEC_CODE_MOVABLE

static const struct sdram_cfg_info_t sdram_cfg = {
    .sdram_size = 2 * 1024 * 1024,
    .sdram_test_size = 4 * 1024,
    .sdram_config_val = -1,
    .sdram_mode = 0,
    .sdram_pll3_en = 0,
    .sdram_pll3_nousb_en = 0,
    .sdram_cl = 2,
    .sdram_rlcnt = 1,
    .sdram_d_dly = 1,
    .sdram_q_dly = 1,
    .sdram_phase = 3,
    .sdram_dq_dly_trm = 4,
};

static u32 *start_of_region = NULL;

extern u32 __mp3_dec_movable_slot_start[];
extern u32 __mp3_dec_movable_slot_end[];
extern u8 __mp3_dec_movable_region_start[];
extern u8 __mp3_dec_movable_region_end[];

static void enter_mp3_dec_movable_region(u32 **start_of_region, u8 *code_load_addr)
{
    u32 code_size = __mp3_dec_movable_region_end - __mp3_dec_movable_region_start;
    code_movable_load(__mp3_dec_movable_region_start, code_size, code_load_addr, __mp3_dec_movable_slot_start, __mp3_dec_movable_slot_end, start_of_region);
}

static void exit_mp3_dec_movable_region(u32 **start_of_region, u8 *code_load_addr)
{
    code_movable_unload(__mp3_dec_movable_region_start, __mp3_dec_movable_slot_start, __mp3_dec_movable_slot_end, start_of_region);
}

void mp3_dec_movable_test_start(void)
{
    sdram_init(&sdram_cfg);

    enter_mp3_dec_movable_region(&start_of_region, __NONCPU_ADDR__);
}

void mp3_dec_movable_test_stop(void)
{
    exit_mp3_dec_movable_region(&start_of_region, __NONCPU_ADDR__);

    sdram_uninit();
}

#endif
