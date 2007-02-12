#define INVERSION_OFF 0
#define INVERSION_ON 1
#define INVERSION_AUTO 99

#define BANDWIDTH_6_MHZ 6
#define BANDWIDTH_7_MHZ 7
#define BANDWIDTH_8_MHZ 8
#define BANDWIDTH_AUTO 999

#define  FEC_NONE 0
#define FEC_1_2 12
#define  FEC_2_3 23
#define FEC_3_4 34
#define FEC_4_5 45
#define FEC_5_6 56
#define FEC_6_7 67
#define FEC_7_8 78
#define FEC_8_9 89
#define FEC_AUTO 999

#define QPSK 0 
#define QAM_16 16
#define QAM_32 32
#define QAM_64 64
#define QAM_128 128
#define QAM_256 256
#define QAM_AUTO 999

#define  TRANSMISSION_MODE_2K 2
#define  TRANSMISSION_MODE_8K 8
#define  TRANSMISSION_MODE_AUTO 999

#define GUARD_INTERVAL_1_4 4 
#define GUARD_INTERVAL_1_8 8
#define GUARD_INTERVAL_1_16 16
#define  GUARD_INTERVAL_1_32 32
#define GUARD_INTERVAL_AUTO 999

#define  HIERARCHY_NONE 0 
#define  HIERARCHY_1 1
#define  HIERARCHY_2 2
#define  HIERARCHY_4 4
#define  HIERARCHY_AUTO 999

#define FE_READ_STATUS		   _IOR('o', 69, fe_status_t)
#define FE_READ_BER		   _IOR('o', 70, __uint32_t)
#define FE_READ_SIGNAL_STRENGTH    _IOR('o', 71, __uint16_t)
#define FE_READ_SNR		   _IOR('o', 72, __uint16_t)
#define FE_READ_UNCORRECTED_BLOCKS _IOR('o', 73, __uint32_t)

typedef enum fe_status {
	FE_HAS_SIGNAL	= 0x01,   /*  found something above the noise level */
	FE_HAS_CARRIER	= 0x02,   /*  found a DVB signal  */
	FE_HAS_VITERBI	= 0x04,   /*  FEC is stable  */
	FE_HAS_SYNC	= 0x08,   /*  found sync bytes  */
	FE_HAS_LOCK	= 0x10,   /*  everything's working... */
	FE_TIMEDOUT	= 0x20,   /*  no lock within the last ~2 seconds */
	FE_REINIT	= 0x40    /*  frontend was reinitialized,  */
} fe_status_t;			  /*  application is recommended to reset */
				  /*  DiSEqC, tone and parameters */

typedef enum fe_type {
	FE_QPSK,
	FE_QAM,
	FE_OFDM,
	FE_ATSC
} fe_type_t;