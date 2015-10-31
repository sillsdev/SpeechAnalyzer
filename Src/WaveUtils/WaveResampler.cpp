#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include <math.h>
#include <mmreg.h>
#include <ks.h>
#include <ksmedia.h>
#include <vector>
#include "WaveResampler.h"
#include "WaveWriter.h"
#include "WaveError.h"
#include "array_ptr.h"

#define PI  3.14159265359
#define PERCENT_TRANSITION   .10F

typedef unsigned short USHORT;

using std::vector;

double CWaveResampler::Limit(double val) {
    return (val>1.0)?1.0:((val<-1.0)?-1.0:val);
}

long CWaveResampler::Round(double val) {
    double result = val+0.5;
    return (long)result;
}

long CWaveResampler::ConvBitSize(unsigned long in, int bps) {
    const unsigned long max = (1 << (bps-1)) - 1;
    return in > max ? in - (max<<1) : in;
}

/**
* calculate greatest common denominator
*/
unsigned long CWaveResampler::GCD(unsigned long m, unsigned long n) {

    while (n != 0) {
        unsigned long r = m % n;
        m = n;
        n = r;
    }
    return m;
}

/**
* calculate least common multiple
*/
unsigned long CWaveResampler::LCM(unsigned long m, unsigned long n) {
    if ((m==0) || (n==0)) {
        return 0;
    }
    return ((m*n)/GCD(m,n));
}

double CWaveResampler::BessI0(float x) {

    double ans = 0;
    double ax=fabs(x);
    if (ax < 3.75) {
        double y=x/3.75;
        y*=y;
        ans=(double)(1.0+y*(3.5156229+y*(3.0899424+y*(1.2067492+y*(0.2659732+y*(0.360768e-1+y*0.45813e-2))))));
    } else {
        double y=3.75/ax;
        ans=(double)((exp(ax)/sqrt(ax))*(0.39894228+y*(0.1328592e-1+y*((0.225319e-2)+y*((-0.157565e-2)+y*((0.916281e-2)+y*(-0.2057706e-1+y*(0.2635537e-1+y*(-0.1647633e-1+y*0.392377e-2)))))))));
    }
    return ans;
}

void CWaveResampler::CalculateCoefficients( DWORD inSampleRate,
											WORD sampleSize, // wBitsPerSample bit size for one sample on one channel
											double *& coeffs,
											size_t & coeffsLen) {

    size_t groupDelay = 0;

    DWORD outSampleRate = 22050;

    //  Compute the Least Common Multiple (LCM) of the input and output sampling
    //  rates on which the filter will be based.  Calculate upsampling
    //  factor to scale the input to the LCM rate, the downsampling factor
    //  to scale the LCM to the output rate.
    ULONG smpRateLCM = LCM(inSampleRate,outSampleRate);
    USHORT upSmpFactor = (USHORT)(smpRateLCM/(ULONG)inSampleRate);

    //  Compute required stopband attenuation in dB to keep high frequency
    //  leakage at or below the quantization noise level for the shorter word
    //  size.
    float attenuation = 6.F*(float)sampleSize - 1.24F;

    // !!use Signal Range?
    //  Set the filter cutoff frequency to be half of the lesser of the
    //  input or output sampling frequencies, based on the Nyquist criterion.
    float cutoff = (float)min(inSampleRate,outSampleRate)/2.F;

    //  Establish a reasonable transition region based on a percentage
    //  of the passband width.
    float transition = cutoff*PERCENT_TRANSITION;

    //  Calculate Kaiser windowed filter width based on the required
    //  attenuation and transition regions for the upsampled signal.
    coeffsLen = (long)(floor((attenuation - 7.95)/ (14.36*transition/smpRateLCM) + 1.));
    coeffsLen += !(coeffsLen % 2);

    // make width odd for integral group delay
    // !! account for 3 dB shift

    //  Compute the tradeoff between a sharp transition from passband to
    //  stopband and greater attenuation.
    float tradeOff = 0;
    if (attenuation > 50.F) {
        tradeOff = 0.1102F*(attenuation - 8.7F);
    } else if (attenuation > 21.F) {
        tradeOff = 0.5842F*(float)pow((double)attenuation - 21.0,0.4) + .07886F*(attenuation - 21.0F);
    } else {
        tradeOff = 0.F;
    }

    //  Set filter starting position to zero out group delay.
    groupDelay = coeffsLen/2;

    //  Set the gain for the interpolation stage.  Gain for the decimation
    //  stage is not required, since it is built-in to the downsampling
    //  process.
    double gain = (double)upSmpFactor;

    //  Allocate storage for filter coefficients.
    coeffs = new double[coeffsLen];

    // create a filter for this case
    for (size_t i = 0; i < groupDelay; i++) {
        double p = pow((((double)i-(double)groupDelay)/(double)groupDelay),2.0);
        double b = (double)BessI0(tradeOff*(float)pow(1.0-p,0.5));
        double b2 = (double)BessI0(tradeOff);
        double s = (double)sin(2.0*PI*(double)((double)i-(double)groupDelay)*(double)cutoff/(double)smpRateLCM);
        coeffs[i] = gain*b/b2*s/(PI*(double)((double)i-(double)groupDelay));
    }
    coeffs[groupDelay] = gain*(2.*(double)cutoff/(double)(smpRateLCM));


    {
        // count and remove the leading zeros
        size_t count=0;
        size_t lastmin=0;
        for (size_t i=0; i<=groupDelay; i++) {
            // we will limit anything below
            if (abs(coeffs[i])>(1.0/65536.0)) {
                break;
            }
            if ((i>0) && (abs(coeffs[i])<abs(coeffs[i-1]))) {
                lastmin = i;
            }
            count++;
        }

        size_t j=0;
        double * temp = new double[coeffsLen-(lastmin*2)];
        for (size_t i=lastmin; i<=groupDelay; i++) {
            temp[j++]=coeffs[i];
        }
        coeffsLen = coeffsLen-(lastmin*2);
        delete [] coeffs;
        coeffs = temp;
        groupDelay -= lastmin;
    }

    double sum = 0.0;
    double min = 2;
    double max = -2;
    for (size_t i=0; i<coeffsLen/2; i++) {
        sum += coeffs[i];
        if (coeffs[i]<min) {
            min = coeffs[i];
        }
        if (coeffs[i]>max) {
            max = coeffs[i];
        }
    }

    // create the other half of the filter
    for (size_t i = groupDelay+1; i < coeffsLen; i++) {
        coeffs[i] = coeffs[coeffsLen-1-i];
    }
}

