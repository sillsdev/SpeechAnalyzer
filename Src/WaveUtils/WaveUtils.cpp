#include <Windows.h>
#include <vector>
#include "FmtParm.h"

using std::vector;

void ExtractChannel(WORD channel, WORD numChannels, WORD blockAlign, vector<char> & in, vector<char> & out) {

    out.clear();
    if (in.size()==0) {
        return;
    }

    DWORD numSamples = in.size()/blockAlign;
    WORD newBlockAlign = blockAlign/numChannels;
    WORD remainder = blockAlign-newBlockAlign;
    // calculate starting index
    DWORD i = channel*newBlockAlign;
    for (int s=0; s<numSamples; s++) {
        for (int a=0; a<newBlockAlign; a++) {
            out.push_back(in[i++]);
        }
        i += remainder;
    }
}

/***************************************************************************
* IsStandardWaveFormat
* Checks basic format of a WAV file and populates CFmtParm.
* Returns false if the file is not a wave file, a non-PCM file, or is not
* in the standard PCM 16/8 bit format that SA uses.
* No errors will be displayed.  Other code will attempt to read the file
* and convert it.
***************************************************************************/
bool IsStandardWaveFormat(LPCTSTR pszPathName, bool allow8Bit) {

    // open file
    HMMIO hmmioFile = mmioOpen(const_cast<TCHAR *>(pszPathName), NULL, MMIO_READ | MMIO_DENYWRITE);
    if (!hmmioFile) {
        return false;
    }

    // locate a 'RIFF' chunk with a 'WAVE' form type to make sure it's a WAVE file.
    MMCKINFO mmckinfoParent;
    mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E'); // prepare search code
    if (mmioDescend(hmmioFile, (LPMMCKINFO)&mmckinfoParent, NULL, MMIO_FINDRIFF)) {
        mmioClose(hmmioFile, 0);
        return false;
    }

    CFmtParm fmtParm;
    // find the format chunk. It should be a subchunk of the 'RIFF' parent chunk
    MMCKINFO mmckinfoSubchunk;
    // prepare search code
    mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
    LONG lError = mmioDescend(hmmioFile, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK);
    if (lError!=MMSYSERR_NOERROR) {
        mmioClose(hmmioFile, 0);
        return false;
    }

    // fmt chunk found
    // read format tag
    lError = mmioRead(hmmioFile, (HPSTR)&fmtParm.wTag, sizeof(WORD));
    if (lError == -1) {
        mmioClose(hmmioFile, 0);
        return false;
    }

	// check if PCM format
    if (fmtParm.wTag != WAVE_FORMAT_PCM) {
        mmioClose(hmmioFile, 0);
        return false;
    }

    // read channel number
    lError = mmioRead(hmmioFile, (HPSTR)&fmtParm.wChannels, sizeof(WORD));
    if (lError == -1) {
        mmioClose(hmmioFile, 0);
        return false;
    }

    // read sampling rate
    lError = mmioRead(hmmioFile, (HPSTR)&fmtParm.dwSamplesPerSec, sizeof(DWORD));
    if (lError == -1) {
        mmioClose(hmmioFile, 0);
        return false;
    }

    // read throughput
    lError = mmioRead(hmmioFile, (HPSTR)&fmtParm.dwAvgBytesPerSec, sizeof(DWORD));
    if (lError == -1) {
        mmioClose(hmmioFile, 0);
        return false;
    }

    // read sampling rate for all channels
    lError = mmioRead(hmmioFile, (HPSTR)&fmtParm.wBlockAlign, sizeof(WORD));
    if (lError == -1) {
        mmioClose(hmmioFile, 0);
        return false;
    }

    // read sample word size
    lError = mmioRead(hmmioFile, (HPSTR)&fmtParm.wBitsPerSample, sizeof(WORD));
    if (lError == -1) {
        mmioClose(hmmioFile, 0);
        return false;
    }

    // if it's not in the expected 8/16 bit format, we will use the conversion routine
	if (allow8Bit) {
		if ((fmtParm.wBitsPerSample != 16) && (fmtParm.wBitsPerSample != 8)) {
			mmioClose(hmmioFile, 0);
			return false;
		}
	} else {
		if (fmtParm.wBitsPerSample != 16) {
			mmioClose(hmmioFile, 0);
			return false;
		}
	}

    // get out of 'fmt ' chunk
    lError = mmioAscend(hmmioFile, &mmckinfoSubchunk, 0);
    if (lError != MMSYSERR_NOERROR) {
        mmioClose(hmmioFile, 0);
        return false;
    }

    // determine how much sound data is in the file. Find the data subchunk
    mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
    lError = mmioDescend(hmmioFile, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK);
    if (lError != MMSYSERR_NOERROR) {
        mmioClose(hmmioFile, 0);
        return false;
    }

    // get the size of the data subchunk
    DWORD dwDataSize = mmckinfoSubchunk.cksize;
    if (dwDataSize == 0L) {
        mmioClose(hmmioFile, 0);
        return false;
    }

    // ascend out of the data chunk
    mmioAscend(hmmioFile, &mmckinfoSubchunk, 0);

    // close the wave file
    mmioClose(hmmioFile, 0);

    return true;
}

