#include "app_config.h"
#include "system/includes.h"
#include "asm/sdram.h"
// #include "asm/p33.h"

#if defined CONFIG_SDRAM_RE_TRIM_ENABLE && !defined CONFIG_NO_SDRAM_ENABLE

#pragma const_seg(".sdram_re_trim_code")
#pragma code_seg(".sdram_re_trim_code")
#pragma str_literal_override(".sdram_re_trim_code")

#define __SDRAM_RE_TRIM_SEC__  SEC(.sdram_re_trim_code)


#define SDRAM_FREE_SPACE_SIZE  ((u32)&HEAP_END - (u32)&HEAP_BEGIN - 0x120)
extern void flushinv_dcache(void *ptr, int len);
extern u32 HEAP_END, HEAP_BEGIN;

// sdram_cfg_info参数需要与sdk生成的ini中的配置一致
__SDRAM_RE_TRIM_SEC__
static struct sdram_cfg_info_t sdram_cfg_info = {
    .sdram_size = __SDRAM_SIZE__,
    .sdram_test_size = 4 * 1024,
    .sdram_config_val = -1,
    .sdram_mode = 0,
    .sdram_rlcnt = 1,
    .sdram_d_dly = 1,
    .sdram_q_dly = 1,
    .sdram_phase = 3,

#if defined CONFIG_OVERCLOCKING_ENABLE
    .sdram_pll3_en = 1,
    .sdram_pll3_nousb_en = 0,
    .sdram_cl = 3,
    .sdram_dq_dly_trm = 5,
#else
    .sdram_pll3_en = 0,
    .sdram_pll3_nousb_en = 0,
#if defined CONFIG_SDRAM_OVERCLOCKING_ENABLE
    .sdram_cl = 3,
    .sdram_dq_dly_trm = 5,
#else
    .sdram_cl = 2,
    .sdram_dq_dly_trm = 4,
#endif
#endif
};

// 本程序打开调试log的步骤：
// 1.uboot需要开打印(即需要使用uboot.boot_debug，并且在ini里配置UTTX打印口)
// 2.UART_IO_TX需要与uboot的UTTX打印口一致
#define UART_IO_TX      "PB03"





#define UART_BUF        JL_UART0->BUF
#define UART_CON        JL_UART0->CON0
#define DELAY_CNT       30

#if 1  //开关调试打印

__SDRAM_RE_TRIM_SEC__
static void putbyte_init(char *tx_pin)
{
    JL_PORT_FLASH_TypeDef *gpio_regs[] = {
        JL_PORTA,
        JL_PORTB,
        JL_PORTC,
        JL_PORTD,
        JL_PORTE,
        JL_PORTF,
        JL_PORTG,
        JL_PORTH,
    };

    JL_PORT_FLASH_TypeDef *g;
    if (tx_pin[0] == 'P' && tx_pin[1] >= 'A' && tx_pin[1] <= 'H') {
        g = gpio_regs[tx_pin[1] - 'A'];
        int mask = (tx_pin[2] - '0') * 10 + (tx_pin[3] - '0');
        if (mask < 16 && mask >= 0) {
            g->DIR &= ~BIT(mask);
            g->PU  |= BIT(mask);
            g->PD  |= BIT(mask);
            g->DIE &= ~BIT(mask);
        }
    }
}

__SDRAM_RE_TRIM_SEC__
static void putbyte_deinit(char *tx_pin)
{
    JL_PORT_FLASH_TypeDef *gpio_regs[] = {
        JL_PORTA,
        JL_PORTB,
        JL_PORTC,
        JL_PORTD,
        JL_PORTE,
        JL_PORTF,
        JL_PORTG,
        JL_PORTH,
    };

    JL_PORT_FLASH_TypeDef *g;
    if (tx_pin[0] == 'P' && tx_pin[1] >= 'A' && tx_pin[1] <= 'H') {
        g = gpio_regs[tx_pin[1] - 'A'];
        int mask = (tx_pin[2] - '0') * 10 + (tx_pin[3] - '0');
        if (mask < 16 && mask >= 0) {
            g->DIR |= BIT(mask);
            g->PU  &= ~BIT(mask);
            g->PD  &= ~BIT(mask);
            g->DIE |= BIT(mask);
        }
    }
}

