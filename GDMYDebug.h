#pragma once

#include "core/object/ref_counted.h"

class GDMYDebug : public Object {
	GDCLASS(GDMYDebug, Object);

protected:
	static void _bind_methods();

public:
	GDMYDebug();
	~GDMYDebug();

	void log(const String &p_message);

private:
	inline static GDMYDebug *singleton = nullptr;
};
