/* Copyright(C) JieLi Technology
 * usb audio 2.0
 * All right reserved
 *
 */

#include "system/includes.h"
#include "usb_config.h"
#include "usb/device/usb_stack.h"
#include "usb/device/uac_audio.h"
#include "usb/device/uac_audio_v2.h"
#include "uac_stream.h"
#include "app_config.h"

#if TCFG_USB_SLAVE_AUDIO_ENABLE

#define LOG_TAG_CONST       USB
#define LOG_TAG             "[USB]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#if (USB_AUDIO_VERSION == USB_AUDIO_VERSION_2_0)

#define SPK_CLOCK_SOURCE_ID         10
#define MIC_CLOCK_SOURCE_ID         11

//bmChannelConfig
#define UAC_V2_CHANNEL_FRONT_LEFT               BIT(0)
#define UAC_V2_CHANNEL_FRONT_RIGHT              BIT(1)
#define UAC_V2_CHANNEL_FRONT_CENTER             BIT(2)
#define UAC_V2_CHANNEL_LOW_FREQ_EFFECT          BIT(3)
#define UAC_V2_CHANNEL_BACK_LEFT                BIT(4)
#define UAC_V2_CHANNEL_BACK_RIGHT               BIT(5)
#define UAC_V2_CHANNEL_FRONT_LEFT_OF_C          BIT(6)
#define UAC_V2_CHANNEL_FRONT_RIGHT_OF_C         BIT(7)
#define UAC_V2_CHANNEL_BACK_CENTER              BIT(8)
#define UAC_V2_CHANNEL_SIDE_LEFT                BIT(9)
#define UAC_V2_CHANNEL_SIDE_RIGHT               BIT(10)
#define UAC_V2_CHANNEL_TOP_CENTER               BIT(11)
#define UAC_V2_CHANNEL_TOP_FRONT_LEFT           BIT(12)
#define UAC_V2_CHANNEL_TOP_FRONT_CENTER         BIT(13)
#define UAC_V2_CHANNEL_TOP_FRONT_RIGHT          BIT(14)
#define UAC_V2_CHANNEL_TOP_BACK_LEFT            BIT(15)
#define UAC_V2_CHANNEL_TOP_BACK_CENTER          BIT(16)
#define UAC_V2_CHANNEL_TOP_BACK_RIGHT           BIT(17)
#define UAC_V2_CHANNEL_TOP_FRONT_LEFT_OF_C      BIT(18)
#define UAC_V2_CHANNEL_TOP_FRONT_RIGHT_OF_C     BIT(19)
#define UAC_V2_CHANNEL_LEFT_LFE                 BIT(20)
#define UAC_V2_CHANNEL_RIGHT_LFE                BIT(21)
#define UAC_V2_CHANNEL_TOP_SIDE_LEFT            BIT(22)
#define UAC_V2_CHANNEL_TOP_SIDE_RIGHT           BIT(23)
#define UAC_V2_CHANNEL_BOTTOM_CENTER            BIT(24)
#define UAC_V2_CHANNEL_BACK_LEFT_OF_C           BIT(25)
#define UAC_V2_CHANNEL_BACK_RIGHT_OF_C          BIT(26)

//feature unit bmControls
#define UAC_V2_FU_CONTROL_UNDEF                 (0x00)
#define UAC_V2_FU_CONTROL_MUTE                  (0x03 << 0)
#define UAC_V2_FU_CONTROL_VOLUME                (0x03 << 2)
#define UAC_V2_FU_CONTROL_BASS                  (0x03 << 4)
#define UAC_V2_FU_CONTROL_MID                   (0x03 << 6)
#define UAC_V2_FU_CONTROL_TREBLE                (0x03 << 8)
#define UAC_V2_FU_CONTROL_EQUALIZER             (0x03 << 10)
#define UAC_V2_FU_CONTROL_AGC                   (0x03 << 12)
#define UAC_V2_FU_CONTROL_DELAY                 (0x03 << 14)
#define UAC_V2_FU_CONTROL_BASS_BOOST            (0x03 << 16)
#define UAC_V2_FU_CONTROL_LOUDNESS              (0x03 << 18)
#define UAC_V2_FU_CONTROL_INP_GAIN              (0x03 << 20)
#define UAC_V2_FU_CONTROL_INP_GAIN_PAD          (0x03 << 22)
#define UAC_V2_FU_CONTROL_PHASE_INVERT          (0x03 << 24)
#define UAC_V2_FU_CONTROL_UNDERFLOW             (0x03 << 26)
#define UAC_V2_FU_CONTROL_OVERFLOW              (0x03 << 28)

//format type
#define UAC_V2_FORMAT_TYPE_I_PCM                0x00000001
#define UAC_V2_FORMAT_TYPE_I_PCM8               0x00000002
#define UAC_V2_FORMAT_TYPE_I_IEEE_FLOAT         0x00000004
#define UAC_V2_FORMAT_TYPE_I_ALAW               0x00000008
#define UAC_V2_FORMAT_TYPE_I_MULAW              0x00000010


#if SPK_CHANNEL == 1

#define SPK_CHANNEL_CONFIG \
        UAC_DWORD( \
        UAC_V2_CHANNEL_FRONT_CENTER \
        )

#define SPK_FEATURE_CONTROLS \
        UAC_DWORD(UAC_V2_FU_CONTROL_MUTE), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME)

#elif SPK_CHANNEL == 2

#define SPK_CHANNEL_CONFIG \
        UAC_DWORD( \
        UAC_V2_CHANNEL_FRONT_LEFT | \
        UAC_V2_CHANNEL_FRONT_RIGHT \
        )

#define SPK_FEATURE_CONTROLS \
        UAC_DWORD(UAC_V2_FU_CONTROL_MUTE), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME)

#elif SPK_CHANNEL == 3  //2.1

#define SPK_CHANNEL_CONFIG \
        UAC_DWORD( \
        UAC_V2_CHANNEL_FRONT_LEFT | \
        UAC_V2_CHANNEL_FRONT_RIGHT | \
        UAC_V2_CHANNEL_LOW_FREQ_EFFECT \
        )

#define SPK_FEATURE_CONTROLS \
        UAC_DWORD(UAC_V2_FU_CONTROL_MUTE), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME)

#elif SPK_CHANNEL == 4

#define SPK_CHANNEL_CONFIG \
        UAC_DWORD( \
        UAC_V2_CHANNEL_FRONT_LEFT | \
        UAC_V2_CHANNEL_FRONT_RIGHT | \
        UAC_V2_CHANNEL_BACK_LEFT | \
        UAC_V2_CHANNEL_BACK_RIGHT \
        )

#define SPK_FEATURE_CONTROLS \
        UAC_DWORD(UAC_V2_FU_CONTROL_MUTE), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME)

#elif SPK_CHANNEL == 5

#define SPK_CHANNEL_CONFIG \
        UAC_DWORD( \
        UAC_V2_CHANNEL_FRONT_LEFT | \
        UAC_V2_CHANNEL_FRONT_RIGHT | \
        UAC_V2_CHANNEL_FRONT_CENTER | \
        UAC_V2_CHANNEL_SIDE_LEFT | \
        UAC_V2_CHANNEL_SIDE_RIGHT \
        )

#define SPK_FEATURE_CONTROLS \
        UAC_DWORD(UAC_V2_FU_CONTROL_MUTE), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME)

#elif SPK_CHANNEL == 6  //5.1

#define SPK_CHANNEL_CONFIG \
        UAC_DWORD( \
        UAC_V2_CHANNEL_FRONT_LEFT | \
        UAC_V2_CHANNEL_FRONT_RIGHT | \
        UAC_V2_CHANNEL_FRONT_CENTER | \
        UAC_V2_CHANNEL_LOW_FREQ_EFFECT | \
        UAC_V2_CHANNEL_SIDE_LEFT | \
        UAC_V2_CHANNEL_SIDE_RIGHT \
        )

#define SPK_FEATURE_CONTROLS \
        UAC_DWORD(UAC_V2_FU_CONTROL_MUTE), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME)

#elif SPK_CHANNEL == 7

#define SPK_CHANNEL_CONFIG \
        UAC_DWORD( \
        UAC_V2_CHANNEL_FRONT_LEFT | \
        UAC_V2_CHANNEL_FRONT_RIGHT | \
        UAC_V2_CHANNEL_FRONT_CENTER | \
        UAC_V2_CHANNEL_BACK_LEFT | \
        UAC_V2_CHANNEL_BACK_RIGHT | \
        UAC_V2_CHANNEL_SIDE_LEFT | \
        UAC_V2_CHANNEL_SIDE_RIGHT \
        )

#define SPK_FEATURE_CONTROLS \
        UAC_DWORD(UAC_V2_FU_CONTROL_MUTE), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME)

#elif SPK_CHANNEL == 8  //7.1

#define SPK_CHANNEL_CONFIG \
        UAC_DWORD( \
        UAC_V2_CHANNEL_FRONT_LEFT | \
        UAC_V2_CHANNEL_FRONT_RIGHT | \
        UAC_V2_CHANNEL_FRONT_CENTER | \
        UAC_V2_CHANNEL_LOW_FREQ_EFFECT | \
        UAC_V2_CHANNEL_BACK_LEFT | \
        UAC_V2_CHANNEL_BACK_RIGHT | \
        UAC_V2_CHANNEL_SIDE_LEFT | \
        UAC_V2_CHANNEL_SIDE_RIGHT \
        )

#define SPK_FEATURE_CONTROLS \
        UAC_DWORD(UAC_V2_FU_CONTROL_MUTE), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(UAC_V2_FU_CONTROL_VOLUME)

#else
#error "unsupport usb speaker channel number"
#endif


#if MIC_CHANNEL == 1

#define MIC_CHANNEL_CONFIG \
        UAC_DWORD( \
        0 \
        )