__SDRAM_RE_TRIM_SEC__
static void putbyte_tmp(char c)
{
    int i = 0;
    while ((UART_CON & BIT(15)) == 0) {
        i++;
        if (i > 1000) {
            break;
        }
    };
    UART_CON |= BIT(13);
    __asm_csync();
    UART_BUF = c;
}

__SDRAM_RE_TRIM_SEC__
static void put_u4hex_tmp(u8 dat)
{
    dat = 0xf & dat;
    if (dat > 9) {
        putbyte_tmp(dat - 10 + 'A');
    } else {
        putbyte_tmp(dat + '0');
    }
}

__SDRAM_RE_TRIM_SEC__
u32 power(u8 x, u8 n)
{
    double result = 1.0;
    for (int i = 0; i < n; i++) {
        result *= x;
    }
    return result;
}


__SDRAM_RE_TRIM_SEC__
void printf_tmp(char *str, u32 value)
{
    while (*str != 0) {
        if (*str == '%') {
            str++;
            u8 format = *str;

            u8 flag = 0;
            if ((format == 'd') || format == 'D') {
                if (value) {
                    for (u8 i = 9; i > 0; i--) {
                        u32 tmp = power(10, i - 1);
                        u8 dat = (value / tmp % 10);
                        if (dat || flag) {
                            put_u4hex_tmp(dat);
                            flag = 1;
                        }
                    }
                } else  {
                    put_u4hex_tmp(0);
                }
            } else if ((format == 'x' || format == 'X')) {
                if (value) {
                    for (u8 i = 8; i > 0; i--) {
                        u8 tmp = 4 * (i - 1);
                        u8 dat = ((value & 0xf << tmp) >> tmp);
                        if (dat || flag) {
                            put_u4hex_tmp(dat);
                            flag = 1;
                        }
                    }
                } else  {
                    put_u4hex_tmp(0);
                }
            }
            str++;
            if (*str == 0) {
                break;
            }
        }
        putbyte_tmp(*str);
        str++;
    }
}

__SDRAM_RE_TRIM_SEC__
void putbuf_tmp(u8 *buf, u32 len)
{
    u32 tmp = len;
    while (len--) {
        put_u4hex_tmp((*buf >> 4) & 0xf);
        put_u4hex_tmp(*buf & 0xf);
        putbyte_tmp(' ');
        buf++;

        if (!((tmp - len) % 16)) {
            putbyte_tmp('\n');
        }
    }
}
#else

#define putbyte_init(x)
#define putbyte_tmp(x)
#define put_u4hex_tmp(x)
#define printf_tmp(x, y)
#define putbuf_tmp(x)

#endif

__SDRAM_RE_TRIM_SEC__
void *memset_tmp(void *dest, u8 value, u32 count)
{
    u8 *ptr = (u8 *)dest;
    u8 byte_value = (u8)value;

    for (u32 i = 0; i < count; i++) {
        ptr[i] = byte_value;
    }

    return dest;
}


/******************************************************************
====================== 4pahse时钟相位关系==========================
             ___     ___     ___     ___     ___     ___
0,相位0：___|   |___|   |___|   |___|   |___|   |___|   |___
               ___     ___     ___     ___     ___     ___
1,相位90： ___|   |___|   |___|   |___|   |___|   |___|   |___
                 ___     ___     ___     ___     ___     ___
2,相位180：  ___|   |___|   |___|   |___|   |___|   |___|   |___
                   ___     ___     ___     ___     ___     ___
3,相位270：    ___|   |___|   |___|   |___|   |___|   |___|   |___
********************************************************************/

struct sdram_test_cfg {
    u8 i: 4;
    u8 j: 4;
    u8 k: 4;
    u8 m: 3;
    u8 wr: 1;//标记写
};

__SDRAM_RE_TRIM_SEC__
struct sdram_test_cfg sdram_cfg_ok = {0};

struct sdram_cfg {
    u8 sdram_size;
    u8 cfg_value;
    u8 i: 4;
    u8 j: 4;
    u8 k: 4;
    u8 m: 4;
};

#define I_MIN 5
#define I_MAX 7
#define M_MIN 1
#define M_MAX 4

#define J_MIN_2  1
#define J_MAX_2  1
#define I_MIN_2  3
#define I_MAX_2  7
#define K_MIN_2  0
#define K_MAX_2  3
#define M_MIN_2  2
#define M_MAX_2  2

#define T(n) C(n##_MAX_2, n##_MIN_2)
#define C(max, min) (max - min + 1)