/**
using history buffers via output buffer index and threads
*/
void CWaveResampler::Func(size_t bufferLen,
						  double * out,
                          size_t coeffsLen,
                          double * coeffs,
                          size_t upSmpFactor,
                          size_t dwnSmpFactor,
						  double * in,
                          IProgressUpdate & progressUpdater) {

    size_t workIdx = 0;
    size_t historyLen = coeffsLen-1;

    int progress = progressUpdater.GetProgress();
    double remain = 95.0 - (double)progress;
    double step = remain / 10.0;
    double current = step;

    for (size_t i=0; i<bufferLen; i++) {

		out[i] = 0;

        // calculate history and current input value on the fly
        size_t coeffIdx = historyLen;

        // skip past the beginning where the history buffer is zeros
        size_t skip = (historyLen>workIdx)?(historyLen-workIdx):0;
        size_t start = 0;
        if (skip>0) {
            start += skip;
            coeffIdx -= skip;
        }

        // now calculate the index of the first valid sample
        int z = (int)workIdx-(int)historyLen+(int)skip;
        size_t remainder = z % upSmpFactor;
        size_t offset = 0;
        if (remainder>0) {
            offset = upSmpFactor-remainder;
            start += offset;
            coeffIdx -= offset;
            z += offset;
        }

        // calculate index into input buffer
        size_t quotient = (size_t)(z/upSmpFactor);

        size_t j = start;
        while (j<=historyLen) {
            // z is guaranteed to be greater than or equal to zero
            // z is guaranteed to start on a valid sample
			out[i] = out[i] + in[quotient++] * coeffs[coeffIdx];
            coeffIdx -= upSmpFactor;
            j += upSmpFactor;
        }

        // calculate how much we've completed
        double complete = ((double)i)/((double)bufferLen);
        double val = remain*complete;
        // only call the status bar once in a while
        if (val>current) {
            progressUpdater.SetProgress(progress+((int)val));
            current += step;
        }

        workIdx+=dwnSmpFactor;
    }
}

