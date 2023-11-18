#include "conversion_utils.h"

char variant_to_specifier(const Variant::Type p_type) {
	switch (p_type) {
		case Variant::Type::BOOL:
			return 'b';
		case Variant::Type::INT:
			return 'x';
		case Variant::Type::FLOAT:
			return 'd';
		case Variant::Type::STRING:
			return 's';
		default:
			ERR_FAIL_V_MSG(' ', "Unsupported Variant type.");
	}
}