#define MIC_FEATURE_CONTROLS \
        UAC_DWORD(UAC_V2_FU_CONTROL_MUTE | UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(0)

#elif MIC_CHANNEL == 2

#define MIC_CHANNEL_CONFIG \
        UAC_DWORD( \
        UAC_V2_CHANNEL_FRONT_LEFT | \
        UAC_V2_CHANNEL_FRONT_RIGHT \
        )

#define MIC_FEATURE_CONTROLS \
        UAC_DWORD(UAC_V2_FU_CONTROL_MUTE | UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(0), \
        UAC_DWORD(0)

#elif MIC_CHANNEL == 3  //2.1

#define MIC_CHANNEL_CONFIG \
        UAC_DWORD( \
        UAC_V2_CHANNEL_FRONT_LEFT | \
        UAC_V2_CHANNEL_FRONT_RIGHT | \
        UAC_V2_CHANNEL_LOW_FREQ_EFFECT \
        )

#define MIC_FEATURE_CONTROLS \
        UAC_DWORD(UAC_V2_FU_CONTROL_MUTE | UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(0), \
        UAC_DWORD(0), \
        UAC_DWORD(0)

#elif MIC_CHANNEL == 4

#define MIC_CHANNEL_CONFIG \
        UAC_DWORD( \
        UAC_V2_CHANNEL_FRONT_LEFT | \
        UAC_V2_CHANNEL_FRONT_RIGHT | \
        UAC_V2_CHANNEL_BACK_LEFT | \
        UAC_V2_CHANNEL_BACK_RIGHT \
        )

#define MIC_FEATURE_CONTROLS \
        UAC_DWORD(UAC_V2_FU_CONTROL_MUTE | UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(0), \
        UAC_DWORD(0), \
        UAC_DWORD(0), \
        UAC_DWORD(0)

#elif MIC_CHANNEL == 5

#define MIC_CHANNEL_CONFIG \
        UAC_DWORD( \
        UAC_V2_CHANNEL_FRONT_LEFT | \
        UAC_V2_CHANNEL_FRONT_RIGHT | \
        UAC_V2_CHANNEL_FRONT_CENTER | \
        UAC_V2_CHANNEL_SIDE_LEFT | \
        UAC_V2_CHANNEL_SIDE_RIGHT \
        )

#define MIC_FEATURE_CONTROLS \
        UAC_DWORD(UAC_V2_FU_CONTROL_MUTE | UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(0), \
        UAC_DWORD(0), \
        UAC_DWORD(0), \
        UAC_DWORD(0), \
        UAC_DWORD(0)

#elif MIC_CHANNEL == 6  //5.1

#define MIC_CHANNEL_CONFIG \
        UAC_DWORD( \
        UAC_V2_CHANNEL_FRONT_LEFT | \
        UAC_V2_CHANNEL_FRONT_RIGHT | \
        UAC_V2_CHANNEL_FRONT_CENTER | \
        UAC_V2_CHANNEL_LOW_FREQ_EFFECT | \
        UAC_V2_CHANNEL_SIDE_LEFT | \
        UAC_V2_CHANNEL_SIDE_RIGHT \
        )

#define MIC_FEATURE_CONTROLS \
        UAC_DWORD(UAC_V2_FU_CONTROL_MUTE | UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(0), \
        UAC_DWORD(0), \
        UAC_DWORD(0), \
        UAC_DWORD(0), \
        UAC_DWORD(0), \
        UAC_DWORD(0)

#elif MIC_CHANNEL == 7

#define MIC_CHANNEL_CONFIG \
        UAC_DWORD( \
        UAC_V2_CHANNEL_FRONT_LEFT | \
        UAC_V2_CHANNEL_FRONT_RIGHT | \
        UAC_V2_CHANNEL_FRONT_CENTER | \
        UAC_V2_CHANNEL_BACK_LEFT | \
        UAC_V2_CHANNEL_BACK_RIGHT | \
        UAC_V2_CHANNEL_SIDE_LEFT | \
        UAC_V2_CHANNEL_SIDE_RIGHT \
        )

#define MIC_FEATURE_CONTROLS \
        UAC_DWORD(UAC_V2_FU_CONTROL_MUTE | UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(0), \
        UAC_DWORD(0), \
        UAC_DWORD(0), \
        UAC_DWORD(0), \
        UAC_DWORD(0), \
        UAC_DWORD(0), \
        UAC_DWORD(0)

#elif MIC_CHANNEL == 8  //7.1

#define MIC_CHANNEL_CONFIG \
        UAC_DWORD( \
        UAC_V2_CHANNEL_FRONT_LEFT | \
        UAC_V2_CHANNEL_FRONT_RIGHT | \
        UAC_V2_CHANNEL_FRONT_CENTER | \
        UAC_V2_CHANNEL_LOW_FREQ_EFFECT | \
        UAC_V2_CHANNEL_BACK_LEFT | \
        UAC_V2_CHANNEL_BACK_RIGHT | \
        UAC_V2_CHANNEL_SIDE_LEFT | \
        UAC_V2_CHANNEL_SIDE_RIGHT \
        )

#define MIC_FEATURE_CONTROLS \
        UAC_DWORD(UAC_V2_FU_CONTROL_MUTE | UAC_V2_FU_CONTROL_VOLUME), \
        UAC_DWORD(0), \
        UAC_DWORD(0), \
        UAC_DWORD(0), \
        UAC_DWORD(0), \
        UAC_DWORD(0), \
        UAC_DWORD(0), \
        UAC_DWORD(0), \
        UAC_DWORD(0)

#else
#error "unsupport usb mic channel number"
#endif



/*********************************************************/
/*
			   Audio Class
*/
/*********************************************************/
///Standard Interface Association Descriptor
static const u8 uac_audio_interface_association[] = {
    USB_DT_INTERFACE_ASSOCIATION_SIZE, //Size of this descriptor in bytes: 8
    USB_DT_INTERFACE_ASSOCIATION,//INTERFACE ASSOCIATION Descriptor
    0,//Interface number of the first interface that is associated with this function
    0,//Number of contiguous interfaces that are associated with this function.
    USB_CLASS_AUDIO,//AUDIO_FUNCTION Function Class code
    0,//FUNCTION_SUBCLASS_UNDEFINED
    UAC_VERSION_2,//AF_VERSION_02_00 Function Protocol code.
    SPEAKER_STR_INDEX,//Index of a string descriptor that describes this interface.
};
static const u8 uac_ac_standard_interface_desc[] = {
///standard interface AC descriptor(Interface 0, Alternate Setting 0)
    USB_DT_INTERFACE_SIZE,    //Length
    USB_DT_INTERFACE,     //DescriptorType:Inerface
    0x00,     //InterfaceNum
    0x00,       //AlternateSetting
    0x00,       //NumEndpoint
    USB_CLASS_AUDIO,       //InterfaceClass:audio
    USB_SUBCLASS_AUDIOCONTROL,       //InterfaceSubClass:audio ctl
    UAC_VERSION_2,       //InterfaceProtocol
    SPEAKER_STR_INDEX,      //Interface String
};

static const u8 uac_spk_ac_interface[] = {
///class-specific AC interface descriptor
    0x09,    //Length
    USB_DT_CS_INTERFACE,      //DescriptorType:audio interface descriptor
    UAC_HEADER,     //DescriptorSubType:audio control header
    UAC_WORD(0x0200), //bcdADC:audio Device Class v2.00
    0x00,  //bCategory:
    UAC_WORD(0x0000), //TotalLength of class-specific AC interface descriptors
    0x00,  //bmControls
};
static const u8 uac_audio_ac_interface[] = {
///class-specific AC interface descriptor
    0x09,    //Length
    USB_DT_CS_INTERFACE,      //DescriptorType:audio interface descriptor
    UAC_HEADER,     //DescriptorSubType:audio control header
    UAC_WORD(0x0200), //bcdADC:audio Device Class v2.00
    0x00,  //bCategory:
    UAC_WORD(0x0000), //TotalLength of class-specific AC interface descriptors
    0x00,  //bmControls
};
static const u8 uac_spk_clock_source_desc[] = {
    0x08,                           /* bLength */
    USB_DT_CS_INTERFACE,            /* bDescriptorType */
    UAC2_CLOCK_SOURCE,              /* bDescriptorSubtype */
    SPK_CLOCK_SOURCE_ID,            /* bClockID */
    0x03,                           /* bmAttributes */
    0x07,                           /* bmControls */
    0x00,                           /* bAssocTerminal */
    0x00                            /* iClockSource */
};
static const u8 uac_spk_input_terminal_desc[] = {

///USB USB Streaming IT(Speaker)
    0x11,       //Length
    USB_DT_CS_INTERFACE,       //DescriptorType:audio interface descriptor
    UAC_INPUT_TERMINAL,       //DescriptorSubType:Input Terminal
    SPK_INPUT_TERMINAL_ID,       //TerminalID
    UAC_WORD(UAC_TERMINAL_STREAMING), //TerminalType:USB Streaming
    0x00,       //AssocTerminal
    SPK_CLOCK_SOURCE_ID,  //bCSourceID: ID of the Clock Entity to which this Input Terminal is connected
    SPK_CHANNEL,       //NrChannels:2 channel
    SPK_CHANNEL_CONFIG,
    0x00,       //ChannelName String
    UAC_WORD(0x0000), //bmControls
    0x00,       //Terminal String
};
///Audio Feature Unit Descriptor(Speaker)
static const u8 uac_spk_feature_desc[] = {
    0x06 + (SPK_CHANNEL + 1) * 4, //Length
    USB_DT_CS_INTERFACE,      //DescriptorType:audio interface descriptor
    UAC_FEATURE_UNIT,       //DescriptorSubType:Audio Feature Unit
    SPK_FEATURE_UNIT_ID,       //UnitID
    SPK_INPUT_TERMINAL_ID,      //SourceID:1 #USB Streaming IT
    SPK_FEATURE_CONTROLS,
    0x00,       //Feature String
};
//SELECTOR_UNIT
static const u8 uac_spk_selector_uint_desc[] = {
    0x8,    //  bLength	7
    USB_DT_CS_INTERFACE,//  bDescriptorType	CS_INTERFACE (0x24)
    UAC_SELECTOR_UNIT,//    bDescriptorSubtype	SELECTOR_UNIT (0x05)
    SPK_SELECTOR_UNIT_ID,//bUnitID	33
    1,//bNrInPins	1
    SPK_FEATURE_UNIT_ID,   //baSourceID(1)	50
    0x00,  //D0..1: Selector Control
    0,//iSelector	None (0)
};
static const u8 uac_spk_output_terminal_desc[] = {
///USB Speaker OT
    0x0c,      //Length
    USB_DT_CS_INTERFACE,      //DescriptorType:audio interface descriptor
    UAC_OUTPUT_TERMINAL,      //DescriptorSubTYpe:Output Terminal
    SPK_OUTPUT_TERMINAL_ID,      //TerminalID
    UAC_WORD(UAC_OUTPUT_TERMINAL_SPEAKER), //TerminalType:Speaker
    0,       //AssocTerminal:
    SPK_FEATURE_UNIT_ID,      //SourceID: Feature UNIT
    //SPK_SELECTOR_UNIT_ID,      //SourceID: Selector UNIT
    SPK_CLOCK_SOURCE_ID,  //bCSourceID: ID of the Clock Entity to which this Input Terminal is connected
    UAC_WORD(0x0000), //bmControls
    0x00,      //Terminal String
};

static const u8 uac_spk_as_interface_desc[] = {
//-------------------Speaker  interface---------------------//
///standard interface AS descriptor(Interface 1, Alternate Setting 0)
    USB_DT_INTERFACE_SIZE,      //Length
    USB_DT_INTERFACE,      //DescriptorType:Interface
    0x00,       //InterfaceNum:2
    0x00,       //AlternateSetting:0
    0x00,      //NumEndpoint:0
    USB_CLASS_AUDIO,       //InterfaceClass:audio
    USB_SUBCLASS_AUDIOSTREAMING,      //InterfaceSubClass:audio streaming
    UAC_VERSION_2,      //InterfaceProtocol
    0x00,       //Interface String

///standard interface AS descriptor(Interface 2, Alternate Setting 1)
    USB_DT_INTERFACE_SIZE,      //Length
    USB_DT_INTERFACE,      //DescriptorType:Interface
    0x00,       //InterfaceNum
    0x01,       //AlternateSetting
    0x01,       //NumEndpoint
    USB_CLASS_AUDIO,      //InterfaceClass:audio
    USB_SUBCLASS_AUDIOSTREAMING,       //InterfaceSubClass:audio streaming
    UAC_VERSION_2,       //InterfaceProtocol
    0x00,      //Interface String

///Audio Streaming Interface Descriptor:AS_GENERAL(0x01),
    0x10,      //Length
    USB_DT_CS_INTERFACE,       //DescriptorType:audio interface descriptor
    UAC_AS_GENERAL,       //DescriptorSubType:AS_GENERAL
    SPK_INPUT_TERMINAL_ID,      //TerminalLink:#1 USB Streaming IT
    0x05, //bmControls, Active Alternate Setting, Valid Alternate Setting
    UAC_FORMAT_TYPE_I,       //FormatType:Format type 1
    UAC_DWORD(UAC_V2_FORMAT_TYPE_I_PCM),  //bmFormat
    SPK_CHANNEL, //bNrChannels
    SPK_CHANNEL_CONFIG,
    0, //iChannelNames

///Type 1 Format type descriptor
    0x06,     //Length
    USB_DT_CS_INTERFACE,       //DescriptorType:audio interface descriptor
    UAC_FORMAT_TYPE,      //DescriptorSubType:Format_type
    UAC_FORMAT_TYPE_I,       //FormatType:Format type 1
#if SPK_AUDIO_RES == 24 && UAC_24BIT_IN_4BYTE
    4,                       //SubframeSize:2byte
#else
    SPK_AUDIO_RES / 8,       //SubframeSize:2byte
#endif
    SPK_AUDIO_RES,      //BitsResolution:16bit

//Isochronous,Synchronization Type(Asynchronous)
    0x07,      //Length
    USB_DT_ENDPOINT,       //DescriptorType:endpoint descriptor
    USB_DIR_OUT | SPK_ISO_EP_OUT,     //EndpointAddress:Output endpoint 1
    USB_ENDPOINT_XFER_ISOC | USB_ENDPOINT_SYNC_ADAPTIVE, //0x09,
    UAC_WORD(SPK_FRAME_LEN), //MaxPacketSize
    UAC_ISO_INTERVAL,//one packet per frame 0x00,  //Interval    //Asynchronous

//Audio Endpoint descriptor,General
    0x08,       //Length
    USB_DT_CS_ENDPOINT,       //DescriptorType:audio endpoint descriptor
    UAC_AS_GENERAL,       //DescriptorSubType:audio endpiont general
    0x00,       //bmAttributes
    0x00,       //bmControls
    0x00,       //LockDelayUnits
    0x00, 0x00, //LockDelay

#if SPK_AUDIO_RES_2
///standard interface AS descriptor(Interface 2, Alternate Setting 2)
    USB_DT_INTERFACE_SIZE,      //Length
    USB_DT_INTERFACE,      //DescriptorType:Interface
    0x00,       //InterfaceNum
    0x02,       //AlternateSetting
    0x01,       //NumEndpoint
    USB_CLASS_AUDIO,      //InterfaceClass:audio
    USB_SUBCLASS_AUDIOSTREAMING,       //InterfaceSubClass:audio streaming
    UAC_VERSION_2,       //InterfaceProtocol
    0x00,      //Interface String

///Audio Streaming Interface Descriptor:AS_GENERAL(0x01),
    0x10,      //Length
    USB_DT_CS_INTERFACE,       //DescriptorType:audio interface descriptor
    UAC_AS_GENERAL,       //DescriptorSubType:AS_GENERAL
    SPK_INPUT_TERMINAL_ID,      //TerminalLink:#1 USB Streaming IT
    0x05, //bmControls, Active Alternate Setting, Valid Alternate Setting
    UAC_FORMAT_TYPE_I,       //FormatType:Format type 1
    UAC_DWORD(UAC_V2_FORMAT_TYPE_I_PCM),  //bmFormat
    SPK_CHANNEL, //bNrChannels
    SPK_CHANNEL_CONFIG,
    0, //iChannelNames

///Type 1 Format type descriptor
    0x06,     //Length
    USB_DT_CS_INTERFACE,       //DescriptorType:audio interface descriptor
    UAC_FORMAT_TYPE,      //DescriptorSubType:Format_type
    UAC_FORMAT_TYPE_I,       //FormatType:Format type 1
#if SPK_AUDIO_RES_2 == 24 && UAC_24BIT_IN_4BYTE
    4,                         //SubframeSize:2byte
#else
    SPK_AUDIO_RES_2 / 8,       //SubframeSize:2byte
#endif
    SPK_AUDIO_RES_2,      //BitsResolution:16bit

//Isochronous,Synchronization Type(Asynchronous)
    0x07,      //Length
    USB_DT_ENDPOINT,       //DescriptorType:endpoint descriptor
    USB_DIR_OUT | SPK_ISO_EP_OUT,     //EndpointAddress:Output endpoint 1
    USB_ENDPOINT_XFER_ISOC | USB_ENDPOINT_SYNC_ADAPTIVE, //0x09,
    UAC_WORD(SPK_FRAME_LEN), //MaxPacketSize
    UAC_ISO_INTERVAL,//one packet per frame 0x00,  //Interval    //Asynchronous

//Audio Endpoint descriptor,General
    0x08,       //Length
    USB_DT_CS_ENDPOINT,       //DescriptorType:audio endpoint descriptor
    UAC_AS_GENERAL,       //DescriptorSubType:audio endpiont general
    0x00,      //bmAttributes
    0x00,       //bmControls
    0x00,       //LockDelayUnits
    0x00, 0x00, //LockDelay
#endif
};

const u8 speakerStringDescriptor[20] = {
    20,         //该描述符的长度为20字节
    0x03,       //字符串描述符的类型编码为0x03
    'U', 0x00, //U
    'S', 0x00, //S
    'B', 0x00, //B
    ' ', 0x00, //
    'A', 0x00, //A
    'u', 0x00, //u
    'd', 0x00, //d
    'i', 0x00, //i
    'o', 0x00, //o
};
/*********************************************************/
/*
			   Microphone Class
*/
/*********************************************************/

static const u8 uac_mic_ac_interface[] = {
///class-specific AC interface descriptor
    0x09,    //Length
    USB_DT_CS_INTERFACE,      //DescriptorType
    UAC_HEADER,     //DescriptorSubType
    UAC_WORD(0x0200), //bcdADC
    0x00,  //bCategory:
    UAC_WORD(0x0000), /*TotalLength of class-specific AC interface descriptors */
    0x00,  //bmControls
};
static const u8 uac_mic_clock_source_desc[] = {
    0x08,                           /* bLength */
    USB_DT_CS_INTERFACE,            /* bDescriptorType */
    UAC2_CLOCK_SOURCE,              /* bDescriptorSubtype */
    MIC_CLOCK_SOURCE_ID,            /* bClockID */
    0x03,                           /* bmAttributes */
    0x07,                           /* bmControls */
    0x00,                           /* bAssocTerminal */
    0x00                            /* iClockSource */
};
///USB USB Streaming IT(Microphone)
static const u8  uac_mic_input_terminal_desc[] = {
    0x11,       //Length
    USB_DT_CS_INTERFACE,       //DescriptorType:audio interface descriptor
    UAC_INPUT_TERMINAL,       //DescriptorSubType:Input Terminal
    MIC_INPUT_TERMINAL_ID,       //TerminalID
    UAC_WORD(UAC_INPUT_TERMINAL_MICROPHONE), //TerminalType:Microphone
    0,       //AssocTerminal
    MIC_CLOCK_SOURCE_ID,  //bCSourceID: ID of the Clock Entity to which this Input Terminal is connected
    MIC_CHANNEL,       //NrChannels
    MIC_CHANNEL_CONFIG,
    0x00,       //ChannelName String
    UAC_WORD(0x0000), //bmControls
    0x00,       //Terminal String
};
//SELECTOR_UNIT
static const u8 uac_mic_selector_uint_desc[] = {
    0x8,    //  bLength	7
    USB_DT_CS_INTERFACE,//  bDescriptorType	CS_INTERFACE (0x24)
    UAC_SELECTOR_UNIT,//    bDescriptorSubtype	SELECTOR_UNIT (0x05)
    MIC_SELECTOR_UNIT_ID,//bUnitID	33
    1,//bNrInPins	1
    MIC_FEATURE_UNIT_ID,   //baSourceID(1)	50
    0x03,  //D0..1: Selector Control
    0,//iSelector	None (0)
};

///Audio Feature Unit Descriptor(Microphone)
static const u8 uac_mic_feature_desc[] = {
    6 + (MIC_CHANNEL + 1) * 4,    	//Length
    USB_DT_CS_INTERFACE,      //DescriptorType:audio interface descriptor
    UAC_FEATURE_UNIT,       //DescriptorSubType:Audio Feature Unit
    MIC_FEATURE_UNIT_ID,       //UnitID
    MIC_INPUT_TERMINAL_ID,      //SourceID: #Microphone
    MIC_FEATURE_CONTROLS,
    0x00,       //Feature String
};
///USB Microphone OT
static const u8  uac_mic_output_terminal_desc[] = {
    0x0c,      //Length
    USB_DT_CS_INTERFACE,      //DescriptorType:audio interface descriptor
    UAC_OUTPUT_TERMINAL,      //DescriptorSubTYpe:Output Terminal
    MIC_OUTPUT_TERMINAL_ID,      //TerminalID
    UAC_WORD(UAC_TERMINAL_STREAMING), //TerminalType:USB Sreaming
    0,       //AssocTerminal:
    MIC_FEATURE_UNIT_ID,      //SourceID:A #Feature UNIT
    //MIC_SELECTOR_UNIT_ID,      //SourceID:A #Feature UNIT
    MIC_CLOCK_SOURCE_ID,  //bCSourceID: ID of the Clock Entity to which this Input Terminal is connected
    UAC_WORD(0x0000), //bmControls
    0x00,      //Terminal String
};

///standard interface AS descriptor(Interface 1, Alternate Setting 0)
static const u8 uac_mic_as_interface_desc[] = {
    USB_DT_INTERFACE_SIZE,      //Length
    USB_DT_INTERFACE,      //DescriptorType:Interface
    0x00,       //InterfaceNum ****
    0x00,       //AlternateSetting
    0x00,      //NumEndpoint
    USB_CLASS_AUDIO,       //InterfaceClass:audio
    USB_SUBCLASS_AUDIOSTREAMING,      //InterfaceSubClass:audio streaming
    UAC_VERSION_2,      //InterfaceProtocol
    0x00,       //Interface String

///standard interface AS descriptor(Interface 2, Alternate Setting 1)
    USB_DT_INTERFACE_SIZE,      //Length
    USB_DT_INTERFACE,      //DescriptorType:Interface
    0x00,       //InterfaceNum ****
    0x01,       //AlternateSetting
    0x01,       //NumEndpoint
    USB_CLASS_AUDIO,      //InterfaceClass:audio
    USB_SUBCLASS_AUDIOSTREAMING,       //InterfaceSubClass:audio streaming
    UAC_VERSION_2,       //InterfaceProtocol
    0x00,      //Interface String

///Audio Streaming Interface Descriptor:AS_GENERAL
    0x10,      //Length
    USB_DT_CS_INTERFACE,       //DescriptorType:audio interface descriptor
    UAC_AS_GENERAL,       //DescriptorSubType:AS_GENERAL
    MIC_OUTPUT_TERMINAL_ID,      //TerminalLink:#1 USB Streaming OT
    0x05, //bmControls, Active Alternate Setting, Valid Alternate Setting
    UAC_FORMAT_TYPE_I,       //FormatType:Format type 1
    UAC_DWORD(UAC_V2_FORMAT_TYPE_I_PCM),  //bmFormat
    MIC_CHANNEL, //bNrChannels
    MIC_CHANNEL_CONFIG,
    0, //iChannelNames

///Type 1 Format type descriptor
    0x06, //Length
    USB_DT_CS_INTERFACE,       //DescriptorType:audio interface descriptor
    UAC_FORMAT_TYPE,      //DescriptorSubType:Format_type
    UAC_FORMAT_TYPE_I,       //FormatType:Format type 1
#if MIC_AUDIO_RES == 24 && UAC_24BIT_IN_4BYTE
    4,                 //SubframeSize:2byte
#else
    MIC_AUDIO_RES / 8, //SubframeSize:2byte
#endif
    MIC_AUDIO_RES,     //BitsResolution:16bit

///Endpoint 1 - Standard Descriptor:Output Endpoint1
//Isochronous,Synchronization Type(Asynchronous)
    0x07,      //Length
    USB_DT_ENDPOINT,       //DescriptorType:endpoint descriptor
    USB_DIR_IN | MIC_ISO_EP_IN,       //EndpointAddress:Output endpoint
    USB_ENDPOINT_XFER_ISOC | USB_ENDPOINT_SYNC_ASYNC,
    UAC_WORD(MIC_FRAME_LEN), //MaxPacketSize
    UAC_ISO_INTERVAL,//one packet per frame   //Interval

///Endpoint - Audio Streaming Descriptor
//Audio Endpoint descriptor,General
    0x08,       //Length
    USB_DT_CS_ENDPOINT,       //DescriptorType:audio endpoint descriptor
    UAC_AS_GENERAL,       //DescriptorSubType:audio endpiont general
    0x00,      //bmAttributes
    0x00,      //bmControls
    0x00,       //LockDelayUnits
    0x00, 0x00, //LockDelay

#if MIC_AUDIO_RES_2
///standard interface AS descriptor(Interface 2, Alternate Setting 2)
    USB_DT_INTERFACE_SIZE,      //Length
    USB_DT_INTERFACE,      //DescriptorType:Interface
    0x00,       //InterfaceNum ****
    0x02,       //AlternateSetting
    0x01,       //NumEndpoint
    USB_CLASS_AUDIO,      //InterfaceClass:audio
    USB_SUBCLASS_AUDIOSTREAMING,       //InterfaceSubClass:audio streaming
    UAC_VERSION_2,       //InterfaceProtocol
    0x00,      //Interface String

///Audio Streaming Interface Descriptor:AS_GENERAL
    0x10,      //Length
    USB_DT_CS_INTERFACE,       //DescriptorType:audio interface descriptor
    UAC_AS_GENERAL,       //DescriptorSubType:AS_GENERAL
    MIC_OUTPUT_TERMINAL_ID,      //TerminalLink:#1 USB Streaming OT
    0x05, //bmControls, Active Alternate Setting, Valid Alternate Setting
    UAC_FORMAT_TYPE_I,       //FormatType:Format type 1
    UAC_DWORD(UAC_V2_FORMAT_TYPE_I_PCM),  //bmFormat
    MIC_CHANNEL, //bNrChannels
    MIC_CHANNEL_CONFIG,
    0, //iChannelNames

///Type 1 Format type descriptor
    0x06, //Length
    USB_DT_CS_INTERFACE,       //DescriptorType:audio interface descriptor
    UAC_FORMAT_TYPE,      //DescriptorSubType:Format_type
    UAC_FORMAT_TYPE_I,       //FormatType:Format type 1
#if MIC_AUDIO_RES_2 == 24 && UAC_24BIT_IN_4BYTE
    4,                   //SubframeSize:2byte
#else
    MIC_AUDIO_RES_2 / 8, //SubframeSize:2byte
#endif
    MIC_AUDIO_RES_2,     //BitsResolution:16bit

///Endpoint 1 - Standard Descriptor:Output Endpoint1
//Isochronous,Synchronization Type(Asynchronous)
    0x07,      //Length
    USB_DT_ENDPOINT,       //DescriptorType:endpoint descriptor
    USB_DIR_IN | MIC_ISO_EP_IN,       //EndpointAddress:Output endpoint
    USB_ENDPOINT_XFER_ISOC | USB_ENDPOINT_SYNC_ASYNC,
    UAC_WORD(MIC_FRAME_LEN), //MaxPacketSize
    UAC_ISO_INTERVAL,//one packet per frame   //Interval

///Endpoint - Audio Streaming Descriptor
//Audio Endpoint descriptor,General
    0x08,       //Length
    USB_DT_CS_ENDPOINT,       //DescriptorType:audio endpoint descriptor
    UAC_AS_GENERAL,       //DescriptorSubType:audio endpiont general
    0x00,      //bmAttributes
    0x00,      //bmControls
    0x00,       //LockDelayUnits
    0x00, 0x00, //LockDelay
#endif
};
static const u8 micStringDescriptor[30] = {
    30,         //该描述符的长度为30字节
    0x03,       //字符串描述符的类型编码为0x03
    'U', 0x00, //U
    'S', 0x00, //S
    'B', 0x00, //B
    ' ', 0x00, //
    'M', 0x00, //M
    'i', 0x00, //i
    'c', 0x00, //c
    'r', 0x00, //r
    'o', 0x00, //o
    'p', 0x00, //p
    'h', 0x00, //h
    'o', 0x00, //o
    'n', 0x00, //n
    'e', 0x00, //e
};


struct spk_info_t {
    u16 spk_left_vol;
    u16 spk_right_vol;
    u16 spk_max_vol;
    u16 spk_min_vol;
    u16 spk_def_vol;
    u16 spk_vol_res;
    u8 *spk_dma_buffer;
    u32 spk_sampling_rate;
    u16 spk_padding_interval;
    u16 spk_padding_cnt;
    u16 res;
    u16 sof_cnt;
    u8  spk_mute;
    u8  ps4_mode;
    u8  alternate_setting;
    u8  downstream_halt;
};

struct mic_info_t {
    u16 mic_max_vol;
    u16 mic_min_vol;
    u16 mic_def_vol;
    u16 mic_vol_res;
    u16 mic_vol;
    u8 *mic_dma_buffer;
    u32 mic_sampling_rate;
    u16 mic_padding_interval;
    u16 mic_padding_cnt;
    u16 res;
    u16 sof_cnt;
    u8  mic_mute;
    u8  bAGC;
    u8  ps4_mode;
    u8  alternate_setting;
    u8  mic_no_data;
    u8  upstream_halt;
};

struct spk_info_t *spk_info;
struct mic_info_t *mic_info;
#if USB_MALLOC_ENABLE
#else
struct spk_info_t _spk_info SEC(.usb.data.bss.exchange);
struct mic_info_t _mic_info SEC(.usb.data.bss.exchange);
#endif

static void open_spk(struct usb_device_t *usb_device);
static void open_mic(struct usb_device_t *usb_device);

#define AUDIO_SAMPLE_FREQ_NUM(num) \
    DW1BYTE(num), DW2BYTE(num)

#define AUDIO_SAMPLE_FREQ_4B(freq) \
    DW1BYTE(freq), DW2BYTE(freq), DW3BYTE(freq), DW4BYTE(freq)


static void close_mic(struct usb_device_t *usb_device);
void audio_reset(struct usb_device_t *usb_device, u32 ift_num);
static u32 uac_vol_handler(struct usb_device_t *usb_device, struct usb_ctrlrequest *setup);


static const u8 speaker_default_sampling_freq_table[] = {
    AUDIO_SAMPLE_FREQ_NUM(SPK_AUDIO_RATE_NUM),
#if SPK_AUDIO_RATE_NUM > 1
    AUDIO_SAMPLE_FREQ_4B(SPK_AUDIO_RATE_1),     //MIN
    AUDIO_SAMPLE_FREQ_4B(SPK_AUDIO_RATE_1),     //MAX
    AUDIO_SAMPLE_FREQ_4B(0x00),                 //RES
    AUDIO_SAMPLE_FREQ_4B(SPK_AUDIO_RATE_2),
    AUDIO_SAMPLE_FREQ_4B(SPK_AUDIO_RATE_2),
    AUDIO_SAMPLE_FREQ_4B(0x00),
    AUDIO_SAMPLE_FREQ_4B(SPK_AUDIO_RATE),
    AUDIO_SAMPLE_FREQ_4B(SPK_AUDIO_RATE),
    AUDIO_SAMPLE_FREQ_4B(0x00),
#else
    AUDIO_SAMPLE_FREQ_4B(SPK_AUDIO_RATE),
    AUDIO_SAMPLE_FREQ_4B(SPK_AUDIO_RATE),
    AUDIO_SAMPLE_FREQ_4B(0x00),
#endif
};

static const u8 mic_default_sampling_freq_table[] = {
    AUDIO_SAMPLE_FREQ_NUM(MIC_SamplingFrequency),
#if MIC_SamplingFrequency > 1
    AUDIO_SAMPLE_FREQ_4B(MIC_AUDIO_RATE_1),     //MIN
    AUDIO_SAMPLE_FREQ_4B(MIC_AUDIO_RATE_1),     //MAX
    AUDIO_SAMPLE_FREQ_4B(0x00),                 //RES
    AUDIO_SAMPLE_FREQ_4B(MIC_AUDIO_RATE_2),
    AUDIO_SAMPLE_FREQ_4B(MIC_AUDIO_RATE_2),
    AUDIO_SAMPLE_FREQ_4B(0x00),
    AUDIO_SAMPLE_FREQ_4B(MIC_AUDIO_RATE),
    AUDIO_SAMPLE_FREQ_4B(MIC_AUDIO_RATE),
    AUDIO_SAMPLE_FREQ_4B(0x00),
#else
    AUDIO_SAMPLE_FREQ_4B(MIC_AUDIO_RATE),
    AUDIO_SAMPLE_FREQ_4B(MIC_AUDIO_RATE),
    AUDIO_SAMPLE_FREQ_4B(0x00),
#endif
};

static u32 vol_convert(u16 v)
{
    const u16 vol_table[] = {
        0xe3a0, 0xe461, 0xe519, 0xe5c8, 0xe670, 0xe711, 0xe7ac, 0xe840, 0xe8cf, 0xe959,
        0xe9df, 0xea60, 0xeadc, 0xeb55, 0xebca, 0xec3c, 0xecab, 0xed16, 0xed7f, 0xede5,
        0xee48, 0xeea9, 0xef08, 0xef64, 0xefbe, 0xf016, 0xf06c, 0xf0c0, 0xf113, 0xf164,
        0xf1b3, 0xf200, 0xf24c, 0xf297, 0xf2e0, 0xf328, 0xf36e, 0xf3b4, 0xf3f8, 0xf43a,
        0xf47c, 0xf4bd, 0xf4fc, 0xf53b, 0xf579, 0xf5b5, 0xf5f1, 0xf62c, 0xf666, 0xf69f,
        0xf6d7, 0xf70e, 0xf745, 0xf77b, 0xf7b0, 0xf7e5, 0xf818, 0xf84b, 0xf87e, 0xf8b0,
        0xf8e1, 0xf911, 0xf941, 0xf970, 0xf99f, 0xf9cd, 0xf9fb, 0xfa28, 0xfa55, 0xfa81,
        0xfaad, 0xfad8, 0xfb03, 0xfb2d, 0xfb56, 0xfb80, 0xfba9, 0xfbd1, 0xfbf9, 0xfc21,
        0xfc48, 0xfc6f, 0xfc95, 0xfcbb, 0xfce1, 0xfd06, 0xfd2b, 0xfd50, 0xfd74, 0xfd98,
        0xfdbc, 0xfddf, 0xfe02, 0xfe25, 0xfe47, 0xfe69, 0xfe8b, 0xfead, 0xfece, 0xfeef,
        0xff0f,
    };

    if (v <= 100) {
        return vol_table[v];
    }

    for (int i = 0; i < sizeof(vol_table) / 2; i++) {
        if (v <= vol_table[i]) {
            return i;
        }
    }

    return 0;
}


#if USB_ROOT2
static u8 SetInterface_0_Lock;
#endif
static u32 audio_ac_itf_handler(struct usb_device_t *usb_device, struct usb_ctrlrequest *setup)
{
    u8 mute;

    const usb_dev usb_id = usb_device2id(usb_device);
    u32 tx_len;
    u8 *tx_payload = usb_get_setup_buffer(usb_device);
    u32 bRequestType = setup->bRequestType & USB_TYPE_MASK;
    u32 ret = 0;
    u8 uac2_range_param[2 + 12];  //layout 3 parameter block

    switch (bRequestType) {
    case USB_TYPE_STANDARD:
        switch (setup->bRequest) {
        case USB_REQ_SET_INTERFACE:
#if USB_ROOT2
            SetInterface_0_Lock = 1;
            if (usb_device->bDeviceStates == USB_DEFAULT) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_ADDRESS) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_CONFIGURED) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);//no alt setting
            }
#else
            if (setup->wValue == 0) { //alt 0
                usb_set_setup_phase(usb_device, USB_EP0_STAGE_SETUP);
            } else {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            }
#endif
            break;
        case USB_REQ_GET_INTERFACE:
#if USB_ROOT2
            if (usb_root2_testing()) {
                SetInterface_0_Lock = 0;
            }
#endif
            if (setup->wValue || (setup->wLength != 1)) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_DEFAULT) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_ADDRESS) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_CONFIGURED) {
                tx_len = 1;
                tx_payload[0] = 0x00;
                usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
            }
            break;
        case USB_REQ_GET_STATUS:
            if (usb_device->bDeviceStates == USB_DEFAULT) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_ADDRESS) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            }
            break;
        default:
            ret = 1 ;
            break;
        }
        break;

    case USB_TYPE_CLASS:
        switch (setup->bRequest) {
        case UAC2_CS_CUR:
            if ((setup->bRequestType & USB_DIR_IN) == 0) {
                usb_set_setup_recv(usb_device, uac_vol_handler);
            } else {
                switch (HIBYTE(setup->wIndex)) {
                case SPK_CLOCK_SOURCE_ID:
                    if (spk_info == NULL) {
                        ret = 1;
                        break;
                    }
                    switch (HIBYTE(setup->wValue)) {
                    case UAC2_CS_CONTROL_SAM_FREQ:
                        tx_payload = (u8 *)&spk_info->spk_sampling_rate;
                        tx_len = setup->wLength;
                        usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
                        break;
                    case UAC2_CS_CONTROL_CLOCK_VALID:
                        tx_payload[0] = 1;
                        tx_len = setup->wLength;
                        usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
                        break;
                    default:
                        ret = 1 ;
                        break;
                    }
                    break;

                case SPK_FEATURE_UNIT_ID:
                    if (spk_info == NULL) {
                        ret = 1;
                        break;
                    }
                    switch (HIBYTE(setup->wValue)) {
                    case UAC_FU_MUTE:
                        mute = spk_info->spk_mute ;
                        tx_payload = &mute;
                        tx_len = setup->wLength;
                        usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
                        break;
                    case UAC_FU_VOLUME:
                        if (LOBYTE(setup->wValue) == 0) {
                            tx_payload = (u8 *)&spk_info->spk_left_vol;
                        } else if (LOBYTE(setup->wValue) == 1) {
                            tx_payload = (u8 *)&spk_info->spk_left_vol;
                        } else if (LOBYTE(setup->wValue) == 2) {
                            tx_payload = (u8 *)&spk_info->spk_right_vol;
                        }
                        tx_len = setup->wLength;
                        usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
                        break;
                    default:
                        ret = 1 ;
                        break;
                    }
                    break;

                case MIC_CLOCK_SOURCE_ID:
                    if (mic_info == NULL) {
                        ret = 1;
                        break;
                    }
                    switch (HIBYTE(setup->wValue)) {
                    case UAC2_CS_CONTROL_SAM_FREQ:
                        tx_payload = (u8 *)&mic_info->mic_sampling_rate;
                        tx_len = setup->wLength;
                        usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
                        break;
                    case UAC2_CS_CONTROL_CLOCK_VALID:
                        tx_payload[0] = 1;
                        tx_len = setup->wLength;
                        usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
                        break;
                    default:
                        ret = 1 ;
                        break;
                    }
                    break;

                case MIC_FEATURE_UNIT_ID:
                    if (mic_info == NULL) {
                        ret = 1;
                        break;
                    }
                    switch (HIBYTE(setup->wValue)) {
                    case UAC_FU_MUTE:
                        mute = mic_info->mic_mute ;
                        tx_payload = &mute;
                        tx_len = setup->wLength;
                        usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
                        break;
                    case UAC_FU_VOLUME:
                        tx_payload = (u8 *)&mic_info->mic_vol;
                        tx_len = setup->wLength;
                        usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
                        break;
                    case UAC_FU_AUTOMATIC_GAIN:
                        if (mic_info->bAGC) {
                            tx_payload[0] = 0x01;
                        } else {
                            tx_payload[0] = 0x00;
                        }
                        tx_len = setup->wLength;
                        usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
                        break;
                    default:
                        ret = 1 ;
                        break;
                    }
                    break;

                default:
                    ret = 1 ;
                    break;
                }
            }
            break;

        case UAC2_CS_RANGE:
            if ((setup->bRequestType & USB_DIR_IN) == 0) {

            } else {
                switch (HIBYTE(setup->wIndex)) {
                case SPK_CLOCK_SOURCE_ID:
                    if (spk_info == NULL) {
                        ret = 1;
                        break;
                    }
                    switch (HIBYTE(setup->wValue)) {
                    case UAC2_CS_CONTROL_SAM_FREQ:
                        memcpy(tx_payload,
                               speaker_default_sampling_freq_table,
                               sizeof(speaker_default_sampling_freq_table));
                        tx_len = sizeof(speaker_default_sampling_freq_table);
                        usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
                        break;
                    default:
                        ret = 1 ;
                        break;
                    }
                    break;

                case SPK_FEATURE_UNIT_ID:
                    if (spk_info == NULL) {
                        ret = 1;
                        break;
                    }
                    switch (HIBYTE(setup->wValue)) {
                    case UAC_FU_VOLUME:
                        memset(uac2_range_param, 0, sizeof(uac2_range_param));
                        uac2_range_param[0] = 1;
                        memcpy(&uac2_range_param[2], &spk_info->spk_min_vol, 2);
                        memcpy(&uac2_range_param[4], &spk_info->spk_max_vol, 2);
                        memcpy(&uac2_range_param[6], &spk_info->spk_vol_res, 2);
                        tx_payload = (u8 *)uac2_range_param;
                        tx_len = setup->wLength;
                        usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
                        break;
                    default:
                        ret = 1 ;
                        break;
                    }
                    break;

                case MIC_CLOCK_SOURCE_ID:
                    if (mic_info == NULL) {
                        ret = 1;
                        break;
                    }
                    switch (HIBYTE(setup->wValue)) {
                    case UAC2_CS_CONTROL_SAM_FREQ:
                        memcpy(tx_payload,
                               mic_default_sampling_freq_table,
                               sizeof(mic_default_sampling_freq_table));
                        tx_len = sizeof(mic_default_sampling_freq_table);
                        usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
                        break;
                    default:
                        ret = 1 ;
                        break;
                    }
                    break;

                case MIC_FEATURE_UNIT_ID:
                    if (mic_info == NULL) {
                        ret = 1;
                        break;
                    }
                    switch (HIBYTE(setup->wValue)) {
                    case UAC_FU_VOLUME:
                        memset(uac2_range_param, 0, sizeof(uac2_range_param));
                        uac2_range_param[0] = 1;
                        memcpy(&uac2_range_param[2], &mic_info->mic_min_vol, 2);
                        memcpy(&uac2_range_param[4], &mic_info->mic_max_vol, 2);
                        memcpy(&uac2_range_param[6], &mic_info->mic_vol_res, 2);
                        tx_payload = (u8 *)uac2_range_param;
                        tx_len = setup->wLength;
                        usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
                        break;
                    default:
                        ret = 1 ;
                        break;
                    }
                    break;

                default:
                    ret = 1 ;
                    break;
                }
            }
            break;

        default:
            ret = 1 ;
            break;
        }
        break;

    default:
        ret = 1 ;
        break;
    }
    return ret;
}
#define     UAC_PLC_EN  0
#if UAC_PLC_EN
typedef struct _USBAUDIO_PLC_API {
    unsigned int (*need_buf)();
    void (*open)(unsigned char *ptr, int sr, int nch);
    int (*run)(unsigned char *ptr, short *inbuf, short *oubuf, short len, short err_flag);
} USBAUDIO_PLC_API;

