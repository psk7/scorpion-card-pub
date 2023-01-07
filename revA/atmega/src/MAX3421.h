#ifndef CH_MAX3421_H
#define CH_MAX3421_H

#define RCVFIFO     1
#define SNDFIFO     2
#define SUDFIFO     4
#define RCBVC       6
#define SNDBC       7
#define USBCTL      15
#define PINCTL      17
#define IOPINS2     21
#define HIRQ        25
#define MODE        27
#define PERADDR     28
#define HCTL        29
#define HXFR        30
#define HRSL        31

#define HXFRDNIRQ   0x80
#define CONDETIRQ   0x20
#define SNDBAVIRQ   0x8
#define RCVDAVIRQ   0x4

#define FDUPSPI     0x10

#define CHIPRES     0x20

#define SNDTOG1     0x80
#define SNDTOG0     0x40
#define RCVTOG1     0x20
#define RCVTOG0     0x10
#define SIGRSM      0x8
#define SAMPLEBUS   0x4
#define FRMRST      0x2
#define BUSRST      0x1

enum Result {
    hrSUCCESS = 0,
    hrBUSY = 1,
    hrNAK = 4,
    hrSTALL = 5,
    hrTOGERR = 6,
    hrKERR = 12,
    hrJERR = 13,
    hrTIMEOUT = 14
};

#define DPPULLDN    0x80
#define DMPULLDN    0x40
#define DELAYISO    0x20
#define SEPIRQ      0x10
#define SOFKAENAB   0x8
#define HUBPRE      0x4
#define LOWSPEED    0x2
#define HOST        0x1

#define JSTATUS     0x80
#define KSTATUS     0x40
#define SNDTOGRD    0x20
#define RCVTOGRD    0x10

#endif //CH_MAX3421_H