#define __SDRAM_ADDR        ((u8*)0x4000000)
#define SDRAM_ADDR(addr)    ((void*)(__SDRAM_ADDR+(addr)))

// mode 0:以u8、u16、u32方式，校验test_size长度
// mode 1:以u32方式校验整块sdram
__SDRAM_RE_TRIM_SEC__
static char sdram_test(u32 test_size, u8 mode)
{
    u32 i, checksum ;
    u32 *ptr_u32 = (u32 *)&HEAP_BEGIN;
    u16 *ptr_u16 = (u16 *)ptr_u32;
    u8 *ptr_u8 = (u8 *)ptr_u16;
#define CHECK_RAND_EN	0

    /* printf_tmp("test_size = %dK\n", test_size / 1024); */
    /* printf_tmp("ptr_u8 = 0x%x\n", ptr_u8); */

    // CLOSE_WDT();
    // p33_tx_1byte(P3_WDT_CON, 0);

    if (mode) {
        goto __sdram_u32_check;
    }

    checksum = 0;

    for (i = 0 ; i < test_size ; i++) {
        ptr_u8[i] = CHECK_RAND_EN ? (u8)JL_RAND->R64L : i;
        checksum += ptr_u8[i];
    }

    flushinv_dcache(ptr_u8, test_size);

    for (i = 0 ; i < test_size ; i++) {
        checksum -=  ptr_u8[i];

        if (!CHECK_RAND_EN && ptr_u8[i] != (u8)i) {
            // printf_tmp("err %d = ", (u8)i) ;
            // printf_tmp("%d\n", ptr_u8[i]) ;
            // putbuf_tmp(ptr_u8, 64);
            /*while(1);*/
            return -1;
        }
    }

    if (checksum) {
        return -1;
    }

///////////////////////////////////////////////////////////////////////////////////////
    checksum = 0;

    for (i = 0 ; i < test_size / 2 ; i++) {
        ptr_u16[i] = CHECK_RAND_EN ? (u16)JL_RAND->R64L : i;
        checksum += ptr_u16[i];
    }

    flushinv_dcache(ptr_u16, test_size);

    for (i = 0 ; i < test_size / 2 ; i++) {
        checksum -= ptr_u16[i];
        if (!CHECK_RAND_EN && ptr_u16[i] != (u16)i) {
            /*printf("sdram test u16 err =0x%x, 0x%x\r\n", (u16)i, ptr_u16[i]) ;*/
            /*putbuf_tmp(&ptr_u16[i], 256);*/
            /*while(1);*/
            return -1;
        }
    }

    if (checksum) {
        return -1;
    }

///////////////////////////////////////////////////////////////////////////////////////
__sdram_u32_check:

    checksum = 0;

    for (i = 0 ; i < test_size / 4 ; i++) {
        ptr_u32[i] = CHECK_RAND_EN ? (u32)JL_RAND->R64L : i;
        checksum += ptr_u32[i];
    }

    flushinv_dcache(ptr_u32, test_size);

    for (i = 0 ; i < test_size / 4 ; i++) {
        checksum -= ptr_u32[i];
        if (!CHECK_RAND_EN && ptr_u32[i] != (u32)i) {
            /*___trig;*/
            /*printf("sdram test u32 err =0x%x, 0x%x\r\n", (u32)i, ptr_u32[i]) ;*/
            /*putbuf_tmp(&ptr_u32[i], 256);*/
            /*while(1);*/
            return -1;
        }
    }

    if (checksum) {
        return -1;
    }

    /*puts("\r\n\r\n sdram test over\r\n");*/
    return 0;
}

__SDRAM_RE_TRIM_SEC__
static void sdram_test_ijkm_save(u8 pass_list[][T(I)][T(K)][T(M)], u8 i, u8 j, u8 k, u8 m)
{
    pass_list[j - J_MIN_2][i - I_MIN_2][k - K_MIN_2][m - M_MIN_2] = 1;
}


struct continue_group_tag {
    u8 start_j;   ///< 当前group的起始组合的J值
    u8 start_i;   ///< 当前group的起始组合的I值
    u8 start_k;   ///< 当前group的起始组合的K值
    u8 start_m;   ///< 当前group的起始组合的M值
    u8 cnt;       ///< 当前group的组合数量
};

__SDRAM_RE_TRIM_SEC__
struct continue_group_tag group[T(J) * T(I) * T(K) * T(M) / 2] = {0};  ///< group数量不会超过所有组合数的1/2

