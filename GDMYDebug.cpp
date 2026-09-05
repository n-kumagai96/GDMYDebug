// pair header
#include "GDMYDebug.h"
// others
#include "core/config/engine.h"
#include "core/core_bind.h"
#include "core/object/callable_mp.h"
#include "core/object/class_db.h"
#include "core/os/main_loop.h"
#include "core/os/os.h"
#include "scene/main/scene_tree.h"
#include "scene/main/canvas_item.h"
#include "scene/main/window.h"
#include "scene/theme/theme_db.h"
#include "scene/gui/control.h"
#include "servers/display/display_server.h"
#include "servers/rendering/rendering_server.h"
#include <optional>

#if GDMYDEBUG_ENABLE_IMPL
#define GDMYDEBUG_IS_GAME_NOW() (GDMYDebug_CPP::is_game_now())
#else
#define GDMYDEBUG_IS_GAME_NOW() (false)
#endif

namespace GDMYDebug_CPP {
struct PerfStats {
	PerfStats() = default;

	String get_one_line_text() const {
		const String fps_value_text = fps.has_value()
				? vformat("%.1f", static_cast<float>(fps.value()))
				: ("-");
		const String ram_now_value_text = ram_now_bytes.has_value()
				? vformat("%.3f", ram_now_bytes.value() / (1024.0 * 1024.0 * 1024.0))
				: ("-");
		const String ram_max_value_text = ram_max_bytes.has_value()
				? vformat("%.3f", ram_max_bytes.value() / (1024.0 * 1024.0 * 1024.0))
				: ("-");
		const String ram_peak_value_text = ram_peak_bytes.has_value()
				? vformat("%.3f", ram_peak_bytes.value() / (1024.0 * 1024.0 * 1024.0))
				: ("-");
		const String vram_now_value_text = vram_now_bytes.has_value()
				? vformat("%.3f", vram_now_bytes.value() / (1024.0 * 1024.0))
				: ("-");
		return vformat("FPS: %s | RAM: %s/ %s GB (PEAK: %s GB) | VRAM: %s MB",
				fps_value_text,
				ram_now_value_text,
				ram_max_value_text,
				ram_peak_value_text,
				vram_now_value_text);
	}

	std::optional<double> fps{ std::nullopt };
	std::optional<uint64_t> ram_now_bytes{ std::nullopt };
	std::optional<uint64_t> ram_max_bytes{ std::nullopt };
	std::optional<uint64_t> ram_peak_bytes{ std::nullopt };
	std::optional<uint64_t> vram_now_bytes{ std::nullopt };
	std::optional<uint64_t> vram_max_bytes{ std::nullopt };
	std::optional<uint64_t> vram_peak_bytes{ std::nullopt };
};

PerfStats create_now_perf_stats() {
	std::optional<double> fps = std::nullopt;
	if (auto *engine = Engine::get_singleton()) {
		fps = std::optional<double>(engine->get_frames_per_second());
	}

	std::optional<uint64_t> ram_now_bytes = std::nullopt;
	std::optional<uint64_t> ram_max_bytes = std::nullopt;
	std::optional<uint64_t> ram_peak_bytes = std::nullopt;
	if (auto *os = OS::get_singleton()) {
		const Dictionary &ram_info = os->get_memory_info();
		if (ram_info.has("physical")) {
			if (const int64_t ram_max = ram_info["physical"];
					ram_max >= 0) {
				ram_max_bytes = std::optional<uint64_t>(ram_max);
			}
		}
		ram_now_bytes = std::optional<uint64_t>(os->get_static_memory_usage());
		ram_peak_bytes = std::optional<uint64_t>(os->get_static_memory_peak_usage());
	}

	std::optional<uint64_t> vram_now_bytes = std::nullopt;
	std::optional<uint64_t> vram_max_bytes = std::nullopt;
	std::optional<uint64_t> vram_peak_bytes = std::nullopt;
	if (auto *rs = RS::get_singleton()) {
		vram_now_bytes = std::optional<uint64_t>(rs->get_rendering_info(RSE::RENDERING_INFO_VIDEO_MEM_USED));
	}

	return PerfStats{
		fps,
		ram_now_bytes,
		ram_max_bytes,
		ram_peak_bytes,
		vram_now_bytes,
		vram_max_bytes,
		vram_peak_bytes,
	};
}

bool is_game_now() {
	if (auto *engine = Engine::get_singleton()) {
		return !engine->is_editor_hint();
	}
	return false;
}

} //namespace GDMYDebug_CPP

GDMYDebug::GDMYDebug() {
	singleton = this;
	if (GDMYDEBUG_IS_GAME_NOW()) {
		if (auto *rs = RenderingServer::get_singleton()) {
			canvas = rs->canvas_create();
			canvas_item = rs->canvas_item_create();
		}
	}
}

GDMYDebug::~GDMYDebug() {
	if (singleton == this) {
		if (GDMYDEBUG_IS_GAME_NOW()) {
			ERR_FAIL_NULL(RenderingServer::get_singleton());
			if (canvas_item.is_valid()) {
				RenderingServer::get_singleton()->free_rid(canvas_item);
			}
			if (canvas.is_valid()) {
				RenderingServer::get_singleton()->free_rid(canvas);
			}
		}
		singleton = nullptr;
	}
}