extern USBAUDIO_PLC_API *get_USBplc_api();

static USBAUDIO_PLC_API *uac_plc_api = 0;
static u8 *uac_plc_buffer;

void usb_audio_plc_open()
{
    uac_plc_api = get_USBplc_api();
    u32 bufsize = uac_plc_api->need_buf();
    uac_plc_buffer = malloc(bufsize);
    uac_plc_api->open(uac_plc_buffer, SPK_AUDIO_RATE, SPK_CHANNEL);
}
void usb_audio_plc_close()
{
    if (uac_plc_api) {
        uac_plc_api = NULL;
    }
    if (uac_plc_buffer) {
        free(uac_plc_buffer);
        uac_plc_buffer = NULL;
    }
}
#endif

static u32 spk_get_max_sampling_rate()
{
    u32 sr = 0;
#if SPK_AUDIO_RATE_NUM > 1
    //find the max sample rate
    if (sr < SPK_AUDIO_RATE_1) {
        sr = SPK_AUDIO_RATE_1;
    }
    if (sr < SPK_AUDIO_RATE_2) {
        sr = SPK_AUDIO_RATE_2;
    }
    if (sr < SPK_AUDIO_RATE) {
        sr = SPK_AUDIO_RATE;
    }
#else
    sr = SPK_AUDIO_RATE;
#endif
    return sr;
}
static u32 spk_get_max_frame_len()
{
    u32 sr;
    u32 spk_res;
    u32 spk_frame_len;

    sr = spk_get_max_sampling_rate();
    spk_res = SPK_AUDIO_RES;
#if SPK_AUDIO_RES_2
    if (SPK_AUDIO_RES < SPK_AUDIO_RES_2) {
        spk_res = SPK_AUDIO_RES_2;
    }
#endif
#if UAC_24BIT_IN_4BYTE
    if (spk_res == 24) {
        spk_res = 32;
    }
#endif
    spk_frame_len = ((sr * (spk_res / 8) * SPK_CHANNEL) / 1000);
    spk_frame_len += ((sr % 1000) ? (spk_res / 8) * SPK_CHANNEL : 0);
    return spk_frame_len;
}
static void spk_transfer(struct usb_device_t *usb_device, u32 ep)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    u8 *ep_buffer;
    u32 spk_res = 0;
    u32 spk_frame_len;
    u32 rx_len;

    if (spk_info == NULL) {
        return;
    }
    ep_buffer = spk_info->spk_dma_buffer;
    spk_res = spk_info->res;