__SDRAM_RE_TRIM_SEC__
static u8 sdram_test_ijkm_find(u8 pass_list[][T(I)][T(K)][T(M)])
{
    u8 i, j, k, m;
    u8 start_i, start_j, start_k, start_m;

    u8 last_para_pass_flag = 0; ///< 记录遍历过程中，上一组参数是否pass，以便找到连续pass的组
    u8 continue_cnt = 0;        ///< 记录连续pass的组的个数(即某group中的成员数)
    u8 group_id = 0;
    u8 group_max_id = 0;        ///< 记录group中组合数最多的id
    u8 group_max_cnt = 0;       ///< 记录group中组合数最多的成员数

    // 初始化最终选取的jikm参数
    sdram_cfg_ok.j = 0;
    sdram_cfg_ok.i = 0;
    sdram_cfg_ok.k = 0;
    sdram_cfg_ok.m = 0;

    /* printf_tmp("~~~~~~sdram parameter pass table~~~~~~\n", 0); */
    // 遍历所有pass组合，找到连续pass的组(姑且称之为group)
    for (j = J_MIN_2; j <= J_MAX_2; j++) {
        for (i = I_MIN_2; i <= I_MAX_2; i++) {
            for (k = K_MIN_2; k <= K_MAX_2; k++) {
                for (m = M_MIN_2; m <= M_MIN_2; m++) {
                    if (pass_list[j - J_MIN_2][i - I_MIN_2][k - K_MIN_2][m - M_MIN_2]) {
                        if (!last_para_pass_flag) {
                            start_j = j;
                            start_i = i;
                            start_k = k;
                            start_m = m;
                        }
                        continue_cnt++;
                        last_para_pass_flag = 1;
                    } else {
                        if (continue_cnt > 1) {
                            group[group_id].start_j = start_j;
                            group[group_id].start_i = start_i;
                            group[group_id].start_k = start_k;
                            group[group_id].start_m = start_m;
                            group[group_id].cnt = continue_cnt;
                            group_id++;
                        }
                        last_para_pass_flag = 0;
                        continue_cnt = 0;
                    }
                }
            }
        }
    }

    // 找到最多连续组的group，并打印所有group的组合(group: 连续的组合的数量至少两个)
    for (u8 id = 0; id < T(J) * T(I) * T(K) * T(M) / 2; id++) {
        if (group[id].cnt) {
            if (group_max_cnt <= group[id].cnt) {
                group_max_id = id;
                group_max_cnt = group[id].cnt;
            }
        }
    }

    if (group_max_cnt > 1) {
        // 找到了最多成员的group的中间的那个组合
        u8 offset = group_max_cnt / 2 + 1; // 奇数取中间值，偶数取<中间值+1>，因为trim流程在芯片启动时，温度较低，正常工作后，温度上升，会导致时序有所变化，因此往高取
        u8 start = 0;
        for (j = J_MIN_2; j <= J_MAX_2; j++) {
            for (i = I_MIN_2; i <= I_MAX_2; i++) {
                for (k = K_MIN_2; k <= K_MAX_2; k++) {
                    for (m = M_MIN_2; m <= M_MIN_2; m++) {
                        if (group[group_max_id].start_j == j && \
                            group[group_max_id].start_i == i && \
                            group[group_max_id].start_k == k && \
                            group[group_max_id].start_m == m) {
                            start = 1;
                        }
                        if (start) {
                            offset--;
                            if (offset == 0) {
                                sdram_cfg_ok.j = j;
                                sdram_cfg_ok.i = i;
                                sdram_cfg_ok.k = k;
                                sdram_cfg_ok.m = m;
                                goto __find_end;
                            }
                        }
                    }
                }
            }
        }
__find_end:
        return 1;
    } else {
        // 没有找到group
        printf_tmp("err in find jikm config\n", 0);
        return 0;
    }
}


#define sdr_con0_init                  (          \
        /* 30-16 -> MRS configure       */ ( 0<<16) | \
        /* 15-13 -> MRS select          */ ( 0<<13) | \
        /*    12 -> onek_one_bank       */ ( 0<<12) | \
        /*    11 -> sdr_en              */ ( 1<<11) | \
        /*    10 -> ddr_reset           */ ( 1<<10) | \
        /*    09 -> ddr_cke             */ ( 1<< 9) | \
        /*    08 -> auto-refresh_en     */ ( 0<< 8) | \
        /*    07 -> zq                  */ ( 0<< 7) | \
        /*    06 -> mode register set   */ ( 0<< 6) | \
        /*    05 -> power down mode     */ ( 0<< 5) | \
        /* 04-03 -> ddr_type            */ ( 0<< 3) | \
        /* 02-00 -> ddr_size            */ ( 0<< 0))

