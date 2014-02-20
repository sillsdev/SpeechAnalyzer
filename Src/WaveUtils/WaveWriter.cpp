#include "WaveWriter.h"
#include "WaveError.h"
#include <stdexcept>

using std::invalid_argument;

CWaveWriter::CWaveWriter() {
	hmmio = NULL;
}

CWaveWriter::~CWaveWriter() {
	if (hmmio!=NULL) {
		mmioClose(hmmio, 0);
		hmmio = NULL;
	}
}

/**
* Writes data chunk header and the data
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
void CWaveWriter::write( LPCTSTR afilename, DWORD dwFlags, WORD bitsPerSample, WORD formatTag, WORD channels, DWORD samplesPerSec, vector<char> & buffer) {

	if (bitsPerSample==0) throw invalid_argument("bitsPerSample");
	if (channels==0) throw invalid_argument("channels");
	if (samplesPerSec==0) throw invalid_argument("samplesPerSec");

	wchar_t filename[128];
	memset(filename,0,_countof(filename));
	wcscpy_s(filename,_countof(filename),afilename);

    //Creating new wav file.
	hmmio = mmioOpen( filename, 0, MMIO_CREATE | MMIO_WRITE | MMIO_EXCLUSIVE);
	if (hmmio==NULL) {
		throw wave_error(cant_open_file_for_writing);
	}

	/**
	* create a 'RIFF' chunk with a 'WAVE' form type
	* NOTE: properly ascending out of this chunk will cause it's size
	* to be automatically written, because, by default the 'dirty' flags
	* is set.
	*/
	MMCKINFO riffChunk;
	riffChunk.fccType = mmioFOURCC('W', 'A', 'V', 'E'); // prepare search code
	//Creating RIFF chunk
	if (mmioCreateChunk(hmmio, &riffChunk, MMIO_CREATERIFF)) {
		throw wave_error(cant_write_riff_chunk);
	}

	WORD blockAlign = (bitsPerSample/8)*channels;
	DWORD nAvgBytesPerSec = samplesPerSec*blockAlign;

	// create the 'fmt ' subchunk
	MMCKINFO fmtChunk;
	memset(&fmtChunk,0,sizeof(fmtChunk));
	fmtChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
	// set chunk size
	fmtChunk.cksize = 16;
	//Creating format chunk and inserting information from source file
	if (mmioCreateChunk(hmmio, &fmtChunk, 0)) {
		throw wave_error(cant_write_format_chunk);
	}

	// write the format parameters into 'fmt ' chunk
	LONG len = mmioWrite(hmmio, (HPSTR)&formatTag, sizeof(WORD));
	if (len!=sizeof(WORD)) {
		throw wave_error(cant_write_format_tag);
	}

	len = mmioWrite(hmmio, (HPSTR)&channels, sizeof(WORD));
	if (len!=sizeof(WORD)) {
		throw wave_error(cant_write_num_channels);
	}

	len = mmioWrite(hmmio, (HPSTR)&samplesPerSec, sizeof(DWORD));
	if (len!=sizeof(DWORD))	{
		throw wave_error(cant_write_samples_per_second);
	}

	len = mmioWrite(hmmio, (HPSTR)&nAvgBytesPerSec, sizeof(DWORD));
	if (len!=sizeof(DWORD))	{
		throw wave_error(cant_write_bytes_per_second);
	}

	len = mmioWrite(hmmio, (HPSTR)&blockAlign, sizeof(WORD));
	if (len!=sizeof(WORD))	{
		throw wave_error(cant_write_block_align);
	}

	len = mmioWrite(hmmio, (HPSTR)&bitsPerSample, sizeof(WORD));
	if (len!=sizeof(WORD)) {
		throw wave_error(cant_write_bits_per_sample);
	}

	if (mmioAscend(hmmio, &fmtChunk, 0))	{
		throw wave_error(cant_ascend_from_format_chunk);
	}

	// descend into data chunk
	MMCKINFO dataChunk;
	memset(&dataChunk,0,sizeof(dataChunk));
	dataChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if (mmioCreateChunk(hmmio, &dataChunk, 0)) {
		throw wave_error(cant_create_data_chunk);
	}

	LONG wrote = mmioWrite( hmmio, &buffer[0], buffer.size());
	if (wrote!=buffer.size()) {
		throw wave_error(cant_write_data_chunk);
	}

	// we don't really care if this succeeds or not...
	if (mmioAscend(hmmio, &dataChunk, 0)) {
		throw wave_error(cant_ascend_from_data_chunk);
	}

	// we don't really care if this succeeds or not...
	if (mmioAscend(hmmio, &riffChunk, 0)) {
		throw wave_error(cant_ascend_from_riff_chunk);
	}

	if (hmmio!=NULL) {
		mmioClose(hmmio, 0);
		hmmio = NULL;
	}
}
