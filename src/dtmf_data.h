#ifndef DTMF_DATA_H
#define DTMF_DATA_H

#define DTMFSampleSize 1000

struct DTMFSamples_t {
	int16_t samp[DTMFSampleSize];
};

struct DTMFResult_t {
	int16_t code;
};

#endif