void GDMYDebug::flush() {
	if (GDMYDEBUG_IS_GAME_NOW()) {
		if (!GDMYDebug_CPP::is_game_now()) {
			return;
		}

		try_attach_scene_tree();

		RenderingServer *rs = RenderingServer::get_singleton();

		rs->canvas_item_clear(canvas_item);

		print_performance();

#if 0
	Vector2 current_position;

	for (const Cmd &command : command_buffer) {
		switch (command.type) {
			case CmdType::SET_CHAR_POSITION: {
				current_position = command.position;
			} break;

			case CmdType::PRINT: {
				// Render command.text at current_position.

				print_line(vformat(
						"DEBUG FONT: (%f, %f) %s",
						current_position.x,
						current_position.y,
						command.text));

				if (auto *theme_db = ThemeDB::get_singleton()) {
					Ref<Theme> default_theme = theme_db->get_default_theme();

					Ref<Font> graph_font = default_theme->get_font("", "");
					if (canvas_item) {
						canvas_item->draw_string(graph_font,
								current_position,
								command.text,
								HORIZONTAL_ALIGNMENT_LEFT,
								-1,
								16);
					}
				}

				current_position.y += 20.0;
			} break;
		}
	}

	command_buffer.clear();
#endif
	}
}

void GDMYDebug::clear()
{
}

void GDMYDebug::set_char_position(const Vector2 &p_position) {
	if (GDMYDEBUG_IS_GAME_NOW()) {
		Cmd command;
		command.type = CmdType::SET_CHAR_POSITION;
		command.position = p_position;
		//command_buffer.push_back(command);
	}
}

void GDMYDebug::print(const String &p_text) {
	if (GDMYDEBUG_IS_GAME_NOW()) {
		Cmd command;
		command.type = CmdType::PRINT;
		command.text = p_text;
		//command_buffer.push_back(command);
	}
}

void GDMYDebug::try_attach_scene_tree() {
	if (GDMYDEBUG_IS_GAME_NOW()) {
		if (!is_attached_scene_tree) {
			// MEMO: main_window seems also fine (Window *main_window = Window::get_from_id(DisplayServerEnums::MAIN_WINDOW_ID))
			if (SceneTree *scene_tree = SceneTree::get_singleton()) {
				if (Viewport *viewport = scene_tree->get_root()) {
					if (RenderingServer *rs = RenderingServer::get_singleton()) {
						rs->viewport_attach_canvas(viewport->get_viewport_rid(), canvas);
						rs->viewport_set_canvas_stacking(viewport->get_viewport_rid(), canvas, RenderingServerEnums::CANVAS_LAYER_MAX, 10);
						rs->canvas_item_set_parent(canvas_item, canvas);

						is_attached_scene_tree = true;
					}
				}
			}
		}
	}
}

Vector2 GDMYDebug::calculate_final_print_screen_pos(
		const Vector2 &in_pos,
		const Ref<Font> &in_font,
		const int font_size) const {
	Vector2 Ret = Vector2(in_pos.x, in_pos.y);
	if (in_font.is_valid()) {
		Ret += Vector2(0, in_font->get_ascent(font_size));
	}
	return Ret;
}

void GDMYDebug::print_performance() {
	if (GDMYDEBUG_IS_GAME_NOW()) {
		if (canvas_item.is_valid()) {
			const GDMYDebug_CPP::PerfStats perf_stats = GDMYDebug_CPP::create_now_perf_stats();
			if (auto *theme_db = ThemeDB::get_singleton()) {
				if (const Ref<Font> font = theme_db->get_fallback_font();
						font.is_valid()) {
					const int font_size = theme_db->get_fallback_font_size();
					font->draw_string(canvas_item,
							calculate_final_print_screen_pos(Vector2(0, 0), font, PERF_STATS_FONT_SIZE),
							perf_stats.get_one_line_text(),
							HORIZONTAL_ALIGNMENT_LEFT,
							-1,
							PERF_STATS_FONT_SIZE,
							PERF_STATS_FONT_COLOR);
					font->draw_string_outline(canvas_item,
							calculate_final_print_screen_pos(Vector2(0, 0), font, PERF_STATS_FONT_SIZE),
							perf_stats.get_one_line_text(),
							HORIZONTAL_ALIGNMENT_LEFT,
							-1,
							PERF_STATS_FONT_SIZE,
							PERF_STATS_OUTLINE_SIZE,
							PERF_STATS_OUTLINE_COLOR);

#if 0
					font->draw_string(canvas_item,
							calculate_final_print_screen_pos(Vector2(0, 0), font, DEBUG_FONT_SIZE),
							perf_stats.get_one_line_text(),
							HORIZONTAL_ALIGNMENT_LEFT,
							-1,
							DEBUG_FONT_SIZE,
							DEBUG_FONT_COLOR);
#endif
				}
			}
		}
	}
}

void GDMYDebug::_bind_methods() {
	ClassDB::bind_method(D_METHOD("print", "message"), &GDMYDebug::print);
	ClassDB::bind_method(D_METHOD("set_char_position", "position"), &GDMYDebug::set_char_position);
}

void GDMYDebug::_notification(int p_what) {
	if (GDMYDEBUG_IS_GAME_NOW()) {
		switch (p_what) {
			case NOTIFICATION_POSTINITIALIZE: {
				// MEMO: the most propor palce to attach, but scene tree is still not created ...
				// so we also try_attach_scene_tree in flush (process_frame)
				try_attach_scene_tree();

				if (auto *rs = RenderingServer::get_singleton()) {
					if (!rs->is_connected("frame_pre_draw", callable_mp(this, &GDMYDebug::flush))) {
						rs->connect("frame_pre_draw", callable_mp(this, &GDMYDebug::flush));
					}
				}
			} break;
			case NOTIFICATION_PREDELETE: {
				if (auto *rs = RenderingServer::get_singleton()) {
					if (rs->is_connected("frame_pre_draw", callable_mp(this, &GDMYDebug::flush))) {
						rs->disconnect("frame_pre_draw", callable_mp(this, &GDMYDebug::flush));
					}
				}
			} break;
		}
	}
}