/**
* Resamples .WAV files to 16-bit data
* the acm* functions will try to convert from other compression types outside of PCM if possible.
* This method uses 32-bit floating point to do the majority of the work.
* The incoming default progress bar is 30%....
*/
CWaveResampler::ECONVERT CWaveResampler::Resample( LPCTSTR src, const TCHAR  * dst, DWORD targetSamplesPerSec, IProgressUpdate & progressUpdater) {

    // yes, I could have used smart pointers...I was in a hurry.
    //size_t length = 0;
    WORD wBitsPerSample = 0;
    WORD wFormatTag = 0;
    WORD nChannels = 0;
    DWORD nSamplesPerSec = 0;
    WORD nBlockAlign = 0;
	array_ptr<BYTE> data;

    // read in the data
    {
        //Creating new wav file.
        HMMIO hmmio = mmioOpen(const_cast<TCHAR *>(src), 0, MMIO_ALLOCBUF | MMIO_READ);
        if (hmmio==NULL) {
            return EC_NOWAVE;
        }

        MMCKINFO waveChunk;
        memset(&waveChunk,0,sizeof(waveChunk));
        /* Tell Windows to locate a WAVE Group header somewhere in the file, and read it in.
        This marks the start of any embedded WAVE format within the file */
        waveChunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
        if (mmioDescend(hmmio, (LPMMCKINFO)&waveChunk, NULL, MMIO_FINDRIFF)) {
            /* Oops! No embedded WAVE format within this file */
            mmioClose(hmmio, 0);
            return EC_NOWAVE;
        }

        MMCKINFO fmtChunk;
        memset(&fmtChunk,0,sizeof(fmtChunk));
        /* Tell Windows to locate the WAVE's "fmt " chunk and read in its header */
        fmtChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
        if (mmioDescend(hmmio, &fmtChunk, &waveChunk, MMIO_FINDCHUNK)) {
            mmioClose(hmmio, 0);
            return EC_READFAIL;
        }

        if ((fmtChunk.cksize==16)||(fmtChunk.cksize==18)) {
            /* Tell Windows to read in the "fmt " chunk into a WAVEFORMATEX structure */
            WAVEFORMATEX format;
            memset(&format,0,sizeof(format));
            if (mmioRead(hmmio, (HPSTR)&format, fmtChunk.cksize) != (LRESULT)fmtChunk.cksize) {
                mmioClose(hmmio, 0);
                return EC_READFAIL;
            }

            wFormatTag = format.wFormatTag;
            nChannels = format.nChannels;
            nSamplesPerSec = format.nSamplesPerSec;
            nBlockAlign = format.nBlockAlign;
            wBitsPerSample = format.wBitsPerSample;

        } else if (fmtChunk.cksize==40) {

            WAVEFORMATEXTENSIBLE waveInEx;
            memset(&waveInEx,0,sizeof(WAVEFORMATEXTENSIBLE));
            /* Tell Windows to read in the "fmt " chunk into a WAVEFORMATEX structure */
            if (mmioRead(hmmio, (HPSTR)&waveInEx, fmtChunk.cksize) != (LRESULT)fmtChunk.cksize) {
                mmioClose(hmmio, 0);
                return EC_READFAIL;
            }

            wFormatTag = waveInEx.Format.wFormatTag;
            nChannels = waveInEx.Format.nChannels;
            nSamplesPerSec = waveInEx.Format.nSamplesPerSec;
            nBlockAlign = waveInEx.Format.nBlockAlign;
            wBitsPerSample = waveInEx.Format.wBitsPerSample;

            if (waveInEx.SubFormat==KSDATAFORMAT_SUBTYPE_PCM) {
                wFormatTag = WAVE_FORMAT_PCM;
            } else if (waveInEx.SubFormat==KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
                wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
            }
        } else {
            mmioClose(hmmio, 0);
            return EC_READFAIL;
        }

        if (mmioAscend(hmmio, &fmtChunk, 0)) {
            mmioClose(hmmio, 0);
            return EC_READFAIL;
        }

        if (wBitsPerSample==64) {
            mmioClose(hmmio, 0);
            return EC_NOTSUPPORTED;
        }

        // read the data chunk
        MMCKINFO dataChunk;
        memset(&dataChunk,0,sizeof(dataChunk));
        /* Tell Windows to locate the WAVE's "fmt " chunk and read in its header */
        dataChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
        if (mmioDescend(hmmio, &dataChunk, NULL, MMIO_FINDCHUNK)) {
            mmioClose(hmmio, 0);
            return EC_NODATA;
        }

		data.reallocate(dataChunk.cksize);

        /** read the data */
        if (mmioRead(hmmio, (HPSTR)data.get(), dataChunk.cksize) != (LRESULT)dataChunk.cksize) {
            mmioClose(hmmio, 0);
            return EC_READFAIL;
        }

        if (mmioAscend(hmmio, &dataChunk, 0)) {
            mmioClose(hmmio, 0);
            return EC_READFAIL;
        }

        // Close the file.
        if (mmioClose(hmmio, 0)) {
            return EC_READFAIL;
        }
    }

    // convert incoming format if possible
    if ((wFormatTag!=WAVE_FORMAT_PCM) && (wFormatTag!=WAVE_FORMAT_IEEE_FLOAT)) {

        HACMSTREAM hacm = 0;
        HACMDRIVER had = NULL;

        WAVEFORMATEX fxSrc;
        memset(&fxSrc,0,sizeof(fxSrc));
        fxSrc.wFormatTag = wFormatTag;
        fxSrc.nChannels = nChannels;
        fxSrc.nSamplesPerSec = nSamplesPerSec;
        fxSrc.nAvgBytesPerSec =  nSamplesPerSec*nBlockAlign;
        fxSrc.nBlockAlign = nBlockAlign;
        fxSrc.wBitsPerSample = wBitsPerSample;

        WAVEFORMATEX fxDst;
        memcpy(&fxDst,&fxSrc,sizeof(WAVEFORMATEX));

        LPWAVEFILTER pwfltr = NULL;
        DWORD_PTR dwCallback = 0;
        DWORD_PTR dwInstance = 0;
        DWORD fdwOpen = ACM_STREAMOPENF_NONREALTIME;
        if (acmStreamOpen(&hacm,had,&fxSrc,&fxDst,pwfltr,dwCallback,dwInstance,fdwOpen)) {
            return EC_CONVERTFORMATFAIL;
        }

		DWORD cbInput = data.size();
        DWORD outputBytes = 0;
        DWORD fdwSize = ACM_STREAMSIZEF_SOURCE;
        if (acmStreamSize(hacm,cbInput,&outputBytes,fdwSize)) {
            acmStreamClose(hacm,0);
            return EC_CONVERTFORMATFAIL;
        }

        ACMSTREAMHEADER ash;
        memset(&ash,0,sizeof(ACMSTREAMHEADER));
        ash.cbStruct = sizeof(ACMSTREAMHEADER);
        ash.fdwStatus = 0;
        ash.dwUser = NULL;
        ash.pbSrc = data.get();
        ash.cbSrcLength = data.size();
        ash.cbSrcLengthUsed = 0;
        ash.dwSrcUser = NULL;
        ash.pbDst = new BYTE[outputBytes];
        ash.cbDstLength = outputBytes;
        ash.cbDstLengthUsed = 0;
        ash.dwDstUser = NULL;

        if (acmStreamPrepareHeader(hacm,&ash,0)) {
            acmStreamClose(hacm,0);
            return EC_CONVERTFORMATFAIL;
        }

        if (acmStreamConvert(hacm,&ash,0)) {
            acmStreamClose(hacm,0);
            return EC_CONVERTFORMATFAIL;
        }

        if (acmStreamUnprepareHeader(hacm,&ash,0)) {
            acmStreamClose(hacm,0);
            return EC_CONVERTFORMATFAIL;
        }

        /* format type */
        wFormatTag = fxDst.wFormatTag;
        /* number of channels (i.e. mono, stereo...) */
        nChannels = fxDst.nChannels;
        /* sample rate */
        nSamplesPerSec = fxDst.nSamplesPerSec;
        /* block size of data */
        nBlockAlign = fxDst.nBlockAlign;
        /* number of bits per sample of mono data */
        wBitsPerSample = fxDst.wBitsPerSample;

		data.reassign( ash.pbDst,ash.cbDstLengthUsed);

        if (acmStreamClose(hacm,0)) {
            return EC_CONVERTFORMATFAIL;
        }
    }

    if (data.get()==NULL) {
        return EC_SOFTWARE;
    }

    // convert everything to 32-bit float
    size_t numSamples = data.size()/nBlockAlign;
	// allocate a buffer for all channels of data
	// we will reindex the data so that the channel data is sequential
	array_ptr<double> dataa(numSamples*nChannels);
    {

        size_t k = 0;
        if (wFormatTag==WAVE_FORMAT_PCM) {

            // if it's 32 bit, we also don't need to do this
            switch (wBitsPerSample) {
            case 16: {
                for (size_t i=0; i<numSamples; i++) {
                    for (unsigned int c=0; c<nChannels; c++) {
                        unsigned long b0 = data[k++];
                        unsigned long b1 = data[k++];
                        unsigned long val = b1;
                        val = val<<8L;
                        val += b0;
                        long result = ConvBitSize(val,16);
                        result *= 0x10000;
                        double temp = result;
                        temp /= (double)0x7fffffff;
                        temp = Limit(temp);
                        dataa[(c*numSamples)+i] = temp;
                    }
                }
            }
            break;
            case 24: {
                for (size_t i=0; i<numSamples; i++) {
                    for (unsigned int c=0; c<nChannels; c++) {
                        unsigned long b0 = data[k++];
                        unsigned long b1 = data[k++];
                        unsigned long b2 = data[k++];
                        unsigned long val = b2;
                        val = val<<8L;
                        val += b1;
                        val = val<<8L;
                        val += b0;
                        long result = ConvBitSize(val,24);
                        result *= 0x100;
                        double temp = result;
                        temp /= (double)0x7fffffff;
                        temp = Limit(temp);
                        dataa[(c*numSamples)+i] = temp;
                    }
                }
            }
            break;
            case 32: {
                for (size_t i=0; i<numSamples; i++) {
                    for (unsigned int c=0; c<nChannels; c++) {
                        unsigned long b0 = data[k++];
                        unsigned long b1 = data[k++];
                        unsigned long b2 = data[k++];
                        unsigned long b3 = data[k++];
                        unsigned long val = b3;
                        val = val<<8L;
                        val += b2;
                        val = val<<8L;
                        val += b1;
                        val = val<<8L;
                        val += b0;
                        long result = ConvBitSize(val,32);
                        double temp = result;
                        temp /= (double)0x7fffffff;
                        temp = Limit(temp);
                        dataa[(c*numSamples)+i] = temp;
                    }
                }
            }
            break;
            default:
                return EC_WRONGFORMAT;
            }
        } else if (wFormatTag==WAVE_FORMAT_IEEE_FLOAT) {
            // assume float
            switch (wBitsPerSample) {
            case 32: {
                float * samples = (float *)data.get();
                for (size_t i=0; i<numSamples; i++) {
                    int z = i*nChannels;
                    for (unsigned int c=0; c<nChannels; c++) {
                        double temp = samples[z+c];
                        temp = Limit(temp);
                        dataa[(c*numSamples)+i] = temp;
                    }
                }
            }
            break;
            default:
                return EC_WRONGFORMAT;
            }
        } else {
            return EC_WRONGFORMAT;
        }
    }

    // at this point we have 32-bit PCM only
    wBitsPerSample = 32;
    nBlockAlign = 4*nChannels;
    wFormatTag = WAVE_FORMAT_PCM;

    // at this point the old data pointer is no longer used...
    if (dataa.get()==NULL) {
        // shouldn't be!
        return EC_SOFTWARE;
    }

    /**
    * we will be converting whatever the input format
    * is into a 22khz, float sample

    Initialize filter if sample rate conversion requested.  Otherwise,
    if input and output file formats and sample word sizes are the same
    indicate no conversion of any kind is required.
    Allocate input and output buffers for processing.

    Converts from one sampling rate to another.
    In theory, the algorithm proceeds as follows:

    1.  Up-sample the original signal to the Least Common Multiple
    of the two sampling rates, padding in between sample values
    with zeros.
    2.  Smooth the signal with a Kaiser windowed low pass FIR filter
    having a cutoff at half the lesser of the original and target
    sampling frequencies and a stop-band rejection at or below the
    full scale signal to quantization noise ratio of the shorter
    of the input and output word lengths.
    3.  Down-sample to the requested sampling frequency.

    In practice, the steps are combined to minimize storage and expedite
    processing.  This is accomplished by filtering only non-zero values
    of the up-sampled signal at intervals of the down-sampling rate.

    The function returns after a specified percentage of output samples,
    designated by ProgIntv, are assembled in an output buffer and
    saved to disk. The function should be called repeatedly until 100%
    of the expected number of output samples has been calculated or a
    fatal error has occurred.
    */

    // is there anything to do?
	bool initial = true;
	array_ptr<double> datab;

    if ((nSamplesPerSec!=targetSamplesPerSec)) {
        for (int ch=0; ch<nChannels; ch++) {
			// data is already in float
            // convert to even number of samples (why?)
            size_t numSamples = dataa.size()/nChannels;
            if ((numSamples-((numSamples/2)*2))>0)
            {
                numSamples--;
            }

            DWORD inSampleRate = nSamplesPerSec;
            DWORD outSampleRate = targetSamplesPerSec;

            // calculate lcm and size of buffer
            unsigned long smpRateLCM = LCM(inSampleRate,outSampleRate);
            size_t upSmpFactor = (smpRateLCM/(ULONG)inSampleRate);
            size_t dwnSmpFactor = (smpRateLCM/(ULONG)outSampleRate);

            // calculate the new work buffer size;
            size_t workLen = numSamples*upSmpFactor;

            // create the output buffer
            size_t bufferLen = workLen/dwnSmpFactor;
            if (bufferLen > 0x7fffffff) {
                return EC_TOOLARGE;
            }

			if (initial) {
				datab.reallocate(bufferLen*nChannels);
				initial = false;
			}

            {
                // build the filter
                size_t coeffsLen = 0;
                double * coeffs = NULL;
                CalculateCoefficients( nSamplesPerSec, wBitsPerSample, coeffs, coeffsLen);
                // do the work!
				size_t data3Offset = (datab.size()/nChannels)*ch;
				size_t datalOffset = (dataa.size()/nChannels)*ch;
                Func( bufferLen, &datab[data3Offset], coeffsLen, coeffs, upSmpFactor, dwnSmpFactor, &dataa[datalOffset], progressUpdater);
                delete [] coeffs;
            }
        }
    } else {
		// do a straight copy of the data
		datab.transfer(dataa);
	}

    nSamplesPerSec = targetSamplesPerSec;

    // we now use the data at whatever frequency it comes in at

    progressUpdater.SetProgress(95);

    // convert the data to 16-bit for all channels
    {
        vector<char> buffer;
		size_t numSamples = datab.size()/nChannels;
        for (size_t i=0; i<numSamples; i++) {
            for (size_t ch=0; ch<nChannels; ch++) {
                double dval = datab[(ch*numSamples)+i];
                dval = Limit(dval);
                long lval = (long)(dval*(double)0x7fffffff);
                lval /= 0x10000;
                __int16 ival = (__int16)lval;
                BYTE lb = ival & 0xff;
                BYTE hb = ival>>8;
                buffer.push_back(lb);
                buffer.push_back(hb);
            }
        }

        CWaveWriter writer;
        try {
            writer.Write(dst, MMIO_CREATE|MMIO_WRITE|MMIO_EXCLUSIVE, 16, wFormatTag, nChannels, nSamplesPerSec, buffer);
        } catch (wave_error & /*e*/) {
            return EC_WRITEFAIL;
        }
    }

    progressUpdater.SetProgress(100);

    return EC_SUCCESS;
}