#if UAC_24BIT_IN_4BYTE
    if (spk_res == 24) {
        spk_res = 32;
    }
#endif
    spk_frame_len = spk_info->spk_sampling_rate * (spk_res / 8) * SPK_CHANNEL / 1000;
    spk_frame_len += ((spk_info->spk_sampling_rate % 1000) ? (spk_res / 8) * SPK_CHANNEL : 0);
    rx_len = usb_g_iso_read(usb_id, SPK_ISO_EP_OUT, NULL, spk_frame_len, 0);

#if UAC_PLC_EN
    if (rx_len) {
        uac_plc_api->run(uac_plc_buffer, ep_buffer, ep_buffer, rx_len, 0);
    } else {
        uac_plc_api->run(uac_plc_buffer, ep_buffer, ep_buffer, rx_len, 1);
    }
#endif

    uac_speaker_stream_write(usb_id, ep_buffer, rx_len);
    spk_info->downstream_halt = 0;
}
static void spk_feed_data_when_downstream_halt(struct usb_device_t *usb_device, u32 ep, u32 frame)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    u8 *ep_buffer;
    u32 spk_res;
    u32 spk_frame_len;
    u32 padding_len = 0;

    if (spk_info == NULL) {
        return;
    }

    if (uac_speaker_stream_status() == 0) {
        usb_g_set_sof_hander(usb_id, SPK_ISO_EP_OUT, NULL);
        return;
    }