#define sdr_con1_init                  (          \
        /*  31-28-> ccd_conf            */ ( 5<<28) | \
        /*  27-24-> rtw_conf            */ ( 10<<24) | \
        /*  23-20-> cl_conf             */ ( 5<<20) | \
        /*  19-16-> wtr_conf            */ ( 8<<16) | \
        /*  15-12-> wr_conf             */ ( 8<<12) | \
        /*  11-8 -> wl_conf             */ ( 0<< 8) | \
        /*    7  -> self-refresh mode   */ ( 0<< 7) | \
        /*    6  -> precharge all       */ ( 0<< 6) | \
        /*    5  -> leave power_down    */ ( 0<< 5) | \
        /*  4-0  -> tREFI               */ ( 0<< 0))

#define sdr_con2_init                  (          \
        /* 31-27 -> xs_conf             */ ( 10<<27) | \
        /* 26-21 -> ras_conf            */ ( 8<<21) | \
        /* 20-16 -> rcd_rp_conf         */ ( 6<<16) | \
        /* 15-08 -> faw_conf            */ ( 9<< 8) | \
        /* 07-04 -> rrd_conf            */ ( 7<< 4) | \
        /* 03-00 -> rtp_conf            */ ( 7<< 0))


#define EMRS_CON                       (          \
        /*    09 -> wbl                 */ ( 0<<25) | \
        /* 08-07 -> TM                  */ ( 0<<23) | \
        /* 06-04 -> CL                  */ ( 3<<20) | \
        /*     3 -> bt                  */ ( 0<<19) | \
        /* 02-00 -> bl                  */ ( 1<<16) | \
        /* 15-13 -> MR0                 */ ( 0<<13) | \
        /* else                         */ ( 0<< 0))

#define sdr_con3_init                  (          \
        /* 27-25 -> reserved            */ ( 0<<31) | \
        /* 27-25 -> reserved            */ ( 0<<30) | \
        /* 27-25 -> reserved            */ ( 0<<29) | \
        /* 27-25 -> reserved            */ ( 0<<28) | \
        /* 27-25 -> reserved            */ ( 0<<25) | \
        /* 24    -> dq_dly_sel          */ ( 1<<24) | \
        /* 23-16 -> auto_ref_quen       */ ( 8<<16) | \
        /* 15-12 -> ddr_phy wl_cnt      */ ( 0<<12) | \
        /* 11-8  -> ddr_phy cl_cnt      */ ( 0<< 8) | \
        /* 7-0   -> rfc_conf            */ ( 18<< 0))

__SDRAM_RE_TRIM_SEC__
static void sdram_set_para(int i, int j, int k, int m, u32 sdr_con0, u32 dq_dly)
{
#if  0
    JL_CLOCK->CLK_CON1 &= ~(BIT(21) | BIT(20));
    SFR(JL_CLOCK->CLK_CON1, 25, 2, j);//OCK相位p0...p3
    SFR(JL_CLOCK->CLK_CON1, 30, 2, k);//QCK相位p0...p3
    JL_IOMAP->CON3 |= BIT(13) | BIT(17);    //no use
    JL_IOMAP->CON0 |= BIT(29);    //dq_dly_sel
    JL_PORTE->DIR = 0x0000;
    JL_PORTE->HD = 0xffff;
    JL_PORTE->HD0 = 0x0000;
    JL_PORTF->DIR = 0x0000;
    JL_PORTF->HD = 0xffff;
    JL_PORTF->HD0 = 0x0000;
    JL_PORTG->HD = 0xffff;
    JL_PORTG->HD0 = 0x0000;
    JL_SDR->CON0 = sdr_con0;                        // config reg
    JL_SDR->CON1 = sdr_con1_init;                   // config reg
    JL_SDR->CON2 = sdr_con2_init;                   // config reg
    delay(DELAY_CNT);                              // >= 200us after clk stable
    JL_SDR->CON1 = sdr_con1_init | (1 << 6) | (dq_dly << 0);// precharge all
    delay(DELAY_CNT);
    JL_SDR->CON0 = sdr_con0 | EMRS_CON | (1 << 6);  // EMRS
    delay(DELAY_CNT);
    JL_SDR->CON0 = sdr_con0 | (1 << 8);             // auto refresh
    delay(DELAY_CNT);
    /* JL_SDR->CON1 = sdr_con1_init | (1<<0);        //adjust tREFI */
    JL_SDR->CON3 = sdr_con3_init | (i << 28) | (m << 8);
    delay(DELAY_CNT);
#else
    SFR(JL_CLOCK->CLK_CON1, 25, 2, j);//OCK相位p0...p3
    SFR(JL_CLOCK->CLK_CON1, 30, 2, k);//QCK相位p0...p3
    delay(DELAY_CNT);                              // >= 200us after clk stable
    JL_SDR->CON3 = sdr_con3_init | (i << 28) | (m << 8);
    delay(DELAY_CNT);
#endif
}