/**
* Resamples .WAV files from another sampling rate to 22050 khz
* the acm* functions will try to convert from other compression types outside of
* PCM if possible.
* This method uses 32-bit floating point to do the majority of the work.
* The incoming default progress bar is 30%....
*/
CWaveResampler::ECONVERT CWaveResampler::Normalize(LPCTSTR src, const TCHAR  * dst, IProgressUpdate & progressUpdater) {

    // yes, I could have used smart pointers...I was in a hurry.
    WORD wBitsPerSample = 0;
    WORD wFormatTag = 0;
    WORD nChannels = 0;
    DWORD nSamplesPerSec = 0;
    WORD nBlockAlign = 0;

    // read in the data
	array_ptr<BYTE> data;
    {
        //Creating new wav file.
        HMMIO hmmio = mmioOpen(const_cast<TCHAR *>(src), 0, MMIO_ALLOCBUF | MMIO_READ);
        if (hmmio==NULL) {
            return EC_NOWAVE;
        }

        MMCKINFO waveChunk;
        memset(&waveChunk,0,sizeof(waveChunk));
        /* Tell Windows to locate a WAVE Group header somewhere in the file, and read it in.
        This marks the start of any embedded WAVE format within the file */
        waveChunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
        if (mmioDescend(hmmio, (LPMMCKINFO)&waveChunk, NULL, MMIO_FINDRIFF)) {
            /* Oops! No embedded WAVE format within this file */
            mmioClose(hmmio, 0);
            return EC_NOWAVE;
        }

        MMCKINFO fmtChunk;
        memset(&fmtChunk,0,sizeof(fmtChunk));
        /* Tell Windows to locate the WAVE's "fmt " chunk and read in its header */
        fmtChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
        if (mmioDescend(hmmio, &fmtChunk, &waveChunk, MMIO_FINDCHUNK)) {
            mmioClose(hmmio, 0);
            return EC_READFAIL;
        }

        if ((fmtChunk.cksize==16)||(fmtChunk.cksize==18)) {
            /* Tell Windows to read in the "fmt " chunk into a WAVEFORMATEX structure */
            WAVEFORMATEX format;
            memset(&format,0,sizeof(format));
            if (mmioRead(hmmio, (HPSTR)&format, fmtChunk.cksize) != (LRESULT)fmtChunk.cksize) {
                mmioClose(hmmio, 0);
                return EC_READFAIL;
            }

            wFormatTag = format.wFormatTag;
            nChannels = format.nChannels;
            nSamplesPerSec = format.nSamplesPerSec;
            nBlockAlign = format.nBlockAlign;
            wBitsPerSample = format.wBitsPerSample;

        } else if (fmtChunk.cksize==40) {

            WAVEFORMATEXTENSIBLE waveInEx;
            memset(&waveInEx,0,sizeof(WAVEFORMATEXTENSIBLE));
            /* Tell Windows to read in the "fmt " chunk into a WAVEFORMATEX structure */
            if (mmioRead(hmmio, (HPSTR)&waveInEx, fmtChunk.cksize) != (LRESULT)fmtChunk.cksize) {
                mmioClose(hmmio, 0);
                return EC_READFAIL;
            }

            wFormatTag = waveInEx.Format.wFormatTag;
            nChannels = waveInEx.Format.nChannels;
            nSamplesPerSec = waveInEx.Format.nSamplesPerSec;
            nBlockAlign = waveInEx.Format.nBlockAlign;
            wBitsPerSample = waveInEx.Format.wBitsPerSample;

            if (waveInEx.SubFormat==KSDATAFORMAT_SUBTYPE_PCM) {
                wFormatTag = WAVE_FORMAT_PCM;
            } else if (waveInEx.SubFormat==KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
                wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
            }
        } else {
            mmioClose(hmmio, 0);
            return EC_READFAIL;
        }

        if (mmioAscend(hmmio, &fmtChunk, 0)) {
            mmioClose(hmmio, 0);
            return EC_READFAIL;
        }

        if (wBitsPerSample==64) {
            mmioClose(hmmio, 0);
            return EC_NOTSUPPORTED;
        }

        // read the data chunk
        MMCKINFO dataChunk;
        memset(&dataChunk,0,sizeof(dataChunk));
        /* Tell Windows to locate the WAVE's "fmt " chunk and read in its header */
        dataChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
        if (mmioDescend(hmmio, &dataChunk, NULL, MMIO_FINDCHUNK)) {
            mmioClose(hmmio, 0);
            return EC_NODATA;
        }

		data.reallocate(dataChunk.cksize);

        /** read the data */
        if (mmioRead(hmmio, (HPSTR)data.get(), dataChunk.cksize) != (LRESULT)dataChunk.cksize) {
            mmioClose(hmmio, 0);
            return EC_READFAIL;
        }

        if (mmioAscend(hmmio, &dataChunk, 0)) {
            mmioClose(hmmio, 0);
            return EC_READFAIL;
        }

        // Close the file.
        if (mmioClose(hmmio, 0)) {
            return EC_READFAIL;
        }
    }

    // convert incoming format if possible
    if ((wFormatTag!=WAVE_FORMAT_PCM) && (wFormatTag!=WAVE_FORMAT_IEEE_FLOAT)) {

        HACMSTREAM hacm = 0;
        HACMDRIVER had = NULL;

        WAVEFORMATEX fxSrc;
        memset(&fxSrc,0,sizeof(fxSrc));
        fxSrc.wFormatTag = wFormatTag;
        fxSrc.nChannels = nChannels;
        fxSrc.nSamplesPerSec = nSamplesPerSec;
        fxSrc.nAvgBytesPerSec =  nSamplesPerSec*nBlockAlign;
        fxSrc.nBlockAlign = nBlockAlign;
        fxSrc.wBitsPerSample = wBitsPerSample;

        WAVEFORMATEX fxDst;
        memcpy(&fxDst,&fxSrc,sizeof(WAVEFORMATEX));

        LPWAVEFILTER pwfltr = NULL;
        DWORD_PTR dwCallback = 0;
        DWORD_PTR dwInstance = 0;
        DWORD fdwOpen = ACM_STREAMOPENF_NONREALTIME;
        if (acmStreamOpen(&hacm,had,&fxSrc,&fxDst,pwfltr,dwCallback,dwInstance,fdwOpen)) {
            return EC_CONVERTFORMATFAIL;
        }

        DWORD cbInput = data.size();
        DWORD outputBytes = 0;
        DWORD fdwSize = ACM_STREAMSIZEF_SOURCE;
        if (acmStreamSize(hacm,cbInput,&outputBytes,fdwSize)) {
            acmStreamClose(hacm,0);
            return EC_CONVERTFORMATFAIL;
        }

        ACMSTREAMHEADER ash;
        memset(&ash,0,sizeof(ACMSTREAMHEADER));
        ash.cbStruct = sizeof(ACMSTREAMHEADER);
        ash.fdwStatus = 0;
        ash.dwUser = NULL;
        ash.pbSrc = data.get();
        ash.cbSrcLength = data.size();
        ash.cbSrcLengthUsed = 0;
        ash.dwSrcUser = NULL;
        ash.pbDst = new BYTE[outputBytes];
        ash.cbDstLength = outputBytes;
        ash.cbDstLengthUsed = 0;
        ash.dwDstUser = NULL;

        if (acmStreamPrepareHeader(hacm,&ash,0)) {
            acmStreamClose(hacm,0);
            return EC_CONVERTFORMATFAIL;
        }

        if (acmStreamConvert(hacm,&ash,0)) {
            acmStreamClose(hacm,0);
            return EC_CONVERTFORMATFAIL;
        }

        if (acmStreamUnprepareHeader(hacm,&ash,0)) {
            acmStreamClose(hacm,0);
            return EC_CONVERTFORMATFAIL;
        }

        /* format type */
        wFormatTag = fxDst.wFormatTag;
        /* number of channels (i.e. mono, stereo...) */
        nChannels = fxDst.nChannels;
        /* sample rate */
        nSamplesPerSec = fxDst.nSamplesPerSec;
        /* block size of data */
        nBlockAlign = fxDst.nBlockAlign;
        /* number of bits per sample of mono data */
        wBitsPerSample = fxDst.wBitsPerSample;

		data.reassign(ash.pbDst,ash.cbDstLengthUsed);

        if (acmStreamClose(hacm,0)) {
            return EC_CONVERTFORMATFAIL;
        }
    }

    if (data.get()==NULL) {
        return EC_SOFTWARE;
    }

    // convert everything to 32-bit float
    size_t numSamples = data.size()/nBlockAlign;
	array_ptr<double> dataa(numSamples*nChannels);
    {

        size_t k = 0;
        if (wFormatTag==WAVE_FORMAT_PCM) {

            // if it's 32 bit, we also don't need to do this
            switch (wBitsPerSample) {
            case 16: {
                for (size_t i=0; i<numSamples; i++) {
                    for (unsigned int c=0; c<nChannels; c++) {
                        unsigned long b0 = data[k++];
                        unsigned long b1 = data[k++];
                        unsigned long val = b1;
                        val = val<<8L;
                        val += b0;
                        long result = ConvBitSize(val,16);
                        result *= 0x10000;
                        double temp = result;
                        temp /= (double)0x7fffffff;
                        temp = Limit(temp);
                        dataa[(numSamples*c)+i]=temp;
                    }
                }
            }
            break;
            case 24: {
                for (size_t i=0; i<numSamples; i++) {
                    for (unsigned int c=0; c<nChannels; c++) {
                        unsigned long b0 = data[k++];
                        unsigned long b1 = data[k++];
                        unsigned long b2 = data[k++];
                        unsigned long val = b2;
                        val = val<<8L;
                        val += b1;
                        val = val<<8L;
                        val += b0;
                        long result = ConvBitSize(val,24);
                        result *= 0x100;
                        double temp = result;
                        temp /= (double)0x7fffffff;
                        temp = Limit(temp);
                        dataa[(numSamples*c)+i]=temp;
                    }
                }
            }
            break;
            case 32: {
                for (size_t i=0; i<numSamples; i++) {
                    for (unsigned int c=0; c<nChannels; c++) {
                        unsigned long b0 = data[k++];
                        unsigned long b1 = data[k++];
                        unsigned long b2 = data[k++];
                        unsigned long b3 = data[k++];
                        unsigned long val = b3;
                        val = val<<8L;
                        val += b2;
                        val = val<<8L;
                        val += b1;
                        val = val<<8L;
                        val += b0;
                        long result = ConvBitSize(val,32);
                        double temp = result;
                        temp /= (double)0x7fffffff;
                        temp = Limit(temp);
                        dataa[(numSamples*c)+i]=temp;
                    }
                }
            }
            break;
            default:
                return EC_WRONGFORMAT;
            }
        } else if (wFormatTag==WAVE_FORMAT_IEEE_FLOAT) {
            // assume float
            switch (wBitsPerSample) {
            case 32: {
                float * samples = (float *)data.get();
                for (size_t i=0; i<numSamples; i++) {
                    int z = i*nChannels;
                    for (unsigned int c=0; c<nChannels; c++) {
                        double temp = samples[z+c];
                        temp = Limit(temp);
                        dataa[(numSamples*c)+i] = temp;
                    }
                }
            }
            break;
            default:
                return EC_WRONGFORMAT;
            }
        } else {
            return EC_WRONGFORMAT;
        }
    }

    // at this point we have 32-bit PCM only
    wBitsPerSample = 32;
    nBlockAlign = 4*nChannels;
    wFormatTag = WAVE_FORMAT_PCM;

    // at this point the old data pointer is no longer used...

    if (dataa.get()==NULL) {
        // shouldn't be!
        return EC_SOFTWARE;
    }

    /**
    * we will be converting whatever the input format
    * is into a 22khz, float sample

    Initialize filter if sample rate conversion requested.  Otherwise,
    if input and output file formats and sample word sizes are the same
    indicate no conversion of any kind is required.
    Allocate input and output buffers for processing.

    Converts from one sampling rate to another.
    In theory, the algorithm proceeds as follows:

    1.  Up-sample the original signal to the Least Common Multiple
    of the two sampling rates, padding in between sample values
    with zeros.
    2.  Smooth the signal with a Kaiser windowed low pass FIR filter
    having a cutoff at half the lesser of the original and target
    sampling frequencies and a stop-band rejection at or below the
    full scale signal to quantization noise ratio of the shorter
    of the input and output word lengths.
    3.  Down-sample to the requested sampling frequency.

    In practice, the steps are combined to minimize storage and expedite
    processing.  This is accomplished by filtering only non-zero values
    of the up-sampled signal at intervals of the down-sampling rate.

    The function returns after a specified percentage of output samples,
    designated by ProgIntv, are assembled in an output buffer and
    saved to disk. The function should be called repeatedly until 100%
    of the expected number of output samples has been calculated or a
    fatal error has occurred.
    */
    // we now use the data at whatever frequency it comes in at

    progressUpdater.SetProgress(95);

    {
        vector<char> buffer;
        for (size_t i=0; i<numSamples; i++) {
            for (size_t ch=0; ch<nChannels; ch++) {
                double dval = dataa[(ch*numSamples)+i];
                dval = Limit(dval);
                long lval = (long)(dval*(double)0x7fffffff);
                lval /= 0x10000;
                __int16 ival = (__int16)lval;
                BYTE lb = ival & 0xff;
                BYTE hb = ival>>8;
                buffer.push_back(lb);
                buffer.push_back(hb);
            }
        }

        CWaveWriter writer;
        try {
            writer.Write(dst, MMIO_CREATE|MMIO_WRITE|MMIO_EXCLUSIVE, 16, wFormatTag, nChannels, nSamplesPerSec, buffer);
        } catch (wave_error & /*e*/) {
            return EC_WRITEFAIL;
        }
    }

    progressUpdater.SetProgress(100);

    return EC_SUCCESS;
}

