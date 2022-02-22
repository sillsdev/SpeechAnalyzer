#include "WaveReader.h"
#include "WaveError.h"
#include <stdexcept>
#include <MMReg.h>

#ifndef KSDATAFORMAT_SUBTYPE_PCM
#define KSDATAFORMAT_SUBTYPE_PCM
#endif

#ifndef KSDATAFORMAT_SUBTYPE_IEEE_FLOAT
#define KSDATAFORMAT_SUBTYPE_IEEE_FLOAT
#endif

using std::logic_error;

CWaveReader::CWaveReader() {
    hmmio = NULL;
}

CWaveReader::~CWaveReader() {
    if (hmmio!=NULL) {
        mmioClose(hmmio, 0);
        hmmio = NULL;
    }
}

/**
*
* Reads the format chunk
*
* Number of Channels
* The number of channels specifies how many separate audio signals that are encoded in the wave data chunk.
* A value of 1 means a mono signal, a value of 2 means a stereo signal, etc.
*
* Sample Rate
* The number of sample slices per second. This value is unaffected by the number of channels.
*
* Average Bytes Per Second
* This value indicates how many bytes of wave data must be streamed to a D/A converter per second in order to
* play the wave file. This information is useful when determining if data can be streamed from the source fast
* enough to keep up with playback. This value can be easily calculated with the formula:
*
* AvgBytesPerSec = SampleRate * BlockAlign
*
* Block Align
* The number of bytes per sample slice. This value is not affected by the number of channels and can be calculated
* with the formula:
*
* BlockAlign = SignificantBitsPerSample / 8 * NumChannels
*
* Significant Bits Per Sample
* This value specifies the number of bits used to define each sample. This value is usually 8, 16, 24 or 32.
* If the number of bits is not byte aligned (a multiple of 8) then the number of bytes used per sample is rounded
* up to the nearest byte size and the unused bytes are set to 0 and ignored.
*
*/
/**
* expects the buffer to be preallocated and match datachunk size
*/
void CWaveReader::Read(LPCTSTR afilename, DWORD flags, WORD & bitsPerSample, WORD & formatTag, WORD & channels, DWORD & samplesPerSec, WORD & blockAlign, vector<char> & buffer) {

    // empty the buffer
    buffer.clear();

    wchar_t filename[MAX_PATH];
    memset(filename,0,_countof(filename));
    wcscpy_s(filename,_countof(filename),afilename);

    // open existing file
    hmmio = mmioOpen(filename, 0, flags);
    if (hmmio==NULL) {
        throw wave_error(cant_open_file_for_reading);
    }

    /* Tell Windows to locate a WAVE Group header somewhere in the file, and read it in.
    This marks the start of any embedded WAVE format within the file */
    MMCKINFO riffChunk;
    memset(&riffChunk,0,sizeof(riffChunk));
    riffChunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
    if (mmioDescend(hmmio, (LPMMCKINFO)&riffChunk, NULL, MMIO_FINDRIFF)) {
        throw wave_error(no_riff_chunk);
    }

    bitsPerSample = 0;
    formatTag = 0;
    channels = 0;
    samplesPerSec = 0;
    blockAlign = 0;

    /* Tell Windows to locate the WAVE's "fmt " chunk and read in its header */
    MMCKINFO fmtChunk;
    memset(&fmtChunk,0,sizeof(fmtChunk));
    fmtChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
    if (mmioDescend(hmmio, &fmtChunk, &riffChunk, MMIO_FINDCHUNK)) {
        throw wave_error(no_format_chunk);
    }

    if ((fmtChunk.cksize==16)||(fmtChunk.cksize==18)) {
        /* Tell Windows to read in the "fmt " chunk into a WAVEFORMATEX structure */
        WAVEFORMATEX format;
        memset(&format,0,sizeof(format));
        if (mmioRead(hmmio, (HPSTR)&format, fmtChunk.cksize) != (LRESULT)fmtChunk.cksize) {
            throw wave_error(format_chunk_read_failure);
        }

        formatTag = format.wFormatTag;
        channels = format.nChannels;
        samplesPerSec = format.nSamplesPerSec;
        blockAlign = format.nBlockAlign;
        bitsPerSample = format.wBitsPerSample;

    } else if (fmtChunk.cksize==40) {

        WAVEFORMATEXTENSIBLE waveInEx;
        memset(&waveInEx,0,sizeof(WAVEFORMATEXTENSIBLE));
        /* Tell Windows to read in the "fmt " chunk into a WAVEFORMATEX structure */
        if (mmioRead(hmmio, (HPSTR)&waveInEx, fmtChunk.cksize) != (LRESULT)fmtChunk.cksize) {
            throw wave_error(format_chunk_read_failure);
        }

        formatTag = waveInEx.Format.wFormatTag;
        channels = waveInEx.Format.nChannels;
        samplesPerSec = waveInEx.Format.nSamplesPerSec;
        blockAlign = waveInEx.Format.nBlockAlign;
        bitsPerSample = waveInEx.Format.wBitsPerSample;

        if (waveInEx.SubFormat==KSDATAFORMAT_SUBTYPE_PCM) {
            formatTag = WAVE_FORMAT_PCM;
        } else if (waveInEx.SubFormat==KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
            formatTag = WAVE_FORMAT_IEEE_FLOAT;
        }
    } else {
        throw wave_error(unsupported_format_chunk_size);
    }

    if (mmioAscend(hmmio, &fmtChunk, 0)) {
        throw wave_error(cant_ascend_from_format_chunk);
    }

    // read the data chunk
    MMCKINFO dataChunk;
    memset(&dataChunk,0,sizeof(dataChunk));
    /* Tell Windows to locate the WAVE's "fmt " chunk and read in its header */
    dataChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
    if (mmioDescend(hmmio, &dataChunk, NULL, MMIO_FINDCHUNK)) {
        throw wave_error(no_data_chunk);
    }

    DWORD length = dataChunk.cksize;

    buffer.resize(length);

    ULONG read = mmioRead(hmmio, &buffer[0], length);
    if (read!=length) {
        throw wave_error(data_read_failure);
    }

    if (mmioAscend(hmmio, &dataChunk, 0)) {
        throw wave_error(cant_ascend_from_data_chunk);
    }

    if (hmmio!=NULL) {
        mmioClose(hmmio, 0);
        hmmio = NULL;
    }
}

