#pragma once

#include <stdexcept>
using std::exception;

enum EWaveError {
	// read errors
	cant_open_file_for_reading=1,
	no_riff_chunk=2,
	no_format_chunk=3,
	format_chunk_read_failure=4,
	unsupported_format_chunk_size=5,
	cant_ascend_from_format_chunk=6,
	no_data_chunk=7,
	data_read_failure=8,
	cant_ascend_from_data_chunk=9,
	// write errors
	cant_open_file_for_writing=20,
	cant_write_riff_chunk=21,
	cant_write_format_chunk=22,
	cant_write_format_tag=23,
	cant_write_num_channels=24,
	cant_write_samples_per_second=25,
	cant_write_bytes_per_second=26,
	cant_write_block_align=27,
	cant_write_bits_per_sample=28,
	cant_create_data_chunk=30,
	cant_write_data_chunk=31,
	cant_ascend_from_riff_chunk=33
};

class wave_error : public exception {
public:
	wave_error( EWaveError val);
	EWaveError reason;
};

