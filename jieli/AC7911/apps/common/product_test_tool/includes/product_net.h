#ifndef __PRODUCT_NET__
#define __PRODUCT_NET__


typedef unsigned int UINT32;
typedef void (*usb_complete_t)(struct urb *);

typedef	struct {
    /* Word0 */ //modify by shunjian
//	UINT32		WIV:1;	/* Wireless Info Valid. 1 if Driver already fill WI,  o if DMA needs to copy WI to correctposition */
//	UINT32		QSEL:2;	/* select on-chip FIFO ID for 2nd-stage output scheduler.0:MGMT, 1:HCCA 2:EDCA */
//	UINT32		reserv:29;

    /* Word1 */
    /* ex: 00 03 00 40 means txop = 3, PHYMODE = 1 */
    UINT32		FRAG: 1;		/* 1 to inform TKIP engine this is a fragment. */
    UINT32		MIMOps: 1;	/* the remote peer is in dynamic MIMO-PS mode */
    UINT32		CFACK: 1;
    UINT32		TS: 1;

    UINT32		AMPDU: 1;
    UINT32		MpduDensity: 3;
    UINT32		txop: 2;	/*FOR "THIS" frame. 0:HT TXOP rule , 1:PIFS TX ,2:Backoff, 3:sifs only when previous frame exchange is successful. */
    UINT32		rsv: 6;

    UINT32		MCS: 7;
    UINT32		BW: 1;	/*channel bandwidth 20MHz or 40 MHz */
    UINT32		ShortGI: 1;
    UINT32		STBC: 2;	/* 1: STBC support MCS =0-7,   2,3 : RESERVE */
    UINT32		Ifs: 1;
    UINT32		rsv2: 2;	/*channel bandwidth 20MHz or 40 MHz */
    UINT32		PHYMODE: 2;
    /* Word2 */
    /* ex:  1c ff 38 00 means ACK=0, BAWinSize=7, MPDUtotalByteCount = 0x38 */
    UINT32		ACK: 1;
    UINT32		NSEQ: 1;
    UINT32		BAWinSize: 6;
    UINT32		WirelessCliID: 8;
    UINT32		MPDUtotalByteCount: 12;
    UINT32		PacketId: 4;
    /*Word3 */
    UINT32		IV;
    /*Word4 */
    UINT32		EIV;
}	TXWI_STRUC, *PTXWI_STRUC;


typedef	struct {
    /* Word	0 */
    UINT32		WirelessCliID: 8;
    UINT32		KeyIndex: 2;
    UINT32		BSSID: 3;
    UINT32		UDF: 3;
    UINT32		MPDUtotalByteCount: 12;
    UINT32		TID: 4;
    /* Word	1 */
    UINT32		FRAG: 4;
    UINT32		SEQUENCE: 12;
    UINT32		MCS: 7;
    UINT32		BW: 1;
    UINT32		ShortGI: 1;
    UINT32		STBC: 2;
    UINT32		rsv: 3;
    UINT32		PHYMODE: 2;             /* 1: this RX frame is unicast to me */
    /*Word2 */
    UINT32		RSSI0: 8;
    UINT32		RSSI1: 8;
    UINT32		RSSI2: 8;
    UINT32		rsv1: 8;
    /*Word3 */
    UINT32		SNR0: 8;
    UINT32		SNR1: 8;
    UINT32		FOFFSET: 8;
    UINT32		rsv2: 8;
    /*UINT32		rsv2:16;*/
}	RXWI_STRUC, *PRXWI_STRUC;


struct urb {
    /* private: usb core and host controller only fields in the urb */
    unsigned int pipe;		/* (in) pipe information */
    int status;			/* (return) non-ISO status */
    void *transfer_buffer;		/* (in) associated data buffer */
    u32 transfer_buffer_length;	/* (in) data buffer length */
    u32 actual_length;		/* (return) actual transfer length */
    void *context;			/* (in) context for completion */
    usb_complete_t complete;	/* (in) completion routine */
};


struct product_conn {
    u8 str[16];
    u8 mode;
    u8 ssid[33];
    u8 pwd[64];
    u8 reset;
    u32 spk_value;
    u32 mic_value;
    u8 mic_res;
};


#define		NET_CAMERA_OPEN		"NET_CAMERA_OPEN"
#define		NET_CAMERA_CLOSE	"NET_CAMERA_CLOSE"
#define		NET_CAMERA_FRAME	"NET_CAMERA_FRAME"
#define		NET_SPEAKER_OPEN	"NET_SPEAKER_OPEN"
#define		NET_SPEAKER_CLOSE	"NET_SPEAKER_CLOSE"
#define		NET_SPEAKER_VOL	    "NET_SPEAKER_VOL"
#define		NET_SPEAKER_FRAME	"NET_SPEAKER_FRAME"
#define		NET_MIC_OPEN		"NET_MIC_OPEN"
#define		NET_MIC_CLOSE		"NET_MIC_CLOSE"
#define		NET_MIC_VOL	    	"NET_MIC_VOL"
#define		NET_MIC_FRAME		"NET_MIC_FRAME"


struct net_cmd_type {
    u8 cmd[32];
    u8 params[32];
};


struct camera_data_head {
    u8 mark[2];
    u8 type;
    u32 size;
};


u8 product_net_main(void);
void product_net_camera_info(u32 *width, u32 *height, u32 *fps);
u8 build_camera_data_conn(void *func, void *priv);
void delete_camera_data_conn(void);
u8 build_audio_data_conn(void);
void delete_audio_data_conn(void);
void camera_conn_init(void);
void audio_conn_init(int spk_value, int mic_value, u8 mic_res);
void product_audio_ctl_tramsmit(char *msg, u32 *param, u32 num);
int product_user_video_callback(u8 type, u8 *buf, u32 size);

#endif


