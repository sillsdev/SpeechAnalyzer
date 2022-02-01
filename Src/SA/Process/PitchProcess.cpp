#include <Stdafx.h>
#include "PitchProcess.h"

CPitchProcess::State CPitchProcess::process(Parameters parameters, HPSTR(Read)(DWORD dwOffset, BOOL bBlockBegin), bool (Write)(const void* lpBuf, UINT nCount)) {

    if (parameters.dataSize==0) {
        return NoData;
    }

    progress = 0;

    // initialize parameters
    dataPos = 0;
    minValue = SHRT_MAX;

    eightbit = (int16)(parameters.smpSize == 1);
    mode = Grappl_fullpitch;;
    smoothfreq = 1000L;
    calcint = 100;                        //!!this should be based on sampling frequency
    minmeanweight = 60;
    maxinterp_pc10 = 300;
    eightbit = (int16)(parameters.smpSize == 1);
    maxchange_pc10 = int16(parameters.maxChange * 10);

    DWORD dataSize;
    DWORD bufferSize;

    // allocate buffer
    HPSTR buffer = new char[bufferSize];

    bool alldone = false;
    bool nomore = false;

    // get block size
    DWORD blockSize = 0x10000 - parameters.blockAlign;    // 64k - 1
    if (bufferSize < blockSize) {
        blockSize = bufferSize;
    }
    HPSTR pBlockStart;
    // start processing
    while (dataPos < parameters.dataSize) {
        // get raw data block
        pBlockStart = Read(dataPos, TRUE);
        if (!pBlockStart) {
            return Error;
        }
        dataPos += blockSize;
        if (dataPos >= parameters.dataSize) {
            blockSize -= (dataPos - parameters.dataSize);
            nomore = TRUE;
        }
        // set grappl input buffer
        uint16 length = (WORD)(blockSize / parameters.blockAlign);
        if (!grapplSetInbuff((void*)buffer, (pGrappl)pBlockStart, length, nomore)) {
            return Error;
        }
        // process
        pGrappl_res pResults;
        int16 nresults;

        while (grapplGetResults((void*)buffer, &pResults, &nresults, &alldone)) {
            // get max and min values and save the results
            for (int16 nLoop = 0; nLoop < nresults; nLoop++) {
                // find min and max values
                if (pResults->fcalc16 >= 0) {
                    // adjust result to precision multiplier format
                    pResults->fcalc16 = (int16)(((long)pResults->fcalc16 * (long)PRECISION_MULTIPLIER + 8L) / 16L);
                    if (pResults->fcalc16 > maxValue) {
                        maxValue = pResults->fcalc16;    // save maximum value
                    }
                    if (pResults->fcalc16 < minValue) {
                        minValue = pResults->fcalc16;    // save minimum value
                    }
                } else {
                    pResults->fcalc16 = -1;    // set this point as unset
                }
                // write one result of the processed grappl pitch data
                if (!Write((HPSTR)&pResults->fcalc16, sizeof(int16))) {
                    return Error;
                }

                pResults++;
            }
        }
        // set progress bar
        progress = (int)((100 * dataPos) / parameters.dataSize)/100;

        if (cancelled) {
            // process canceled
            return Cancelled;      
        }
        if (alldone) {
            break;
        }
    }

    if (dataSize < 2) {
        // error, not enough data
        return Error;             
    }

    if (alldone) {
        if (dataSize < 2) {
            // error, not enough data        
            //!!should be TRUE = close file?
            return Error;    
        }
        if (minValue == SHRT_MAX) {
            return NoPitch;
        }
    }
    return (nomore || alldone) ? DataReady : Error;
}
