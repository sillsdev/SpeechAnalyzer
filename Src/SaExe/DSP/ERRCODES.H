#ifndef ERR_CODES_H
 #define ERR_CODES_H
 

 #define MAX_WARN_CODE               7
 #define UNDEFINED_WARN_CODE         7
 #define INFINITY                    6
 #define NO_ROOT_FOUND               5
 #define HIST_BUFFER_CALLBACK        4
 #define FRAG_BUFFER_FULL            3 
 #define PITCH_BUFFER_CALLBACK       2
 #define WAVE_BUFFER_CALLBACK        1

 #define DONE                        0

 #define SYNTAX_ERR                 -1     //for main test driver
 #define NOT_READY                  -2
 #define OUT_OF_SEQUENCE            -3
 #define INVALID_STATE              -4
 #define OUT_OF_MEMORY              -5     // insufficient memory to create object
 #define INVALID_PARM               -6
 #define INVALID_PARM_PTR           -6     // NULL pointer specified
 #define INVALID_BFR_PTR            -6
 #define INVALID_SIG_DATA_PTR       -6
 #define INVALID_BANDWIDTH          -7
 #define INVALID_PREEMPH_OPTION     -8
 #define INVALID_FMNTTRACK_OPTION   -8
 #define INVALID_SIG_LEN            -9     // invalid signal length (in samples) specified
 #define INVALID_DATA_LEN           -9
 #define INVALID_BLOCK              -9     // invalid process block offset
 #define INVALID_BLOCK_LEN          -9     // invalid no. of samples to process specified
 #define INSUFF_SMP_DATA            -9
 #define INVALID_SMP_DATA_FMT      -10
 #define UNSUPP_SMP_DATA_FMT       -11     // unsupported sample data format specified
 #define UNSUPP_SMP_RATE           -12     // unsupported sampling rate specified
 #define INVALID_SMP_RATE          -13
 #define INVALID_FREQ              -14
 #define INVALID_SPGM_HGT          -15
 #define INVALID_NUM_SPECTRA       -16
 #define INVALID_BATCH_SIZE        -17
 #define INVALID_SPGM_HND          -18
 #define INVALID_WIN_HND           -18
 #define INVALID_SPGM_DATA_FMT     -19
 #define INVALID_SPGM_COORD        -20
 #define INVALID_WIN_LEN           -21
 #define INVALID_GAIN              -22
 #define INVALID_ATTEN             -23
 #define INVALID_SPECT_LEN         -24
 #define INVALID_SCALE             -25
 #define INVALID_RESOLUTION        -26
 #define INVALID_SWITCH_STATE      -27
 #define INVALID_PITCH_CONTOUR_LEN -28     // no length specified for pitch contour
 #define INVALID_SMP_INTERVAL      -29     // no sample interval specified
 #define INVALID_SCALE_FACTOR      -30     // no scale factor specified
 #define INVALID_RANGE             -31
 #define INVALID_FRAME_LEN         -32
 #define INVALID_NUM_STAGES        -33
 #define INVALID_LPC_METHOD        -34
 #define INVALID_FRAME_PTR         -35
 #define INVALID_WARP_SPEED        -36
 #define FRAGMENT_NOT_FOUND        -37
 #define INVALID_BFR_SIZE          -38     // invalid buffer size (in bytes) specified
 #define INVALID_BIN_COUNT         -39     // bin count is zero
 #define INVALID_BIN_DIVS          -40     // bin divisions not in numerical order
 #define INVALID_FFT_LENGTH        -41
 #define INVALID_NUM_PEAKS         -42
 #define INVALID_NUM_FORMANTS      -42 
 #define BUMP_NOT_FOUND            -43
 #define INVALID_MAGNITUDE         -44
 #define FAILED_TO_CONVERGE        -45
 #define INVALID_ORDER             -46
 
 #define UNDEFINED_ERR_CODE        -47
 #define MAX_ERR_CODE               47
 
 #endif