#ifdef USB_HW_20
#if FUSB_MODE == 0

    if (usb_device->bSpeed == USB_SPEED_HIGH) {
        if ((++spk_info->sof_cnt % 8) != 0) {
            return;
        }
    }
#endif
#endif
    spk_res = spk_info->res;
    if (spk_info->spk_padding_interval) {
        if (++spk_info->spk_padding_cnt == spk_info->spk_padding_interval) {
            spk_info->spk_padding_cnt = 0;
            padding_len = (spk_res / 8) * SPK_CHANNEL;
        }
    }
    ep_buffer = spk_info->spk_dma_buffer;
    if (spk_info->downstream_halt >= 5) {
        spk_info->downstream_halt = 5;  //avoid numerical overflow
        spk_frame_len = spk_info->spk_sampling_rate * (spk_res / 8) * SPK_CHANNEL / 1000;
        spk_frame_len += padding_len;
        memset(ep_buffer, 0, spk_frame_len);
        uac_speaker_stream_write(usb_id, ep_buffer, spk_frame_len);
    }
    spk_info->downstream_halt++;
}


void spk_reset(struct usb_device_t *usb_device, u32 ift_num)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    log_debug("%s", __func__);
    if (spk_info) {
        u8 *ep_buffer = spk_info->spk_dma_buffer;
        usb_g_ep_config(usb_id, SPK_ISO_EP_OUT | USB_DIR_OUT, USB_ENDPOINT_XFER_ISOC, 1, ep_buffer, spk_get_max_frame_len());
    }
}

static void open_spk(struct usb_device_t *usb_device)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    u32 spk_frame_len;
    u8 *ep_buffer;
    u32 spk_res = 0;

    if (spk_info == NULL) {
        return;
    }
    log_info("%s", __func__);
    if (spk_info->spk_sampling_rate == 0) {
        spk_info->spk_sampling_rate = SPK_AUDIO_RATE_2;//spk_get_max_sampling_rate();
    }
    if (spk_info->alternate_setting == 1) {
        spk_res = SPK_AUDIO_RES;
    } else if (spk_info->alternate_setting == 2) {
        spk_res = SPK_AUDIO_RES_2;
    } else {
        return;
    }
#if UAC_24BIT_IN_4BYTE
    if (spk_res == 24) {
        spk_res = 32;
    }
#endif
    spk_frame_len = spk_info->spk_sampling_rate * (spk_res / 8) * SPK_CHANNEL / 1000;
    spk_frame_len += ((spk_info->spk_sampling_rate % 1000) ? (spk_res / 8) * SPK_CHANNEL : 0);
    if (spk_info->spk_sampling_rate % 1000) {
        spk_info->spk_padding_interval = 1000 / (spk_info->spk_sampling_rate % 1000);
        spk_info->spk_padding_cnt = 0;
    } else {
        spk_info->spk_padding_interval = 0;
        spk_info->spk_padding_cnt = 0;
    }
#if UAC_24BIT_IN_4BYTE
    if (spk_res == 32) {
        spk_res = 24;
    }
#endif
    spk_info->res = spk_res;
    ep_buffer = spk_info->spk_dma_buffer;
    uac_speaker_stream_open(usb_id, spk_info->spk_sampling_rate, SPK_CHANNEL);
    //uac_speaker_stream_open(usb_id, 48000, SPK_CHANNEL);

#if UAC_PLC_EN
    usb_audio_plc_open();
#endif

    usb_g_set_intr_hander(usb_id, SPK_ISO_EP_OUT | USB_DIR_OUT, spk_transfer);

    spk_info->sof_cnt = 0;
    spk_info->downstream_halt = 0;
    //usb_g_set_sof_hander(usb_id, SPK_ISO_EP_OUT, spk_feed_data_when_downstream_halt);

    usb_g_ep_config(usb_id, SPK_ISO_EP_OUT | USB_DIR_OUT, USB_ENDPOINT_XFER_ISOC, 1, ep_buffer, spk_frame_len);
}
static void close_spk(struct usb_device_t *usb_device)
{
    log_info("%s", __func__);
    const usb_dev usb_id = usb_device2id(usb_device);
    int release = 0;
    usb_clr_intr_rxe(usb_id, SPK_ISO_EP_OUT);
    usb_g_set_intr_hander(usb_id, SPK_ISO_EP_OUT, NULL);
    //usb_g_set_sof_hander(usb_id, SPK_ISO_EP_OUT, NULL);

#if UAC_PLC_EN
    usb_audio_plc_close();
#endif

    if (spk_info) {
        release = (spk_info->alternate_setting == (u8)(-1)) ? 1 : 0;
    }
    //uac_speaker_stream_close(usb_id);

}
static u32 spk_as_itf_hander(struct usb_device_t *usb_device, struct usb_ctrlrequest *setup)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    u32 tx_len;
    u8 *tx_payload = usb_get_setup_buffer(usb_device);
    u32 bRequestType = setup->bRequestType & USB_TYPE_MASK;
    u32 ret = 0;

    switch (setup->bRequest) {
    case USB_REQ_SET_INTERFACE:
        if (usb_device->bDeviceStates == USB_DEFAULT) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        } else if (usb_device->bDeviceStates == USB_ADDRESS) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        } else if (usb_device->bDeviceStates == USB_CONFIGURED) {
            if (spk_info) {
                spk_info->alternate_setting = setup->wValue;
            }
#if USB_ROOT2
            if (!usb_root2_testing()) {
                SetInterface_0_Lock = 1;
            }
            if (SetInterface_0_Lock == 0) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else {
                usb_set_setup_phase(usb_device, USB_EP0_STAGE_SETUP);
                if (setup->wValue == 1) { //alt 1
                    open_spk(usb_device);
                } else if (setup->wValue == 0) { //alt 0
                    close_spk(usb_device);
                } else {
                    usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
                }
            }
#else
            usb_set_setup_phase(usb_device, USB_EP0_STAGE_SETUP);
            if (setup->wValue == 1) { //alt 1
                open_spk(usb_device);
            } else if (setup->wValue == 2) { //alt 2
                open_spk(usb_device);
            } else if (setup->wValue == 0) { //alt 0
                close_spk(usb_device);
            } else {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            }
#endif
        }

        break;
    case USB_REQ_GET_INTERFACE:
        if (usb_root2_testing()) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        } else {
            if (setup->wValue || (setup->wLength != 1)) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_DEFAULT) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_ADDRESS) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_CONFIGURED) {
                tx_len = 1;
                if (spk_info) {
                    tx_payload[0] = spk_info->alternate_setting;
                }
                usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
            }
        }
        break;
    case USB_REQ_GET_STATUS:
        if (usb_device->bDeviceStates == USB_DEFAULT) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        } else if (usb_device->bDeviceStates == USB_ADDRESS) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        } else {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        }
        break;
    default:
        break;
    }
    return ret;
}


static u32 mic_as_itf_hander(struct usb_device_t *usb_device, struct usb_ctrlrequest *setup)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    u8 *tx_payload = usb_get_setup_buffer(usb_device);
    u32 tx_len;
    u32 bRequestType = setup->bRequestType & USB_TYPE_MASK;
    u32 ret = 0;

    switch (setup->bRequest) {
    case USB_REQ_SET_INTERFACE:
        printf("@@@%s, line = %d\n", __FUNCTION__, __LINE__);
        if (usb_device->bDeviceStates == USB_DEFAULT) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        } else if (usb_device->bDeviceStates == USB_ADDRESS) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        } else if (usb_device->bDeviceStates == USB_CONFIGURED) {
            if (mic_info) {
                mic_info->alternate_setting = setup->wValue;
            }
            printf("@@@%s, line = %d\n", __FUNCTION__, __LINE__);
#if USB_ROOT2
            if (!usb_root2_testing()) {
                SetInterface_0_Lock = 1;
            }
            if (SetInterface_0_Lock == 0) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else {
                usb_set_setup_phase(usb_device, USB_EP0_STAGE_SETUP);
                if (setup->wValue == 1) {//alt 1
                    open_mic(usb_device);
                } else if (setup->wValue == 0) { //alt 0
                    close_mic(usb_device);
                } else {
                    usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
                }
            }
#else
            usb_set_setup_phase(usb_device, USB_EP0_STAGE_SETUP);
            if (setup->wValue == 1) {//alt 1
                open_mic(usb_device);
                printf("@@@%s, line = %d\n", __FUNCTION__, __LINE__);
            } else if (setup->wValue == 2) {//alt 2
                open_mic(usb_device);
                printf("@@@%s, line = %d\n", __FUNCTION__, __LINE__);
            } else if (setup->wValue == 0) { //alt 0
                close_mic(usb_device);
                printf("@@@%s, line = %d\n", __FUNCTION__, __LINE__);
            } else {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
                printf("@@@%s, line = %d\n", __FUNCTION__, __LINE__);
            }
#endif
        }
        break;
    case USB_REQ_GET_INTERFACE:
        if (usb_root2_testing()) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        } else {
            if (setup->wValue || (setup->wLength != 1)) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_DEFAULT) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_ADDRESS) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_CONFIGURED) {
                tx_len = 1;
                if (mic_info) {
                    tx_payload[0] = mic_info->alternate_setting;
                }
                usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
            }
        }
        break;
    case USB_REQ_GET_STATUS:
        if (usb_device->bDeviceStates == USB_DEFAULT) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        } else if (usb_device->bDeviceStates == USB_ADDRESS) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        } else {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        }
        break;
    default:
        break;
    }
    return ret;
}


