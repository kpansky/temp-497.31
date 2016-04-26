#ifndef DTMF_DATA_H
#define DTMF_DATA_H

#define DTMFSampleSize 1000
#define DTMFSampleType int16_t
#define DTMFSampleRate 100000

#define DTMF_PERIOD(x) (DTMFSampleRate / x)
#define DTMF_NO_PERIOD 0
#define DTMF_L0_PERIOD DTMF_PERIOD(697)
#define DTMF_L1_PERIOD DTMF_PERIOD(770)
#define DTMF_L2_PERIOD DTMF_PERIOD(852)
#define DTMF_L3_PERIOD DTMF_PERIOD(941)
#define DTMF_H0_PERIOD DTMF_PERIOD(1209)
#define DTMF_H1_PERIOD DTMF_PERIOD(1336)
#define DTMF_H2_PERIOD DTMF_PERIOD(1477)
#define DTMF_H3_PERIOD DTMF_PERIOD(1633)

struct DTMFSamples_t {
	DTMFSampleType samp[DTMFSampleSize];
};

struct DTMFResult_t {
	int16_t code;
};

#endif
