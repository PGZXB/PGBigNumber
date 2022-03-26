#include "../include/PGBigNumber/fwd.h"

using namespace pgbn;

int main() {
	PGZXB_DEBUG_ASSERT(isLittleEndian() == checkLittleEndian());
	PGZXB_DEBUG_Print("PASSED");
	return 0;
}
