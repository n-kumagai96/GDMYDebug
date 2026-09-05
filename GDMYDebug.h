#pragma once

#if defined(DEBUG_ENABLED) || defined(DEV_ENABLED)
#define GDMYDEBUG_ENABLE_IMPL 1
#else
#define GDMYDEBUG_ENABLE_IMPL 0
#endif

#include "scene/main/node.h"
#include "core/object/object.h"

// forward declaration
class Control;
class GDMYDebugPrintScreenOverlay;
class Font;


// this is a custom server
class GDMYDebug : public Object {
	GDCLASS(GDMYDebug, Object);

protected:
	static void _bind_methods();
	void _notification(int p_what);


public:
	enum class CmdType : int32_t {
		NONE = 0,
		SET_CHAR_POSITION,
		PRINT,
	};

	struct Cmd {
		Cmd() :
				type(CmdType::NONE),
				position(Vector2()),
				text() {}

		static Cmd SetCharPosCmd(const Vector2 &in_pos) {}

		CmdType type{ CmdType::NONE };
		Vector2 position{ Vector2() };
		String text{};
	};

	GDMYDebug();
	~GDMYDebug();


	void flush();
	void clear();
	void set_char_position(const Vector2 &p_position);
	void print(const String &p_text);
	void try_attach_scene_tree();

	

private:
	inline static GDMYDebug *singleton = nullptr;
	inline static constexpr int DEBUG_FONT_SIZE = 12;
	inline static constexpr Color DEBUG_FONT_COLOR = Color(221 / 225.f, 34 / 225.f, 136 / 225.f);
	inline static constexpr int PERF_STATS_FONT_SIZE = 24;
	inline static constexpr int PERF_STATS_OUTLINE_SIZE = 2;
	inline static constexpr Color PERF_STATS_FONT_COLOR = Color(225 / 225.f, 225 / 225.f, 225 / 225.f);
	inline static constexpr Color PERF_STATS_OUTLINE_COLOR = Color(0 / 225.f, 0 / 225.f, 0 / 225.f);

	// for HORIZONTAL_ALIGNMENT_LEFT
	Vector2 calculate_final_print_screen_pos(
			const Vector2 &in_pos,
			const Ref<Font> &in_font,
			const int font_size) const;
	void print_performance();


	RID canvas;
	RID canvas_item;
	bool is_attached_scene_tree{ false };
};
