#include "WaveError.h"

wave_error::wave_error( EWaveError val) :
exception("wave error"),
reason(val) {
}