__SDRAM_RE_TRIM_SEC__
void sdram_re_trim_main(struct sdram_cfg_info_t *sdram_info)
{
    u8 sdram_test_pass_list[T(J)][T(I)][T(K)][T(M)]; ///< jikm排列组合测试，pass时对应组会置1
    struct sdram_cfg sdram_cfg_table[] = {
        {   2, 0, 6, 3, 0, 2},
        {   4, 0, 6, 3, 0, 2},
        {   8, 1, 6, 3, 0, 2},
        {  16, 2, 6, 0, 0, 0},
        {  32, 3, 6, 0, 0, 0},
        {  64, 4, 6, 0, 0, 0},
        { 128, 5, 6, 0, 0, 0},
    };

    int i, j, k, m;
    u32 sdr_con0 = sdr_con0_init;
    u8 index = 0;
    u8 retry_cnt = 0;
    u8 find_cfg_ok = 0, save_flash = 0;
    char test_ok_phase = 0;
    u32 sdram_test_len = (sdram_info->sdram_test_size ? sdram_info->sdram_test_size : 4096);
    u32 sdram_ijkm = 0;//0-23
    u8 sdram_cl = 0;//24-27

    for (index = 0; index < 7; ++index) {
        if (sdram_cfg_table[index].sdram_size == sdram_info->sdram_size / (1024 * 1024)) {
            i = sdram_cfg_table[index].i;
            j = sdram_cfg_table[index].j;
            k = sdram_cfg_table[index].k;
            m = sdram_cfg_table[index].m;
            sdr_con0 |= sdram_cfg_table[index].cfg_value;
            printf_tmp("sdr_con0 = 0x%x\n", sdr_con0);
            printf_tmp("sdram_size = %dM\n", sdram_cfg_table[index].sdram_size);
            break;
        }
    }

    if (index == 7) {
        printf_tmp("sdram size error!!!\n", 0);
        while (1);
    }

    // sdram_test_ijkm_init();
    memset_tmp(&sdram_test_pass_list, 0, sizeof(sdram_test_pass_list));

    /*i = 6, j = 1, k = 0, m = 1;*/
    test_ok_phase = 0;

    if (sdram_info->sdram_config_val != (u32) - 1 && sdram_info->sdram_config_val) {
        sdram_ijkm = sdram_info->sdram_config_val & 0xFFFFFF;//0-23
        sdram_cl = (sdram_info->sdram_config_val >> 24) & 0xF;//24-27
        if (sdram_ijkm && sdram_info->sdram_cl == sdram_cl) {
            i = sdram_ijkm & 0xF;
            j = (sdram_ijkm >> 4) & 0xF;
            k = (sdram_ijkm >> 8) & 0xF;
            m = (sdram_ijkm >> 12) & 0xF;
            test_ok_phase = 1;
        } else {
            sdram_ijkm = 0;
            sdram_test_len = SDRAM_FREE_SPACE_SIZE;
        }
    } else if (sdram_info->sdram_config_val == (u32) - 1) {
        sdram_test_len = (sdram_info->sdram_test_size ? sdram_info->sdram_test_size : 4096);
        test_ok_phase = 0;//在ini文件配置sdram_config_val选项，一般为发布版SDK，此时需要先检测所以ijkm的最大频率出现值的配置，然后使用该值
        save_flash = 1;
        // printf_tmp("---> used defalut i, j, k, m\n", 0);
    }
    if (sdram_info->sdram_pll3_en || sdram_info->sdram_pll3_nousb_en) {
        if (test_ok_phase) {
            goto SDRAM_8PHASE;
        }
        goto SDRAM_USE_8PAHSE;
    }
    if (test_ok_phase) {
        goto SDRAM_PHASE;
    }

retry:
    retry_cnt++;
    // sdram_test_ijkm_init();
    memset_tmp(&sdram_test_pass_list, 0, sizeof(sdram_test_pass_list));
    if (find_cfg_ok) {//查找出最后频率最高最优参数
        i = sdram_cfg_ok.i;
        j = sdram_cfg_ok.j;
        k = sdram_cfg_ok.k;
        m = sdram_cfg_ok.m;
        if (save_flash) {
            sdram_test_len = SDRAM_FREE_SPACE_SIZE;
        } else {
            sdram_test_len = (sdram_info->sdram_test_size ? sdram_info->sdram_test_size : 4096);
        }
        test_ok_phase = 1;
        goto SDRAM_PHASE;
    } else if (retry_cnt > 2) {
        sdram_test_len = (sdram_info->sdram_test_size ? sdram_info->sdram_test_size : 4096);
        test_ok_phase = 1;
    }

    // 4phase的新策略
    for (j = J_MIN_2; j <= J_MAX_2; j++) {
        for (i = I_MIN_2; i <= I_MAX_2; i++) {
            for (k = K_MIN_2; k <= K_MAX_2; k++) {
                for (m = M_MIN_2; m <= M_MIN_2; m++) {
SDRAM_PHASE:
                    sdram_set_para(i, j, k, m, sdr_con0, sdram_info->sdram_dq_dly_trm);

                    if (!sdram_info->sdram_d_dly) {
                        printf_tmp("no test sdram_8phase\n", 0);
                        return;
                    }

                    printf_tmp("j = %d, ", j);
                    printf_tmp("i = %d, ", i);
                    printf_tmp("k = %d, ", k);
                    printf_tmp("m = %d ", m);
                    u8 sdram_check_mode = find_cfg_ok ? 1 : 0;
                    if (0 == sdram_test(sdram_test_len, sdram_check_mode)) {
                        printf_tmp("pass!!!\n", 0);
                        if (!find_cfg_ok) {
                            sdram_test_ijkm_save(sdram_test_pass_list, i, j, k, m);//存储ijkm到四维数组
                        }
                        if (sdram_info->sdram_config_val == (u32) - 1 && test_ok_phase) {
                            // i:0-3, j:4-7; k:8-11; m:12-15
                            sdram_info->sdram_config_val = i | (j << 4) | (k << 8) | (m << 12) | \
                                                           ((sdram_info->sdram_cl & 0xF) << 24);
                            sdram_info->sdram_config_val |= BIT(31);
                        }
                        if (test_ok_phase) {
                            return;
                        }
                    } else {
                        printf_tmp("fail\n", 0);
                    }
                }
            }
        }
    }


    find_cfg_ok = sdram_test_ijkm_find(sdram_test_pass_list);//查找最优参数因子

    if (retry_cnt < 3) {
        printf_tmp("sdram_4phase retry\n", 0);
        goto retry;
    }
    printf_tmp("sdram_4phase test fix fail !!!!\n", 0);
    cpu_reset();
    return;

SDRAM_USE_8PAHSE:
    retry_cnt++;
    // sdram_test_ijkm_init();
    memset_tmp(&sdram_test_pass_list, 0, sizeof(sdram_test_pass_list));
    if (find_cfg_ok) {//查找出最后频率最高最优参数
        i = sdram_cfg_ok.i;
        j = sdram_cfg_ok.j;
        k = sdram_cfg_ok.k;
        m = sdram_cfg_ok.m;
        if (save_flash) {
            sdram_test_len = SDRAM_FREE_SPACE_SIZE;
        } else {
            sdram_test_len = (sdram_info->sdram_test_size ? sdram_info->sdram_test_size : 4096);
        }
        test_ok_phase = 1;
        goto SDRAM_8PHASE;
    } else if (retry_cnt > 2) {
        sdram_test_len = (sdram_info->sdram_test_size ? sdram_info->sdram_test_size : 4096);
        test_ok_phase = 1;
    }

    for (m = M_MIN; m < M_MAX; m++) {//剔除m=0和m=4
        for (i = I_MIN; i < I_MAX; i++) {//i从5开始，6结束
            for (j = 0; j < 8; j++) {
                for (k = 0; k < 8; k++) {
SDRAM_8PHASE:
                    //8phase
                    JL_CLOCK->CLK_CON1 &= ~BIT(20);
                    JL_CLOCK->CLK_CON1 |= BIT(21);
                    SFR(JL_CLOCK->CLK_CON2, 16, 3, j);//OCK相位p0...p7
                    SFR(JL_CLOCK->CLK_CON2, 19, 3, k);//QCK相位p0...p7
                    JL_IOMAP->CON3 |= BIT(13) | BIT(17);    //no use
                    JL_IOMAP->CON0 |= BIT(29);    //dq_dly_sel
                    JL_PORTE->DIR = 0x0000;
                    JL_PORTE->HD = 0xffff;
                    JL_PORTE->HD0 = 0x0000;
                    JL_PORTF->DIR = 0x0000;
                    JL_PORTF->HD = 0xffff;
                    JL_PORTF->HD0 = 0x0000;
                    JL_PORTG->HD = 0xffff;
                    JL_PORTG->HD0 = 0x0000;
                    JL_SDR->CON0 = sdr_con0;
                    JL_SDR->CON1 = sdr_con1_init;
                    JL_SDR->CON2 = sdr_con2_init;
                    delay(0x5);                          // >= 200us after clk stable
                    JL_SDR->CON1 = sdr_con1_init | (1 << 6) | (sdram_info->sdram_dq_dly_trm << 0);// precharge all
                    delay(0x5);
                    JL_SDR->CON0 = sdr_con0 | EMRS_CON | (1 << 6);   // EMRS
                    delay(0x5);
                    JL_SDR->CON0 = sdr_con0 | (1 << 8);              // auto refresh
                    delay(0x5);
                    /*JL_SDR->CON1 = sdr_con1_init | (1<<0);         //adjust tREFI*/
                    JL_SDR->CON3 = sdr_con3_init | (i << 28) | (m << 8);
                    delay(0x5);

                    if (!sdram_info->sdram_d_dly) {
                        /* printf_tmp("no test sdram_8phase: i=%d,j=%d,k=%d,m=%d\n", i, j, k, m); */
                        return;
                    }
                    u8 sdram_check_mode = find_cfg_ok ? 1 : 0;
                    if (0 == sdram_test(sdram_test_len, sdram_check_mode)) {
                        /*printf_tmp("i=%d,j=%d,k=%d,m=%d;\r\n", i, j, k, m);*/
                        if (!find_cfg_ok) {
                            sdram_test_ijkm_save(sdram_test_pass_list, i, j, k, m);//存储ijkm到四维数组
                        }
                        if (sdram_info->sdram_config_val == (u32) - 1 && test_ok_phase) {
                            sdram_info->sdram_config_val = i | (j << 4) | (k << 8) | (m << 12) | ((sdram_info->sdram_cl & 0xF) << 24);//i:0-3, j:4-7; k:8-11; m:12-15;
                            sdram_info->sdram_config_val |= BIT(31);
                        }
                        if (test_ok_phase) {
                            /* printf_tmp("init_sdram_8phase test ok: i=%d,j=%d,k=%d,m=%d\n", i, j, k, m); */
                            return;
                        }
                    }
                }
            }
        }
    }
    find_cfg_ok = sdram_test_ijkm_find(sdram_test_pass_list);//查找最优参数因子
    if (retry_cnt < 3) {
        printf_tmp("sdram_8phase retry\n", 0);
        goto SDRAM_USE_8PAHSE;
    }
    printf_tmp("sdram_8phase test fix fail !!!!n", 0);
    cpu_reset();
}

__SDRAM_RE_TRIM_SEC__
void startup_boot_hook_after_ram_ready(void)
{
    putbyte_init(UART_IO_TX);

    printf_tmp("=======sdram re-trim(code in ram0)=======\n", 0);
    printf_tmp("HEAP_BEGIN = 0x%x\n", &HEAP_BEGIN);
    printf_tmp("HEAP_END = 0x%x\n", &HEAP_END);
    printf_tmp("SDRAM_FREE_SPACE_SIZE = 0x%x\n", SDRAM_FREE_SPACE_SIZE);

    sdram_re_trim_main(&sdram_cfg_info);

    putbyte_deinit(UART_IO_TX);
}

#endif