static u32 mic_get_max_sampling_rate()
{
    u32 sr = 0;
#if MIC_SamplingFrequency > 1
    //find the max sample rate
    if (sr < MIC_AUDIO_RATE_1) {
        sr = MIC_AUDIO_RATE_1;
    }
    if (sr < MIC_AUDIO_RATE_2) {
        sr = MIC_AUDIO_RATE_2;
    }
    if (sr < MIC_AUDIO_RATE) {
        sr = MIC_AUDIO_RATE;
    }
#else
    sr = MIC_AUDIO_RATE;
#endif
    return sr;
}
static u32 mic_get_max_frame_len()
{
    u32 sr;
    u32 mic_res;
    u32 mic_frame_len;

    sr = mic_get_max_sampling_rate();
    mic_res = MIC_AUDIO_RES;
#if MIC_AUDIO_RES_2
    if (MIC_AUDIO_RES < MIC_AUDIO_RES_2) {
        mic_res = MIC_AUDIO_RES_2;
    }
#endif
#if UAC_24BIT_IN_4BYTE
    if (mic_res == 24) {
        mic_res = 32;
    }
#endif
    mic_frame_len = ((sr * (mic_res / 8) * MIC_CHANNEL) / 1000);
    mic_frame_len += ((sr % 1000) ? (mic_res / 8) * MIC_CHANNEL : 0);
    return mic_frame_len;
}
static void mic_transfer(struct usb_device_t *usb_device, u32 ep)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    u8 *ep_buffer;
    u32 mic_frame_len;
    u32 mic_res = 0;

    if (mic_info == NULL) {
        return;
    }
    ep_buffer = mic_info->mic_dma_buffer;
    mic_res = mic_info->res;
#if UAC_24BIT_IN_4BYTE
    if (mic_res == 24) {
        mic_res = 32;
    }
#endif
    mic_frame_len = ((mic_info->mic_sampling_rate * mic_res / 8 * MIC_CHANNEL) / 1000);
    if (mic_info->mic_padding_interval) {
        if (++mic_info->mic_padding_cnt == mic_info->mic_padding_interval) {
            mic_info->mic_padding_cnt = 0;
            mic_frame_len += (mic_res / 8) * MIC_CHANNEL;
        }
    }

    int len = uac_mic_stream_read(usb_id, ep_buffer,  mic_frame_len);
    if (len) {
        mic_info->mic_no_data = 0;
    } else { /* if (mic_info->mic_no_data) */
        len = mic_frame_len;
        memset(ep_buffer, 0, len);
    }

    usb_g_iso_write(usb_id, MIC_ISO_EP_IN, NULL, len);

    mic_info->upstream_halt = 0;
}
static void mic_drop_data_when_upstream_halt(struct usb_device_t *usb_device, u32 ep, u32 frame)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    u8 *ep_buffer;
    u32 mic_frame_len;
    u32 mic_res = 0;

    if (mic_info == NULL) {
        return;
    }

    if (uac_get_mic_stream_status(usb_id) == 0) {
        usb_g_set_sof_hander(usb_id, MIC_ISO_EP_IN | USB_DIR_IN, NULL);
        return;
    }
#ifdef USB_HW_20
#if FUSB_MODE == 0
    if (usb_device->bSpeed == USB_SPEED_HIGH) {
        if ((++mic_info->sof_cnt % 8) != 0) {
            return;
        }
    }
#endif
#endif

    ep_buffer = mic_info->mic_dma_buffer;

    if (mic_info->upstream_halt >= 5) {
        mic_info->upstream_halt = 5;  //avoid numerical overflow
        mic_res = mic_info->res;
#if UAC_24BIT_IN_4BYTE
        if (mic_res == 24) {
            mic_res = 32;
        }
#endif
        mic_frame_len = ((mic_info->mic_sampling_rate * mic_res / 8 * MIC_CHANNEL) / 1000);
        if (mic_info->mic_padding_interval) {
            if (++mic_info->mic_padding_cnt == mic_info->mic_padding_interval) {
                mic_info->mic_padding_cnt = 0;
                mic_frame_len += (mic_res / 8) * MIC_CHANNEL;
            }
        }

        uac_mic_stream_read(usb_id, ep_buffer,  mic_frame_len);
    }
    mic_info->upstream_halt++;
}


void mic_reset(struct usb_device_t *usb_device, u32 ift_num)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    log_error("%s", __func__);
#if USB_ROOT2
    usb_disable_ep(usb_id, SPK_ISO_EP_OUT);
    /* uac_release(usb_id); */
#else
    if (mic_info) {
        u8 *ep_buffer = mic_info->mic_dma_buffer;
        usb_g_ep_config(usb_id, MIC_ISO_EP_IN | USB_DIR_IN, USB_ENDPOINT_XFER_ISOC, 1, ep_buffer, mic_get_max_frame_len());
    }
#endif
}


static void open_mic(struct usb_device_t *usb_device)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    u8 *ep_buffer;
    u32 mic_frame_len;
    u32 mic_res = 0;
    printf("@@@%s, line = %d\n", __FUNCTION__, __LINE__);
    if (mic_info == NULL) {
        printf("@@@%s, line = %d\n", __FUNCTION__, __LINE__);
        return;
    }
    log_info("%s", __func__);
    mic_info->mic_no_data = 1;
    usb_enable_ep(usb_id, MIC_ISO_EP_IN);

    ep_buffer = mic_info->mic_dma_buffer;

    usb_g_set_intr_hander(usb_id, MIC_ISO_EP_IN | USB_DIR_IN, mic_transfer);

    if (mic_info->mic_sampling_rate == 0) {
        mic_info->mic_sampling_rate = mic_get_max_sampling_rate();
    }
    if (mic_info->alternate_setting == 1) {
        mic_res = MIC_AUDIO_RES;
    } else if (mic_info->alternate_setting == 2) {
        mic_res = MIC_AUDIO_RES_2;
    } else {
        return;
    }
#if UAC_24BIT_IN_4BYTE
    if (mic_res == 24) {
        mic_res = 32;
    }
#endif
    mic_frame_len = ((mic_info->mic_sampling_rate * mic_res / 8 * MIC_CHANNEL) / 1000);
    mic_frame_len += ((mic_info->mic_sampling_rate % 1000) ? (mic_res / 8) * MIC_CHANNEL : 0);
    if (mic_info->mic_sampling_rate % 1000) {
        mic_info->mic_padding_interval = 1000 / (mic_info->mic_sampling_rate % 1000);
        mic_info->mic_padding_cnt = 0;
    } else {
        mic_info->mic_padding_interval = 0;
        mic_info->mic_padding_cnt = 0;
    }
#if UAC_24BIT_IN_4BYTE
    if (mic_res == 32) {
        mic_res = 24;
    }
#endif

    mic_info->res = mic_res;
    uac_mic_stream_open(usb_id, mic_info->mic_sampling_rate, MIC_CHANNEL, mic_res);

    mic_info->sof_cnt = 0;
    mic_info->upstream_halt = 0;
    usb_g_set_sof_hander(usb_id, MIC_ISO_EP_IN | USB_DIR_IN, mic_drop_data_when_upstream_halt);

    usb_g_ep_config(usb_id,
                    MIC_ISO_EP_IN | USB_DIR_IN,
                    USB_ENDPOINT_XFER_ISOC,
                    1, ep_buffer, mic_frame_len);

    mic_transfer(usb_device, MIC_ISO_EP_IN);
}
static void close_mic(struct usb_device_t *usb_device)
{
    log_info("%s", __func__);
    const usb_dev usb_id = usb_device2id(usb_device);
    int release = 0;
    usb_clr_intr_txe(usb_id, MIC_ISO_EP_IN);
    usb_g_set_intr_hander(usb_id, MIC_ISO_EP_IN | USB_DIR_IN, NULL);
    if (mic_info) {
        release = (mic_info->alternate_setting == (u8)(-1)) ? 1 : 0;
    }
    uac_mic_stream_close(usb_id);
}


const u8 *uac_get_string(u32 id)
{
    if (id == SPEAKER_STR_INDEX) {
        return speakerStringDescriptor;
    } else if (id == MIC_STR_INDEX) {
        return micStringDescriptor;
    }
    return NULL;
}


u32 uac_spk_desc_config(const usb_dev usb_id, u8 *ptr, u32 *cur_itf_num)
{
    //cppcheck-suppress unreadVariable
    struct usb_device_t *usb_device = usb_id2device(usb_id);
    int i = 0;
    u8 *tptr = ptr;
    u32 offset;
    u32 frame_len;
    u32 sr;
    u8 *pCtrlItf;
    u32 dwCtrlSize = 0;

    log_debug("spkdc:%d\n", *cur_itf_num);

    memcpy(tptr, (u8 *)uac_audio_interface_association, sizeof(uac_audio_interface_association));
    tptr[2] = *cur_itf_num;
    tptr[3] = 2;
    tptr += sizeof(uac_audio_interface_association);//0x09

    memcpy(tptr, (u8 *)uac_ac_standard_interface_desc, sizeof(uac_ac_standard_interface_desc));
    tptr[2] = *cur_itf_num;
    tptr += sizeof(uac_ac_standard_interface_desc);//0x09

    memcpy(tptr, (u8 *)uac_spk_ac_interface, sizeof(uac_spk_ac_interface));
    pCtrlItf = tptr;
    dwCtrlSize = sizeof(uac_spk_ac_interface);
    tptr += sizeof(uac_spk_ac_interface);//0x09

    memcpy(tptr, (u8 *)uac_spk_clock_source_desc, sizeof(uac_spk_clock_source_desc));
    dwCtrlSize += sizeof(uac_spk_clock_source_desc);
    tptr += sizeof(uac_spk_clock_source_desc);

    memcpy(tptr, (u8 *)uac_spk_input_terminal_desc, sizeof(uac_spk_input_terminal_desc));
    dwCtrlSize += sizeof(uac_spk_input_terminal_desc);
    tptr += sizeof(uac_spk_input_terminal_desc);//0x0c

    memcpy(tptr, (u8 *)uac_spk_feature_desc, sizeof(uac_spk_feature_desc));
    dwCtrlSize += sizeof(uac_spk_feature_desc);
    tptr += sizeof(uac_spk_feature_desc);//0x0a

    /* memcpy(tptr, (u8 *)uac_spk_selector_uint_desc, sizeof(uac_spk_selector_uint_desc)); */
    /* dwCtrlSize += sizeof(uac_spk_selector_uint_desc); */
    /* tptr += sizeof(uac_spk_selector_uint_desc); */

    memcpy(tptr, (u8 *)uac_spk_output_terminal_desc, sizeof(uac_spk_output_terminal_desc));
    dwCtrlSize += sizeof(uac_spk_output_terminal_desc);
    tptr += sizeof(uac_spk_output_terminal_desc);//0x09

    //wTotalLength of class-specific AC interface descriptors
    pCtrlItf[6] = LOBYTE(dwCtrlSize);
    pCtrlItf[7] = HIBYTE(dwCtrlSize);

    memcpy(tptr, (u8 *)uac_spk_as_interface_desc, sizeof(uac_spk_as_interface_desc));
    tptr[2] = *cur_itf_num + 1;
    tptr[9 + 2] = *cur_itf_num + 1;
    sr = spk_get_max_sampling_rate();
#if SPK_AUDIO_RES == 24 && UAC_24BIT_IN_4BYTE
    frame_len = sr * 4 * SPK_CHANNEL / 1000;
    frame_len += (sr % 1000 ? 4 * SPK_CHANNEL : 0);
#else
    frame_len = sr * (SPK_AUDIO_RES / 8) * SPK_CHANNEL / 1000;
    frame_len += ((sr % 1000) ? (SPK_AUDIO_RES / 8) * SPK_CHANNEL : 0);
#endif
    offset = 9 + 9 + 16 + 6 + 4;
    //MaxPacketSize of spk iso out
    tptr[offset] = LOBYTE(frame_len);
    tptr[offset + 1] = HIBYTE(frame_len);
#if defined(FUSB_MODE) && FUSB_MODE == 0
    if (usb_device->bSpeed == USB_SPEED_FULL) {
        tptr[offset + 2] = UAC_ISO_INTERVAL_FS;
    }
#endif
#if SPK_AUDIO_RES_2
    offset = 9 + 9 + 16 + 6 + 7 + 8;
    tptr[offset + 2] = *cur_itf_num + 1;
#if SPK_AUDIO_RES_2 == 24 && UAC_24BIT_IN_4BYTE
    frame_len = sr * 4 * SPK_CHANNEL / 1000;
    frame_len += (sr % 1000 ? 4 * SPK_CHANNEL : 0);
#else
    frame_len = sr * (SPK_AUDIO_RES_2 / 8) * SPK_CHANNEL / 1000;
    frame_len += (sr % 1000 ? (SPK_AUDIO_RES_2 / 8) * SPK_CHANNEL : 0);
#endif
    offset += 9 + 16 + 6 + 4;
    //MaxPacketSize of spk iso out
    tptr[offset] = LOBYTE(frame_len);
    tptr[offset + 1] = HIBYTE(frame_len);
#if defined(FUSB_MODE) && FUSB_MODE == 0
    if (usb_device->bSpeed == USB_SPEED_FULL) {
        tptr[offset + 2] = UAC_ISO_INTERVAL_FS;
    }
#endif
#endif
    tptr += sizeof(uac_spk_as_interface_desc);

    if (usb_set_interface_hander(usb_id, *cur_itf_num, audio_ac_itf_handler) != *cur_itf_num) {
        ASSERT(0, "uac spk set interface_hander fail");
    }
    if (usb_set_reset_hander(usb_id, *cur_itf_num, spk_reset) != *cur_itf_num) {

    }
    (*cur_itf_num) ++;
    if (usb_set_interface_hander(usb_id, *cur_itf_num, spk_as_itf_hander) != *cur_itf_num) {
        ASSERT(0, "uac spk set interface_hander fail");
    }
    (*cur_itf_num) ++;

    i = tptr - ptr;
    return i;
}