/**
*
* Reads the format chunk and retreive wave file information
* This function does not read the data
*
* Number of Channels
* The number of channels specifies how many separate audio signals that are encoded in the wave data chunk.
* A value of 1 means a mono signal, a value of 2 means a stereo signal, etc.
*
* Sample Rate
* The number of sample slices per second. This value is unaffected by the number of channels.
*
* Average Bytes Per Second
* This value indicates how many bytes of wave data must be streamed to a D/A converter per second in order to
* play the wave file. This information is useful when determining if data can be streamed from the source fast
* enough to keep up with playback. This value can be easily calculated with the formula:
*
* AvgBytesPerSec = SampleRate * BlockAlign
*
* Block Align
* The number of bytes per sample slice. This value is not affected by the number of channels and can be calculated
* with the formula:
*
* BlockAlign = SignificantBitsPerSample / 8 * NumChannels
*
* Significant Bits Per Sample
* This value specifies the number of bits used to define each sample. This value is usually 8, 16, 24 or 32.
* If the number of bits is not byte aligned (a multiple of 8) then the number of bytes used per sample is rounded
* up to the nearest byte size and the unused bytes are set to 0 and ignored.
*
*/
/**
* expects the buffer to be preallocated and match datachunk size
*/
void CWaveReader::Read(LPCTSTR afilename, DWORD flags, WORD & bitsPerSample, WORD & formatTag, WORD & channels, DWORD & samplesPerSec, WORD & blockAlign) {

    wchar_t filename[MAX_PATH];
    memset(filename,0,_countof(filename));
    wcscpy_s(filename,_countof(filename),afilename);

    // open existing file
    hmmio = mmioOpen(filename, 0, flags);
    if (hmmio==NULL) {
        throw wave_error(cant_open_file_for_reading);
    }

    /* Tell Windows to locate a WAVE Group header somewhere in the file, and read it in.
    This marks the start of any embedded WAVE format within the file */
    MMCKINFO riffChunk;
    memset(&riffChunk,0,sizeof(riffChunk));
    riffChunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
    if (mmioDescend(hmmio, (LPMMCKINFO)&riffChunk, NULL, MMIO_FINDRIFF)) {
        throw wave_error(no_riff_chunk);
    }

    bitsPerSample = 0;
    formatTag = 0;
    channels = 0;
    samplesPerSec = 0;
    blockAlign = 0;

    /* Tell Windows to locate the WAVE's "fmt " chunk and read in its header */
    MMCKINFO fmtChunk;
    memset(&fmtChunk,0,sizeof(fmtChunk));
    fmtChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
    if (mmioDescend(hmmio, &fmtChunk, &riffChunk, MMIO_FINDCHUNK)) {
        throw wave_error(no_format_chunk);
    }

    if ((fmtChunk.cksize==16)||(fmtChunk.cksize==18)) {
        /* Tell Windows to read in the "fmt " chunk into a WAVEFORMATEX structure */
        WAVEFORMATEX format;
        memset(&format,0,sizeof(format));
        if (mmioRead(hmmio, (HPSTR)&format, fmtChunk.cksize) != (LRESULT)fmtChunk.cksize) {
            throw wave_error(format_chunk_read_failure);
        }

        formatTag = format.wFormatTag;
        channels = format.nChannels;
        samplesPerSec = format.nSamplesPerSec;
        blockAlign = format.nBlockAlign;
        bitsPerSample = format.wBitsPerSample;

    } else if (fmtChunk.cksize==40) {

        WAVEFORMATEXTENSIBLE waveInEx;
        memset(&waveInEx,0,sizeof(WAVEFORMATEXTENSIBLE));
        /* Tell Windows to read in the "fmt " chunk into a WAVEFORMATEX structure */
        if (mmioRead(hmmio, (HPSTR)&waveInEx, fmtChunk.cksize) != (LRESULT)fmtChunk.cksize) {
            throw wave_error(format_chunk_read_failure);
        }

        formatTag = waveInEx.Format.wFormatTag;
        channels = waveInEx.Format.nChannels;
        samplesPerSec = waveInEx.Format.nSamplesPerSec;
        blockAlign = waveInEx.Format.nBlockAlign;
        bitsPerSample = waveInEx.Format.wBitsPerSample;

        if (waveInEx.SubFormat==KSDATAFORMAT_SUBTYPE_PCM) {
            formatTag = WAVE_FORMAT_PCM;
        } else if (waveInEx.SubFormat==KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
            formatTag = WAVE_FORMAT_IEEE_FLOAT;
        }
    } else {
        throw wave_error(unsupported_format_chunk_size);
    }

    if (mmioAscend(hmmio, &fmtChunk, 0)) {
        throw wave_error(cant_ascend_from_format_chunk);
    }

    if (hmmio!=NULL) {
        mmioClose(hmmio, 0);
        hmmio = NULL;
    }
}

