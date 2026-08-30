// pair header
#include "GDMYDebug.h"
// others
#include "core/object/class_db.h"

GDMYDebug::GDMYDebug() {
	singleton = this;
}

GDMYDebug::~GDMYDebug() {
	if (singleton == this) {
		singleton = nullptr;
	}
}

void GDMYDebug::log(const String &p_message) {
	print_line(p_message);
}

void GDMYDebug::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("log", "message"), &GDMYDebug::log);
}