u32 uac_mic_desc_config(const usb_dev usb_id, u8 *ptr, u32 *cur_itf_num)
{
    //cppcheck-suppress unreadVariable
    struct usb_device_t *usb_device = usb_id2device(usb_id);
    int i = 0;
    u8 *tptr = ptr;
    u32 offset;
    u32 frame_len;
    u32 sr;
    u8 *pCtrlItf;
    u32 dwCtrlSize = 0;

    log_debug("micdc:%d\n", *cur_itf_num);

    memcpy(tptr, (u8 *)uac_audio_interface_association, sizeof(uac_audio_interface_association));
    tptr[2] = *cur_itf_num;
    tptr[3] = 2;
    tptr += sizeof(uac_audio_interface_association);//0x09

    memcpy(tptr, (u8 *)uac_ac_standard_interface_desc, sizeof(uac_ac_standard_interface_desc));
    tptr[2] = *cur_itf_num;
    tptr[8] = MIC_STR_INDEX;
    tptr += sizeof(uac_ac_standard_interface_desc);//0x09

    memcpy(tptr, uac_mic_ac_interface, sizeof(uac_mic_ac_interface));
    pCtrlItf = tptr;
    dwCtrlSize = sizeof(uac_audio_ac_interface);
    tptr += sizeof(uac_mic_ac_interface);

    memcpy(tptr, (u8 *)uac_mic_clock_source_desc, sizeof(uac_mic_clock_source_desc));
    dwCtrlSize += sizeof(uac_mic_clock_source_desc);
    tptr += sizeof(uac_mic_clock_source_desc);

    memcpy(tptr, (u8 *)uac_mic_input_terminal_desc, sizeof(uac_mic_input_terminal_desc));
    dwCtrlSize += sizeof(uac_mic_input_terminal_desc);
    tptr += sizeof(uac_mic_input_terminal_desc);//0x09

    memcpy(tptr, (u8 *)uac_mic_feature_desc, sizeof(uac_mic_feature_desc));
    dwCtrlSize += sizeof(uac_mic_feature_desc);
    tptr += sizeof(uac_mic_feature_desc);//0x09

    /* memcpy(tptr, (u8 *)uac_mic_selector_uint_desc, sizeof(uac_mic_selector_uint_desc)); */
    /* dwCtrlSize += sizeof(uac_mic_selector_uint_desc); */
    /* tptr += sizeof(uac_mic_selector_uint_desc); */

    memcpy(tptr, (u8 *)uac_mic_output_terminal_desc, sizeof(uac_mic_output_terminal_desc));
    dwCtrlSize += sizeof(uac_mic_output_terminal_desc);
    tptr += sizeof(uac_mic_output_terminal_desc);//0x09

    //wTotalLength of class-specific AC interface descriptors
    pCtrlItf[6] = LOBYTE(dwCtrlSize);
    pCtrlItf[7] = HIBYTE(dwCtrlSize);

    memcpy(tptr, (u8 *)uac_mic_as_interface_desc, sizeof(uac_mic_as_interface_desc));
    tptr[2] = *cur_itf_num + 1;
    tptr[9 + 2] = *cur_itf_num + 1;
    sr = mic_get_max_sampling_rate();
#if MIC_AUDIO_RES == 24 && UAC_24BIT_IN_4BYTE
    frame_len = sr * 4 * MIC_CHANNEL / 1000;
    frame_len += (sr % 1000 ? 4 * MIC_CHANNEL : 0);
#else
    frame_len = sr * (MIC_AUDIO_RES / 8) * MIC_CHANNEL / 1000;
    frame_len += ((sr % 1000) ? (MIC_AUDIO_RES / 8) * MIC_CHANNEL : 0);
#endif
    offset = 9 + 9 + 16 + 6 + 4;
    //MaxPacketSize of mic iso in
    tptr[offset] = LOBYTE(frame_len);
    tptr[offset + 1] = HIBYTE(frame_len);
#if defined(FUSB_MODE) && FUSB_MODE == 0
    if (usb_device->bSpeed == USB_SPEED_FULL) {
        tptr[offset + 2] = UAC_ISO_INTERVAL_FS;
    }
#endif
#if MIC_AUDIO_RES_2
    offset = 9 + 9 + 16 + 6 + 7 + 8;
    tptr[offset + 2] = *cur_itf_num + 1;
#if MIC_AUDIO_RES == 24 && UAC_24BIT_IN_4BYTE
    frame_len = sr * 4 * MIC_CHANNEL / 1000;
    frame_len += (sr % 1000 ? 4 * MIC_CHANNEL : 0);
#else
    frame_len = sr * (MIC_AUDIO_RES_2 / 8) * MIC_CHANNEL / 1000;
    frame_len += (sr % 1000 ? (MIC_AUDIO_RES_2 / 8) * MIC_CHANNEL : 0);
#endif
    offset += 9 + 16 + 6 + 4;
    //MaxPacketSize of mic iso in
    tptr[offset] = LOBYTE(frame_len);
    tptr[offset + 1] = HIBYTE(frame_len);
#if defined(FUSB_MODE) && FUSB_MODE == 0
    if (usb_device->bSpeed == USB_SPEED_FULL) {
        tptr[offset + 2] = UAC_ISO_INTERVAL_FS;
    }
#endif
#endif
    tptr += sizeof(uac_mic_as_interface_desc);//0x09

    if (usb_set_interface_hander(usb_id, *cur_itf_num, audio_ac_itf_handler) != *cur_itf_num) {
        ASSERT(0, "uac mic set interface_hander fail");
    }
    if (usb_set_reset_hander(usb_id, *cur_itf_num, mic_reset) != *cur_itf_num) {

    }
    (*cur_itf_num) ++;
    if (usb_set_interface_hander(usb_id, *cur_itf_num, mic_as_itf_hander) != *cur_itf_num) {
        ASSERT(0, "uac mic set interface_hander fail");
    }
    (*cur_itf_num) ++;

    i = tptr - ptr;
    return i;
}


u32 uac_audio_desc_config(const usb_dev usb_id, u8 *ptr, u32 *cur_itf_num)
{
    //cppcheck-suppress unreadVariable
    struct usb_device_t *usb_device = usb_id2device(usb_id);
    int i = 0;
    u8 *tptr = ptr;
    u32 offset;
    u32 frame_len;
    u32 sr;
    u8 *pCtrlItf;
    u32 dwCtrlSize = 0;

    memcpy(tptr, (u8 *)uac_audio_interface_association, sizeof(uac_audio_interface_association));
    tptr[2] = *cur_itf_num;
    tptr[3] = 3;
    tptr += sizeof(uac_audio_interface_association);//0x09

    memcpy(tptr, (u8 *)uac_ac_standard_interface_desc, sizeof(uac_ac_standard_interface_desc));
    tptr[2] = *cur_itf_num;
    tptr += sizeof(uac_ac_standard_interface_desc);//0x09

    memcpy(tptr, (u8 *)uac_audio_ac_interface, sizeof(uac_audio_ac_interface));
    pCtrlItf = tptr;
    dwCtrlSize = sizeof(uac_audio_ac_interface);
    tptr += sizeof(uac_audio_ac_interface);//0x09

    memcpy(tptr, (u8 *)uac_spk_clock_source_desc, sizeof(uac_spk_clock_source_desc));
    dwCtrlSize += sizeof(uac_spk_clock_source_desc);
    tptr += sizeof(uac_spk_clock_source_desc);

    memcpy(tptr, (u8 *)uac_spk_input_terminal_desc, sizeof(uac_spk_input_terminal_desc));
    dwCtrlSize += sizeof(uac_spk_input_terminal_desc);
    tptr += sizeof(uac_spk_input_terminal_desc);//0x09

    memcpy(tptr, (u8 *)uac_spk_feature_desc, sizeof(uac_spk_feature_desc));
    dwCtrlSize += sizeof(uac_spk_feature_desc);
    tptr += sizeof(uac_spk_feature_desc);//0x09

    /* memcpy(tptr, (u8 *)uac_spk_selector_uint_desc, sizeof(uac_spk_selector_uint_desc)); */
    /* dwCtrlSize += sizeof(uac_spk_selector_uint_desc); */
    /* tptr += sizeof(uac_spk_selector_uint_desc); */

    memcpy(tptr, (u8 *)uac_spk_output_terminal_desc, sizeof(uac_spk_output_terminal_desc));
    dwCtrlSize += sizeof(uac_spk_output_terminal_desc);
    tptr += sizeof(uac_spk_output_terminal_desc);//0x09

    memcpy(tptr, (u8 *)uac_mic_clock_source_desc, sizeof(uac_mic_clock_source_desc));
    dwCtrlSize += sizeof(uac_mic_clock_source_desc);
    tptr += sizeof(uac_mic_clock_source_desc);

    memcpy(tptr, (u8 *)uac_mic_input_terminal_desc, sizeof(uac_mic_input_terminal_desc));
    dwCtrlSize += sizeof(uac_mic_input_terminal_desc);
    tptr += sizeof(uac_mic_input_terminal_desc);//0x09

    memcpy(tptr, (u8 *)uac_mic_feature_desc, sizeof(uac_mic_feature_desc));
    dwCtrlSize += sizeof(uac_mic_feature_desc);
    tptr += sizeof(uac_mic_feature_desc);//0x09

    /* memcpy(tptr, (u8 *)uac_mic_selector_uint_desc, sizeof(uac_mic_selector_uint_desc)); */
    /* dwCtrlSize += sizeof(uac_mic_selector_uint_desc); */
    /* tptr += sizeof(uac_mic_selector_uint_desc); */

    memcpy(tptr, (u8 *)uac_mic_output_terminal_desc, sizeof(uac_mic_output_terminal_desc));
    dwCtrlSize += sizeof(uac_mic_output_terminal_desc);
    tptr += sizeof(uac_mic_output_terminal_desc);//0x09

    //wTotalLength of class-specific AC interface descriptors
    pCtrlItf[6] = LOBYTE(dwCtrlSize);
    pCtrlItf[7] = HIBYTE(dwCtrlSize);

    memcpy(tptr, (u8 *)uac_spk_as_interface_desc, sizeof(uac_spk_as_interface_desc));
    tptr[2] = *cur_itf_num + 1;
    tptr[9 + 2] = *cur_itf_num + 1;
    sr = spk_get_max_sampling_rate();
#if SPK_AUDIO_RES == 24 && UAC_24BIT_IN_4BYTE
    frame_len = sr * 4 * SPK_CHANNEL / 1000;
    frame_len += (sr % 1000 ? 4 * SPK_CHANNEL : 0);
#else
    frame_len = sr * (SPK_AUDIO_RES / 8) * SPK_CHANNEL / 1000;
    frame_len += ((sr % 1000) ? (SPK_AUDIO_RES / 8) * SPK_CHANNEL : 0);
#endif
    offset = 9 + 9 + 16 + 6 + 4;
    //MaxPacketSize of spk iso out
    tptr[offset] = LOBYTE(frame_len);
    tptr[offset + 1] = HIBYTE(frame_len);
#if defined(FUSB_MODE) && FUSB_MODE == 0
    if (usb_device->bSpeed == USB_SPEED_FULL) {
        tptr[offset + 2] = UAC_ISO_INTERVAL_FS;
    }
#endif
#if SPK_AUDIO_RES_2
    offset = 9 + 9 + 16 + 6 + 7 + 8;
    tptr[offset + 2] = *cur_itf_num + 1;
#if SPK_AUDIO_RES == 24 && UAC_24BIT_IN_4BYTE
    frame_len = sr * 4 * SPK_CHANNEL / 1000;
    frame_len += (sr % 1000 ? 4 * SPK_CHANNEL : 0);
#else
    frame_len = sr * (SPK_AUDIO_RES_2 / 8) * SPK_CHANNEL / 1000;
    frame_len += (sr % 1000 ? (SPK_AUDIO_RES_2 / 8) * SPK_CHANNEL : 0);
#endif
    offset += 9 + 16 + 6 + 4;
    //MaxPacketSize of spk iso out
    tptr[offset] = LOBYTE(frame_len);
    tptr[offset + 1] = HIBYTE(frame_len);
#if defined(FUSB_MODE) && FUSB_MODE == 0
    if (usb_device->bSpeed == USB_SPEED_FULL) {
        tptr[offset + 2] = UAC_ISO_INTERVAL_FS;
    }
#endif
#endif
    tptr += sizeof(uac_spk_as_interface_desc);//0x09

    memcpy(tptr, (u8 *)uac_mic_as_interface_desc, sizeof(uac_mic_as_interface_desc));
    tptr[2] = *cur_itf_num + 2;
    tptr[9 + 2] = *cur_itf_num + 2;
    sr = mic_get_max_sampling_rate();
#if MIC_AUDIO_RES == 24 && UAC_24BIT_IN_4BYTE
    frame_len = sr * 4 * MIC_CHANNEL / 1000;
    frame_len += (sr % 1000 ? 4 * MIC_CHANNEL : 0);
#else
    frame_len = sr * (MIC_AUDIO_RES / 8) * MIC_CHANNEL / 1000;
    frame_len += ((sr % 1000) ? (MIC_AUDIO_RES / 8) * MIC_CHANNEL : 0);
#endif
    offset = 9 + 9 + 16 + 6 + 4;
    //MaxPacketSize of mic iso in
    tptr[offset] = LOBYTE(frame_len);
    tptr[offset + 1] = HIBYTE(frame_len);
#if defined(FUSB_MODE) && FUSB_MODE == 0
    if (usb_device->bSpeed == USB_SPEED_FULL) {
        tptr[offset + 2] = UAC_ISO_INTERVAL_FS;
    }
#endif
#if MIC_AUDIO_RES_2
    offset = 9 + 9 + 16 + 6 + 7 + 8;
    tptr[offset + 2] = *cur_itf_num + 2;
#if MIC_AUDIO_RES_2 == 24 && UAC_24BIT_IN_4BYTE
    frame_len = sr * 4 * MIC_CHANNEL / 1000;
    frame_len += (sr % 1000 ? 4 * MIC_CHANNEL : 0);
#else
    frame_len = sr * (MIC_AUDIO_RES_2 / 8) * MIC_CHANNEL / 1000;
    frame_len += (sr % 1000 ? (MIC_AUDIO_RES_2 / 8) * MIC_CHANNEL : 0);
#endif
    offset += 9 + 16 + 6 + 4;
    //MaxPacketSize of mic iso in
    tptr[offset] = LOBYTE(frame_len);
    tptr[offset + 1] = HIBYTE(frame_len);
#if defined(FUSB_MODE) && FUSB_MODE == 0
    if (usb_device->bSpeed == USB_SPEED_FULL) {
        tptr[offset + 2] = UAC_ISO_INTERVAL_FS;
    }
#endif
#endif
    tptr += sizeof(uac_mic_as_interface_desc);//0x09

    log_debug("audio control interface num:%d\n", *cur_itf_num);
    if (usb_set_interface_hander(usb_id, *cur_itf_num, audio_ac_itf_handler) != *cur_itf_num) {
        ASSERT(0, "uac spk set interface_hander fail");
    }

    if (usb_set_reset_hander(usb_id, *cur_itf_num, audio_reset) != *cur_itf_num) {

    }
    (*cur_itf_num) ++;
    log_debug("speaker stream interface num:%d\n", *cur_itf_num);
    if (usb_set_interface_hander(usb_id, *cur_itf_num, spk_as_itf_hander) != *cur_itf_num) {
        ASSERT(0, "uac spk set interface_hander fail");
    }
    (*cur_itf_num) ++;
    log_debug("mic stream interface num:%d\n", *cur_itf_num);
    if (usb_set_interface_hander(usb_id, *cur_itf_num, mic_as_itf_hander) != *cur_itf_num) {
        ASSERT(0, "uac mic set interface_hander fail");
    }
    (*cur_itf_num) ++;

    i = tptr - ptr;
    return i;
}
u32 uac_spk_register(const usb_dev usb_id)
{
    if (spk_info == NULL) {
#if USB_MALLOC_ENABLE
        spk_info = (struct spk_info_t *)zalloc(sizeof(struct spk_info_t));
        if (spk_info == NULL) {
            printf("uac_spk_register err\n");
            return -1;
        }
#else
        memset(&_spk_info, 0, sizeof(struct spk_info_t));
        spk_info = &_spk_info;
#endif
#if USB_EP_PROTECT
        u32 ep_reserved_len = 1024 - spk_get_max_frame_len();
#else
        u32 ep_reserved_len = 0;
#endif
        spk_info->spk_dma_buffer = usb_alloc_ep_dmabuffer(usb_id, SPK_ISO_EP_OUT, spk_get_max_frame_len() + ep_reserved_len);
    }
    spk_info->spk_def_vol = vol_convert(100);
    spk_info->spk_left_vol = spk_info->spk_def_vol;
    spk_info->spk_right_vol = spk_info->spk_def_vol;
    spk_info->spk_max_vol = vol_convert(100);
    spk_info->spk_min_vol = vol_convert(0);
    spk_info->spk_vol_res = 0x30;
    spk_info->spk_mute = 0;
    spk_info->ps4_mode = 0;
    return 0;
}
void uac_spk_release(const usb_dev usb_id)
{
    struct usb_device_t *usb_device = usb_id2device(usb_id);

    if (spk_info) {
        spk_info->alternate_setting = -1;
        close_spk(usb_device);
        if (spk_info->spk_dma_buffer) {
            usb_free_ep_dmabuffer(usb_id, spk_info->spk_dma_buffer);
            spk_info->spk_dma_buffer = NULL;
        }
#if USB_MALLOC_ENABLE
        free(spk_info);
#endif
        spk_info = NULL;
    }
}
u32 uac_mic_register(const usb_dev usb_id)
{
    if (mic_info == NULL) {
#if USB_MALLOC_ENABLE
        mic_info = (struct mic_info_t *)zalloc(sizeof(struct mic_info_t));
        if (mic_info == NULL) {
            printf("uac_register err\n");
            return -1;
        }
#else
        memset(&_mic_info, 0, sizeof(struct mic_info_t));
        mic_info = &_mic_info;
#endif
        mic_info->mic_dma_buffer = usb_alloc_ep_dmabuffer(usb_id, MIC_ISO_EP_IN | USB_DIR_IN, mic_get_max_frame_len());
    }
    mic_info->mic_max_vol = vol_convert(100);
    mic_info->mic_min_vol = vol_convert(0);
    mic_info->mic_vol_res = 0x30;
    mic_info->mic_def_vol = vol_convert(13 / 14.0 * 100);
    mic_info->mic_vol = mic_info->mic_def_vol;
    mic_info->mic_mute = 0;
    mic_info->ps4_mode = 0;
    return 0;
}
void uac_mic_release(const usb_dev usb_id)
{
    struct usb_device_t *usb_device = usb_id2device(usb_id);

    if (mic_info) {
        mic_info->alternate_setting = -1;
        close_mic(usb_device);
        if (mic_info->mic_dma_buffer) {
            usb_free_ep_dmabuffer(usb_id, mic_info->mic_dma_buffer);
            mic_info->mic_dma_buffer = NULL;
        }
#if USB_MALLOC_ENABLE
        free(mic_info);
#endif
        mic_info = NULL;
    }
}


