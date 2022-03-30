#include "expr.h"

// TODO: Full
int main() {
	using namespace pgbn::expr;

	PGZXB_DEBUG_ASSERT(eval("12 * 12 + 12**2 - 1000").toString() == "-712");

	return 0;
}