u32 uac_register(const usb_dev usb_id)
{
    uac_spk_register(usb_id);
    uac_mic_register(usb_id);
    return 0;
}


void uac_release(const usb_dev usb_id)
{
    uac_spk_release(usb_id);
    uac_mic_release(usb_id);
}

void uac_get_cur_vol(const usb_dev usb_id, u16 *l_vol, u16 *r_vol)
{
    if (l_vol) {
        *l_vol = 0;
    }
    if (r_vol) {
        *r_vol = 0;
    }
    if (spk_info) {
        if (!spk_info->spk_mute) {
            if (l_vol) {
                *l_vol = vol_convert(spk_info->spk_left_vol);
            }
            if (r_vol) {
                *r_vol = vol_convert(spk_info->spk_right_vol);
            }
        }
    }
}
u16 uac_get_mic_vol(const usb_dev usb_id)
{
    if (mic_info) {
        if (!mic_info->mic_mute) {
            return vol_convert(mic_info->mic_vol);
        }
    }
    return 0;
}

u32 uac_get_mic_sameplerate(void *priv)
{
    u32 sample_rate = MIC_AUDIO_RATE;
    if (mic_info) {
        sample_rate = mic_info->mic_sampling_rate;
    }
    return sample_rate;
}

static u32 uac_vol_handler(struct usb_device_t *usb_device, struct usb_ctrlrequest *setup)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    u32 ret = 0;
    u8 read_ep[64];
    u8 mute;
    u16 volume = 0;
    usb_read_ep0(usb_id, read_ep, setup->wLength);
    ret = USB_EP0_STAGE_SETUP;
    switch (HIBYTE(setup->wIndex)) {
    case SPK_CLOCK_SOURCE_ID:
        if (spk_info == NULL) {
            break;
        }
        switch (HIBYTE(setup->wValue)) {
        case UAC2_CS_CONTROL_SAM_FREQ:
            memcpy(&spk_info->spk_sampling_rate, read_ep, 4);
            log_info("Clock Source Control Set Sampling Frequency to %d", spk_info->spk_sampling_rate);
            break;
        case UAC2_CS_CONTROL_CLOCK_VALID:
            log_info("Clock Source Control Set Valid to %d", read_ep[0]);
            break;
        }
        break;

    case SPK_FEATURE_UNIT_ID:
        if (spk_info == NULL) {
            break;
        }
        switch (HIBYTE(setup->wValue)) {
        case UAC_FU_MUTE:
            memcpy((u8 *)&mute, read_ep, 1);
            if (mute) {
                uac_mute_volume(usb_id, SPK_FEATURE_UNIT_ID, 0, 0);
                spk_info->spk_mute = 1;
            } else {
                uac_mute_volume(usb_id, SPK_FEATURE_UNIT_ID,
                                vol_convert(spk_info->spk_left_vol),
                                vol_convert(spk_info->spk_right_vol));
                spk_info->spk_mute = 0;
            }
            break;
        case UAC_FU_VOLUME:
            memcpy((u8 *)&volume, read_ep, setup->wLength);
            if (LOBYTE(setup->wValue) == 0) {
                spk_info->spk_left_vol = volume;
                spk_info->spk_right_vol = volume;
            } else if (LOBYTE(setup->wValue) == 1) {
                spk_info->spk_left_vol = volume;
            } else if (LOBYTE(setup->wValue) == 2) {
                spk_info->spk_right_vol = volume;
            }
            if (spk_info->ps4_mode) {
                spk_info->spk_left_vol = vol_convert(100);
                spk_info->spk_right_vol = vol_convert(100);
            }

            if (!spk_info->spk_mute && !spk_info->ps4_mode) {
                uac_mute_volume(usb_id, SPK_FEATURE_UNIT_ID,
                                vol_convert(spk_info->spk_left_vol),
                                vol_convert(spk_info->spk_right_vol));
            }
            break;

        default :
            ret = USB_EP0_SET_STALL;
            break;
        }
        break;

    case SPK_SELECTOR_UNIT_ID:
        if (spk_info == NULL) {
            break;
        }
        switch (HIBYTE(setup->wValue)) {
        case 0:
            printf("SPK SELECTOR_UNIT_ID");
            spk_info->ps4_mode = read_ep[0];
            //usb_set_host_type(usb_id, HOST_TYPE_PS4);
            if (read_ep[0]) {
                spk_info->spk_left_vol = vol_convert(100);
                spk_info->spk_right_vol = vol_convert(100);
                uac_mute_volume(usb_id, SPK_FEATURE_UNIT_ID, 100, 100);
            }
            break;
        }
        break;

    case MIC_CLOCK_SOURCE_ID:
        if (mic_info == NULL) {
            break;
        }
        switch (HIBYTE(setup->wValue)) {
        case UAC2_CS_CONTROL_SAM_FREQ:
            memcpy(&mic_info->mic_sampling_rate, read_ep, 4);
            log_info("Clock Source Control Set Sampling Frequency to %d", mic_info->mic_sampling_rate);
            break;
        case UAC2_CS_CONTROL_CLOCK_VALID:
            log_info("Clock Source Control Set Valid to %d", read_ep[0]);
            break;
        }
        break;

    case MIC_FEATURE_UNIT_ID:
        if (mic_info == NULL) {
            break;
        }
        switch (HIBYTE(setup->wValue)) {
        case UAC_FU_MUTE:
            memcpy((u8 *)&mute, read_ep, 1);
            if (mute) {
                uac_mute_volume(usb_id, MIC_FEATURE_UNIT_ID, 0, 0);
                mic_info->mic_mute = 1;
            } else {
                uac_mute_volume(usb_id, MIC_FEATURE_UNIT_ID, vol_convert(mic_info->mic_vol), 0);
                mic_info->mic_mute = 0;
            }
            break;
        case UAC_FU_VOLUME:
            memcpy((u8 *)&mic_info->mic_vol, read_ep, sizeof(mic_info->mic_vol));
            //if (mic_info->mic_vol == 0x8000) {
            //    mic_info->mic_vol = 0;
            //} else if (mic_info->mic_vol >= 100) {
            //    mic_info->mic_vol = 99;
            //}
            if (!mic_info->mic_mute) {
                uac_mute_volume(usb_id, MIC_FEATURE_UNIT_ID, vol_convert(mic_info->mic_vol), 0);
            }
            break;
        case UAC_FU_AUTOMATIC_GAIN:
            if (read_ep[0]) {
                mic_info->bAGC = 1;
            } else {
                mic_info->bAGC = 0;
            }
            break;

        default :
            ret = USB_EP0_SET_STALL;
            break;
        }
        break;

    case MIC_SELECTOR_UNIT_ID:
        if (mic_info == NULL) {
            break;
        }
        switch (HIBYTE(setup->wValue)) {
        case 0:
            printf("MIC SELECTOR_UNIT_ID");
            break;
        }
        break;

    default :
        ret = USB_EP0_SET_STALL;
        break;
    }

    return ret;
}
void audio_reset(struct usb_device_t *usb_device, u32 ift_num)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    log_debug("%s", __func__);
#if USB_ROOT2
    usb_disable_ep(usb_id, SPK_ISO_EP_OUT);
    /* uac_release(usb_id); */
#else
    u8 *ep_buffer;
    if (spk_info) {
        ep_buffer = spk_info->spk_dma_buffer;
        usb_g_ep_config(usb_id, SPK_ISO_EP_OUT | USB_DIR_OUT, USB_ENDPOINT_XFER_ISOC, 1, ep_buffer, spk_get_max_frame_len());
    }

    if (mic_info) {
        ep_buffer = mic_info->mic_dma_buffer;
        usb_g_ep_config(usb_id, MIC_ISO_EP_IN | USB_DIR_IN, USB_ENDPOINT_XFER_ISOC, 1, ep_buffer, mic_get_max_frame_len());
    }
#endif
}


#endif
#endif
