/*
 * This file is part of the AdvanceMAME project.
 *
 * Copyright (C) 1999-2002 Andrea Mazzoleni
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details. 
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "mconfig.h"
#include "text.h"

#include <sstream>

#include <dirent.h>
#include <unistd.h>

using namespace std;

// --------------------------------------------------------------------------
// Configuration init

static struct conf_enum_int OPTION_TRISTATE[] = {
{ "include", include },
{ "exclude", exclude },
{ "exclude_not", exclude_not }
};

static struct conf_enum_int OPTION_SORT[] = {
{ "group", sort_by_group },
{ "name", sort_by_name },
{ "parent", sort_by_root_name },
{ "time", sort_by_time },
{ "coin", sort_by_coin },
{ "year", sort_by_year },
{ "manufacturer", sort_by_manufacturer },
{ "type", sort_by_type },
{ "size", sort_by_size },
{ "resolution", sort_by_res }
};

static struct conf_enum_int OPTION_RESTORE[] = {
{ "save_at_exit", restore_none },
{ "restore_at_exit", restore_exit },
{ "restore_at_idle", restore_idle }
};

static struct conf_enum_int OPTION_MODE[] = {
{ "list", mode_list },
{ "list_mixed", mode_list_mixed },
{ "tile_small", mode_tile_small },
{ "tile_normal", mode_tile_normal },
{ "tile_big", mode_tile_big },
{ "tile_enormous", mode_tile_enormous },
{ "tile_giant", mode_tile_giant },
{ "full", mode_full },
{ "full_mixed", mode_full_mixed },
{ "tile_icon", mode_tile_icon },
{ "tile_marquee", mode_tile_marquee },
{ "text", mode_text }
};

static struct conf_enum_int OPTION_SAVER[] = {
{ "snap", saver_snap },
{ "play", saver_play },
{ "flyers", saver_flyer },
{ "cabinets", saver_cabinet },
{ "titles", saver_title },
{ "none", saver_off }
};

static struct conf_enum_int OPTION_PREVIEW[] = {
{ "snap", preview_snap },
{ "flyers", preview_flyer },
{ "cabinets", preview_cabinet },
{ "titles", preview_title }
};

static struct conf_enum_int OPTION_EVENTMODE[] = {
{ "fast", 1 },
{ "wait", 0 }
};

static struct conf_enum_int OPTION_MERGE[] = {
{ "none", merge_no },
{ "differential", merge_differential },
{ "parent", merge_parent },
{ "any", merge_any },
{ "disable", merge_disable }
};

static struct conf_enum_int OPTION_DEPTH[] = {
{ "8", 8 },
{ "15", 15 },
{ "16", 16 },
{ "32", 32 }
};

static void config_error_la(const string& line, const string& arg) {
	cerr << "Invalid argument '" << arg <<  "' at line '" << line << "'" << endl;
}

static void config_error_oa(const string& opt, const string& arg) {
	cerr << "Invalid argument '" << arg <<  "' at option '" << opt << "'" << endl;
}

static void config_error_a(const string& arg) {
	cerr << "Invalid argument '" << arg <<  "'" << endl;
}

static void config_error_o(const string& opt) {
	cerr << "Invalid option '" << opt <<  "'" << endl;
}

static bool config_import(const string& s, string& a0) {
	if (!arg_split(s, a0)) {
		config_error_a(s);
		return false;
	}

	a0 = path_import(a0);

	return true;
}

static bool config_split(const string& s, string& a0) {
	if (!arg_split(s, a0)) {
		config_error_a(s);
		return false;
	}
	return true;
}

static bool config_split(const string& s, string& a0, string& a1) {
	if (!arg_split(s, a0, a1)) {
		config_error_a(s);
		return false;
	}
	return true;
}

static bool config_split(const string& s, string& a0, string& a1, string& a2) {
	if (!arg_split(s, a0, a1, a2)) {
		config_error_a(s);
		return false;
	}
	return true;
}

static bool config_split(const string& s, string& a0, string& a1, string& a2, string& a3) {
	if (!arg_split(s, a0, a1, a2, a3)) {
		config_error_a(s);
		return false;
	}
	return true;
}

void config_init(struct conf_context* config_context) {
	conf_string_register_multi(config_context, "emulator");
	conf_string_register_multi(config_context, "emulator_roms");
	conf_string_register_multi(config_context, "emulator_roms_filter");
	conf_string_register_multi(config_context, "emulator_altss");
	conf_string_register_multi(config_context, "emulator_flyers");
	conf_string_register_multi(config_context, "emulator_cabinets");
	conf_string_register_multi(config_context, "emulator_icons");
	conf_string_register_multi(config_context, "emulator_marquees");
	conf_string_register_multi(config_context, "emulator_titles");
	conf_string_register_multi(config_context, "emulator_include");
	conf_string_register_multi(config_context, "group");
	conf_string_register_multi(config_context, "type");
	conf_string_register_multi(config_context, "group_include");
	conf_string_register_multi(config_context, "type_include");
	conf_string_register_default(config_context,"type_import","none");
	conf_string_register_default(config_context, "group_import", "none");
	conf_string_register_default(config_context, "desc_import", "none");
	conf_string_register_multi(config_context, "game");
	conf_int_register_enum_default(config_context, "select_neogeo", conf_enum(OPTION_TRISTATE), include);
	conf_int_register_enum_default(config_context, "select_deco", conf_enum(OPTION_TRISTATE), include);
	conf_int_register_enum_default(config_context, "select_playchoice", conf_enum(OPTION_TRISTATE), include);
	conf_int_register_enum_default(config_context, "select_clone", conf_enum(OPTION_TRISTATE), exclude);
	conf_int_register_enum_default(config_context, "select_bad", conf_enum(OPTION_TRISTATE), exclude);
	conf_int_register_enum_default(config_context, "select_missing", conf_enum(OPTION_TRISTATE), include);
	conf_int_register_enum_default(config_context, "select_vector", conf_enum(OPTION_TRISTATE), include);
	conf_int_register_enum_default(config_context, "select_vertical", conf_enum(OPTION_TRISTATE), include);
	conf_int_register_enum_default(config_context, "sort", conf_enum(OPTION_SORT), sort_by_root_name);
	conf_bool_register_default(config_context, "lock", 0);
	conf_int_register_enum_default(config_context, "config", conf_enum(OPTION_RESTORE), restore_none);
	conf_int_register_enum_default(config_context, "mode", conf_enum(OPTION_MODE), mode_list);
	conf_string_register_default(config_context, "mode_skip", "");
	conf_int_register_limit_default(config_context, "event_exit_press", 0, 3, 1);
	conf_string_register_multi(config_context, "event_assign");
	conf_string_register_multi(config_context, "color");
	conf_string_register_default(config_context, "idle_start", "0 0");
	conf_string_register_default(config_context, "idle_screensaver", "60 10");
	conf_int_register_enum_default(config_context, "idle_screensaver_preview", conf_enum(OPTION_SAVER), saver_snap);
	conf_int_register_default(config_context, "menu_base", 0);
	conf_int_register_default(config_context, "menu_rel", 0);
	conf_string_register_default(config_context, "event_repeat", "500 50");
	conf_string_register_default(config_context, "msg_run", "\"Run game\"");
	conf_int_register_enum_default(config_context, "preview", conf_enum(OPTION_PREVIEW), preview_snap);
	conf_float_register_limit_default(config_context, "preview_expand", 1.0, 3.0, 1.15);
	conf_string_register_default(config_context, "preview_default", "none");
	conf_string_register_default(config_context, "preview_default_snap", "none");
	conf_string_register_default(config_context, "preview_default_flyer", "none");
	conf_string_register_default(config_context, "preview_default_cabinet", "none");
	conf_string_register_default(config_context, "preview_default_icon", "none");
	conf_string_register_default(config_context, "preview_default_marquee", "none");
	conf_string_register_default(config_context, "preview_default_title", "none");
	conf_int_register_enum_default(config_context, "event_mode", conf_enum(OPTION_EVENTMODE), 1);
	conf_bool_register_default(config_context, "event_alpha", 1);
	conf_int_register_enum_default(config_context, "merge", conf_enum(OPTION_MERGE), merge_differential);
	conf_int_register_limit_default(config_context, "icon_space", 10, 500, 43);
	conf_string_register_default(config_context, "sound_foreground_begin", "default");
	conf_string_register_default(config_context, "sound_foreground_end", "default");
	conf_string_register_default(config_context, "sound_foreground_key", "default");
	conf_string_register_default(config_context, "sound_foreground_start", "default");
	conf_string_register_default(config_context, "sound_foreground_stop", "default");
	conf_string_register_default(config_context, "sound_background_loop", "default");
	conf_string_register_default(config_context, "sound_background_begin", "none");
	conf_string_register_default(config_context, "sound_background_end", "none");
	conf_string_register_default(config_context, "sound_background_start", "none");
	conf_string_register_default(config_context, "sound_background_stop", "none");
	conf_string_register_default(config_context, "sound_background_loop_dir", "\"mp3\"");
	conf_int_register_limit_default(config_context, "video_size",160,2048,1024);
	conf_int_register_enum_default(config_context, "video_depth", conf_enum(OPTION_DEPTH), 16);
	conf_string_register_default(config_context, "video_font", "none");
	conf_string_register_default(config_context, "video_orientation", "");
	conf_float_register_limit_default(config_context,"video_gamma",0.2,5,1);
	conf_float_register_limit_default(config_context,"video_brightness",0.2,5,1);
	conf_bool_register_default(config_context,"video_restore",1);
	conf_bool_register_default(config_context,"misc_quiet",0);
}

// -------------------------------------------------------------------------
// Configuration load

static bool config_load_background_dir(const string& dir, path_container& c) {
	if (dir=="none")
		return false;

	bool almost_one = false;
	DIR* d = opendir(cpath_export(slash_remove(dir)));
	if (!d)
		return almost_one;

	struct dirent* dd;
	while ((dd = readdir(d))!=0) {
		string file = os_import(dd->d_name);
		if (file_ext(file) == ".mp3" || file_ext(file) == ".wav") {
			c.insert( c.end(), slash_add(dir) + file);
		}
	}

	closedir(d);
	return almost_one;
}

static bool config_load_background_list(const string& list, path_container& c) {
	bool almost_one = false;
	int i = 0;
	while (i<list.length()) {
		string dir = token_get(list,i,";");
		token_skip(list,i,";");
		almost_one = almost_one || config_load_background_dir(dir, c);
	}
	return almost_one;
}

static bool config_load_game(config_state& rs, const string& name, const string& group, const string& type, const string& time, const string& coin, const string& desc) {
	game_set::const_iterator i = rs.gar.find( game( name ) );
	if (i==rs.gar.end())
		return false;

	if (group != CATEGORY_UNDEFINED)
		i->user_group_set(group);

	if (type != CATEGORY_UNDEFINED)
		i->user_type_set(type);

	if (desc.length()!=0)
		i->user_description_set(desc);

	if (time.length()!=0 && isdigit(time[0]))
		i->time_set(atoi(time.c_str()));

	if (coin.length()!=0 && isdigit(coin[0]))
		i->coin_set(atoi(coin.c_str()));

	rs.group.insert_double(group,rs.include_group_orig);
	rs.type.insert_double(type,rs.include_type_orig);

	return true;
}

static bool config_emulator_load(const string& name, pemulator_container& emu, void (emulator::*set)(const string& s), const string& value) {
	pemulator_container::iterator i = emu.begin();
	while (i!=emu.end() && name!=(*i)->user_name_get())
		++i;
	if (i!=emu.end()) {
		((*i)->*set)(value);
		return true;
	} else {
		return false;
	}
}

static bool config_load_iterator(struct conf_context* config_context, const string& tag, bool (*func)(const string& s)) {
	conf_iterator i;
	conf_iterator_begin(&i, config_context, tag.c_str());
	while (!conf_iterator_is_end(&i)) {
		string a0 = conf_iterator_string_get(&i);
		if (!func(a0)) {
			config_error_a(a0);
			return false;
		}
		conf_iterator_next(&i);
	}
	return true;
}

static bool config_load_iterator_emu_set(struct conf_context* config_context, const string& tag, pemulator_container& emu, void (emulator::*set)(const string& s)) {
	conf_iterator i;
	conf_iterator_begin(&i, config_context, tag.c_str());
	while (!conf_iterator_is_end(&i)) {
		string s,a0,a1;
		s = conf_iterator_string_get(&i);
		if (!config_split(s,a0,a1))
			return false;
		if (!config_emulator_load(a0,emu,set,a1)) {
			config_error_a(s);
			return false;
		}
		conf_iterator_next(&i);
	}
	return true;
}

static bool config_load_skip(struct conf_context* config_context, unsigned& mask) {
	string s;
	int i;

	s = conf_string_get_default(config_context, "mode_skip");
	i = 0;
	mask = 0;
	while (i<s.length()) {
		unsigned j;
		string a0;
		a0 = arg_get(s,i);
		for(j=0;j<conf_size(OPTION_MODE);++j)
			if (a0 == OPTION_MODE[j].value)
				break;
		if (j == conf_size(OPTION_MODE)) {
			config_error_la("mode_skip " + s,a0);
			return false;
		}
		mask |= OPTION_MODE[j].map;
	}

	return true;
}

static bool config_load_iterator_emu(struct conf_context* config_context, const string& tag, pemulator_container& emu) {
	conf_iterator i;
	conf_iterator_begin(&i, config_context, tag.c_str());
	while (!conf_iterator_is_end(&i)) {
		string a0,a1,a2,a3;
		string s = conf_iterator_string_get(&i);
		if (!config_split(s,a0,a1,a2,a3))
			return false;
		if (a0.length()==0 || a1.length()==0) {
			config_error_a(s);
			return false;
		}
		emulator* e;
		if (a1 == "mame") {
			e = new mame(a0,a2,a3);
		} else if (a1 == "mess") {
			e = new mess(a0,a2,a3);
		} else if (a1 == "raine") {
			e = new raine(a0,a2,a3);
		} else if (a1 == "generic") {
			e = new generic(a0,a2,a3);
		} else if (a1 == "advmame") {
			e = new advmame(a0,a2,a3);
		} else if (a1 == "advmess") {
			e = new advmess(a0,a2,a3);
		} else if (a1 == "advpac") {
			e = new advpac(a0,a2,a3);
		} else {
			config_error_la(tag + " " + s,a1);
			return false;
		}
		emu.insert(emu.end(),e);

		conf_iterator_next(&i);
	}
	return true;
}

static bool config_load_iterator_category(struct conf_context* config_context, const string& tag, category_container& cat) {
	conf_iterator i;
	conf_iterator_begin(&i, config_context, tag.c_str());
	while (!conf_iterator_is_end(&i)) {
		string a0;
		string s = conf_iterator_string_get(&i);
		if (!config_split(s,a0))
			return false;
		cat.insert(cat.end(),a0);
		conf_iterator_next(&i);
	}
	return true;
}

static bool config_load_iterator_emu_include(struct conf_context* config_context, const string& tag, emulator_container& emu) {
	conf_iterator i;
	conf_iterator_begin(&i, config_context, tag.c_str());
	while (!conf_iterator_is_end(&i)) {
		string a0;
		string s = conf_iterator_string_get(&i);
		if (!config_split(s,a0))
			return false;
		emu.insert(emu.end(),a0);
		conf_iterator_next(&i);
	}
	return true;
}

static bool config_load_iterator_game(struct conf_context* config_context, const string& tag, config_state& rs) {
	conf_iterator i;
	conf_iterator_begin(&i, config_context, tag.c_str());
	while (!conf_iterator_is_end(&i)) {
		string s = conf_iterator_string_get(&i);
		int j = 0;
		string game = arg_get(s,j);
		string group = arg_get(s,j);
		string type = arg_get(s,j);
		string time = arg_get(s,j);
		string coin = arg_get(s,j);
		string desc = arg_get(s,j);

		if (j != s.length()) {
			config_error_a(s);
			return false;
		}

		if (game.length()==0) {
			config_error_a(s);
			return false;
		}

		if (group.length()==0)
			group = CATEGORY_UNDEFINED;

		if (type.length()==0)
			type = CATEGORY_UNDEFINED;

		if (!config_load_game(rs,game,group,type,time,coin,desc)) {
			cerr << "warning: ignoring info for game " << game << endl;
		}

		conf_iterator_next(&i);
	}

	return true;
}

static bool config_load_orientation(struct conf_context* config_context, unsigned& mask) {
	string s;
	int i;

	s = conf_string_get_default(config_context, "video_orientation");
	i = 0;
	mask = 0;
	while (i<s.length()) {
		string arg = arg_get(s,i);
		if (arg == "flip_xy")
			mask ^= TEXT_ORIENTATION_SWAP_XY;
		else if (arg == "mirror_x")
			mask ^= TEXT_ORIENTATION_FLIP_X;
		else if (arg == "mirror_y")
			mask ^= TEXT_ORIENTATION_FLIP_Y;
		else {
			config_error_la("video_orientation " + s,arg);
			return false;
		}
	}

	return true;
}

bool config_is_emulator(const pemulator_container& ec, const string& emulator) {
	for(pemulator_container::const_iterator i=ec.begin();i!=ec.end();++i) {
		if ((*i)->user_name_get() == emulator)
			return true;
	}
	return false;
}

bool config_load(config_state& rs, struct conf_context* config_context, bool opt_verbose) {
	string a0,a1;

	rs.preview_mask = 0;
	rs.current_game = 0;
	rs.current_clone = 0;
	rs.fast = "";
	rs.current_backdrop = resource();
	rs.current_sound = resource();

	if (!config_split(conf_string_get_default(config_context, "desc_import"),rs.desc_import_type,rs.desc_import_sub,rs.desc_import_file))
		return false;
	rs.desc_import_file = path_import(rs.desc_import_file);
	if (!config_split(conf_string_get_default(config_context, "type_import"),rs.type_import_type,rs.type_import_sub,rs.type_import_file,rs.type_import_section))
		return false;
	rs.type_import_file = path_import(rs.type_import_file);
	if (!config_split(conf_string_get_default(config_context, "group_import"),rs.group_import_type,rs.group_import_sub,rs.group_import_file,rs.group_import_section))
		return false;
	rs.group_import_file = path_import(rs.group_import_file);
	rs.exclude_neogeo_orig = (tristate_t)conf_int_get_default(config_context, "select_neogeo");
	rs.exclude_deco_orig = (tristate_t)conf_int_get_default(config_context, "select_deco");
	rs.exclude_playchoice_orig = (tristate_t)conf_int_get_default(config_context, "select_playchoice");
	rs.exclude_clone_orig = (tristate_t)conf_int_get_default(config_context, "select_clone");
	rs.exclude_bad_orig = (tristate_t)conf_int_get_default(config_context, "select_bad");
	rs.exclude_vertical_orig = (tristate_t)conf_int_get_default(config_context, "select_vertical");
	rs.exclude_missing_orig = (tristate_t)conf_int_get_default(config_context, "select_missing");
	rs.exclude_vector_orig = (tristate_t)conf_int_get_default(config_context, "select_vector");
	rs.sort_orig = (game_sort_t)conf_int_get_default(config_context, "sort");
	rs.lock_orig = (bool)conf_bool_get_default(config_context, "lock");
	rs.restore = (restore_t)conf_int_get_default(config_context, "config");
	rs.mode_orig = (show_t)conf_int_get_default(config_context, "mode");
	if (!config_load_skip(config_context,rs.mode_skip_mask))
		return false;
	rs.exit_count = conf_int_get_default(config_context, "event_exit_press");
	if (!config_load_iterator(config_context, "event_assign", text_key_in))
		return false;
	if (!config_load_iterator(config_context, "color", text_color_in))
		return false;
	if (!config_split(conf_string_get_default(config_context, "idle_start"), a0, a1))
		return false;
	rs.idle_start_first = atoi( a0.c_str() );
	rs.idle_start_rep = atoi( a1.c_str() );
	rs.menu_base_orig = conf_int_get_default(config_context, "menu_base");
	rs.menu_rel_orig = conf_int_get_default(config_context, "menu_rel");
	if (!config_split(conf_string_get_default(config_context, "idle_screensaver"), a0, a1))
		return false;
	rs.idle_saver_first = atoi( a0.c_str() );
	rs.idle_saver_rep = atoi( a1.c_str() );
	if (!config_split(conf_string_get_default(config_context, "event_repeat"), a0, a1))
		return false;
	rs.repeat = atoi( a0.c_str() );
	rs.repeat_rep = atoi( a1.c_str() );
	rs.video_size = conf_int_get_default(config_context, "video_size");
	rs.video_depth = conf_int_get_default(config_context, "video_depth");
	if (!config_import(conf_string_get_default(config_context, "video_font"), a0))
		return false;
	rs.video_font_path = a0;
	if (!config_load_orientation(config_context,rs.video_orientation_orig))
		return false;
	rs.video_gamma = conf_float_get_default(config_context, "video_gamma");
	rs.video_brightness = conf_float_get_default(config_context, "video_brightness");
	rs.video_reset_mode = conf_bool_get_default(config_context,"video_restore");
	rs.quiet = conf_bool_get_default(config_context,"misc_quiet");
	if (!config_split(conf_string_get_default(config_context, "msg_run"),rs.msg_run_game))
		return false;
	rs.preview_orig = (preview_t)conf_int_get_default(config_context, "preview");
	rs.idle_saver_type = (saver_t)conf_int_get_default(config_context, "idle_screensaver_preview");
	rs.preview_expand = conf_float_get_default(config_context, "preview_expand");
	if (!config_import(conf_string_get_default(config_context, "preview_default"),rs.preview_default))
		return false;
	if (!config_import(conf_string_get_default(config_context, "preview_default_snap"),rs.preview_default_snap))
		return false;
	if (!config_import(conf_string_get_default(config_context, "preview_default_flyer"),rs.preview_default_flyer))
		return false;
	if (!config_import(conf_string_get_default(config_context, "preview_default_cabinet"),rs.preview_default_cabinet))
		return false;
	if (!config_import(conf_string_get_default(config_context, "preview_default_icon"),rs.preview_default_icon))
		return false;
	if (!config_import(conf_string_get_default(config_context, "preview_default_marquee"),rs.preview_default_marquee))
		return false;
	if (!config_import(conf_string_get_default(config_context, "preview_default_title"),rs.preview_default_title))
		return false;
	rs.preview_fast = (bool)conf_int_get_default(config_context, "event_mode");
	rs.alpha_mode = (bool)conf_bool_get_default(config_context, "event_alpha");
	rs.merge = (merge_t)conf_int_get_default(config_context, "merge");
	rs.icon_space = conf_int_get_default(config_context, "icon_space");

	if (!config_import(conf_string_get_default(config_context, "sound_foreground_begin"),rs.sound_foreground_begin))
		return false;
	if (!config_import(conf_string_get_default(config_context, "sound_foreground_end"),rs.sound_foreground_end))
		return false;
	if (!config_import(conf_string_get_default(config_context, "sound_foreground_key"),rs.sound_foreground_key))
		return false;
	if (!config_import(conf_string_get_default(config_context, "sound_foreground_start"),rs.sound_foreground_start))
		return false;
	if (!config_import(conf_string_get_default(config_context, "sound_foreground_stop"),rs.sound_foreground_stop))
		return false;
	if (!config_import(conf_string_get_default(config_context, "sound_background_begin"),rs.sound_background_begin))
		return false;
	if (!config_import(conf_string_get_default(config_context, "sound_background_end"),rs.sound_background_end))
		return false;
	if (!config_import(conf_string_get_default(config_context, "sound_background_start"),rs.sound_background_start))
		return false;
	if (!config_import(conf_string_get_default(config_context, "sound_background_stop"),rs.sound_background_stop))
		return false;
	if (!config_import(conf_string_get_default(config_context, "sound_background_loop"),rs.sound_background_loop))
		return false;
	if (!config_import(conf_string_get_default(config_context, "sound_background_loop_dir"),rs.sound_background_loop_dir))
		return false;
	if (!config_load_iterator_emu(config_context, "emulator", rs.emu))
		return false;
	if (!config_load_iterator_emu_set(config_context, "emulator_roms", rs.emu,&emulator::user_rom_path_set))
		return false;
	if (!config_load_iterator_emu_set(config_context, "emulator_roms_filter", rs.emu, &emulator::user_rom_filter_set))
		return false;
	if (!config_load_iterator_emu_set(config_context, "emulator_altss", rs.emu, &emulator::user_alts_path_set))
		return false;
	if (!config_load_iterator_emu_set(config_context, "emulator_flyers", rs.emu, &emulator::user_flyer_path_set))
		return false;
	if (!config_load_iterator_emu_set(config_context, "emulator_cabinets", rs.emu, &emulator::user_cabinet_path_set))
		return false;
	if (!config_load_iterator_emu_set(config_context, "emulator_icons", rs.emu, &emulator::user_icon_path_set))
		return false;
	if (!config_load_iterator_emu_set(config_context, "emulator_marquees", rs.emu, &emulator::user_marquee_path_set))
		return false;
	if (!config_load_iterator_emu_set(config_context, "emulator_titles", rs.emu, &emulator::user_title_path_set))
		return false;

	// print the copyright message before other messages
	if (!rs.quiet) {
		cerr << "AdvanceMENU - Copyright (C) 1999-2002 by Andrea Mazzoleni" << endl;
#ifdef __MSDOS__
		cerr << _go32_dpmi_remaining_physical_memory()/(1024*1024) << " [Mb] free physical memory, " << _go32_dpmi_remaining_virtual_memory()/(1024*1024) << " [Mb] free virtual memory" << endl;
#endif
	}

	// select the active emulators
	for(pemulator_container::iterator i=rs.emu.begin();i!=rs.emu.end();++i) {
		if ((*i)->is_ready())
			rs.emu_active.insert(rs.emu_active.end(),*i);
		else
			cerr << "warning: emulator " << (*i)->user_exe_path_get() << " not found" << endl;
	}

	if (rs.emu_active.size() == 0) {
		cerr << "No emulator found. Add an `emulator' option in your configuration file." << endl;
		return false;
	}

	// load the game definitions
	for(pemulator_container::iterator i=rs.emu_active.begin();i!=rs.emu_active.end();++i) {
		if (opt_verbose)
			cerr << "log: load game for " << (*i)->user_name_get() << endl;
		if (!(*i)->load_game(rs.gar)) {
			return false;
		}
	}

	// load the emulator configurations
	for(pemulator_container::iterator i=rs.emu_active.begin();i!=rs.emu_active.end();++i) {
		if (opt_verbose)
			cerr << "log: load cfg for " << (*i)->user_name_get() << endl;
		if (!(*i)->load_cfg(rs.gar)) {
			return false;
		}
	}

	// load the software definitions
	for(pemulator_container::iterator i=rs.emu_active.begin();i!=rs.emu_active.end();++i) {
		if (opt_verbose)
			cerr << "log: load software for " << (*i)->user_name_get() << endl;
		if (!(*i)->load_software(rs.gar)) {
			exit(EXIT_FAILURE);
		}
	}

	if (opt_verbose)
		cerr << "log: adjust list" << endl;

	// compile the relations
	rs.gar.sync_relationships();

	// set the previews
	for(pemulator_container::iterator i=rs.emu_active.begin();i!=rs.emu_active.end();++i) {
		if (opt_verbose)
			cerr << "log: load preview for " << (*i)->user_name_get() << endl;
		(*i)->preview_set(rs.gar);
	}

	if (opt_verbose)
		cerr << "log: load group and types" << endl;

	// load the group/type informations
	if (!config_load_iterator_category(config_context,"group",rs.group))
		return false;
	if (!config_load_iterator_category(config_context,"type",rs.type))
		return false;
	if (!config_load_iterator_category(config_context,"group_include",rs.include_group_orig))
		return false;
	if (!config_load_iterator_category(config_context,"type_include",rs.include_type_orig))
		return false;

	rs.group.insert_double(CATEGORY_UNDEFINED,rs.include_group_orig);
	rs.type.insert_double(CATEGORY_UNDEFINED,rs.include_type_orig);
	if (rs.include_group_orig.size() == 0)
		rs.include_group_orig = rs.group;
	if (rs.include_type_orig.size() == 0)
		rs.include_type_orig = rs.type;

	if (opt_verbose)
		cerr << "log: load games info" << endl;

	if (!config_load_iterator_game(config_context,"game",rs))
		return false;

	// load the emulator active
	if (!config_load_iterator_emu_include(config_context,"emulator_include",rs.include_emu_orig))
		return false;

	if (rs.include_emu_orig.size() == 0) {
		for(pemulator_container::iterator i=rs.emu_active.begin();i!=rs.emu_active.end();++i) {
			rs.include_emu_orig.insert(rs.include_emu_orig.end(),(*i)->user_name_get());
		}
	}

	// load the emulator data
	for(pemulator_container::iterator i=rs.emu_active.begin();i!=rs.emu_active.end();++i) {
		if (opt_verbose)
			cerr << "log: load data for " << (*i)->user_name_get() << endl;
		if (!(*i)->load_data(rs.gar)) {
			exit(EXIT_FAILURE);
		}
	}

	if (rs.desc_import_type != "none") {
		if (!config_is_emulator(rs.emu,rs.desc_import_sub)) {
			config_error_oa("desc_import",rs.desc_import_sub);
			return false;
		}
	}
	if (rs.desc_import_type == "nms") {
		if (opt_verbose)
			cerr << "log: importing from " << rs.desc_import_file << endl;
		rs.gar.import_nms(rs.desc_import_file,rs.desc_import_sub,&game::auto_description_set);
	} else if (rs.desc_import_type != "none") {
		config_error_oa("desc_import", rs.desc_import_type);
		return false;
	}

	if (rs.type_import_type != "none") {
		if (!config_is_emulator(rs.emu,rs.type_import_sub)) {
			config_error_oa("type_import",rs.type_import_sub);
			return false;
		}
	}
	if (rs.type_import_type == "ini") {
		if (opt_verbose)
			cerr << "log: importing from " << rs.type_import_file << endl;
		rs.type.import_ini(rs.gar,rs.type_import_file,rs.type_import_section,rs.type_import_sub,&game::auto_type_set,rs.include_type_orig);
	} else if (rs.type_import_type == "mac") {
		if (opt_verbose)
			cerr << "log: importing from " << rs.type_import_file << endl;
		rs.type.import_mac(rs.gar,rs.type_import_file,rs.type_import_section,rs.type_import_sub,&game::auto_type_set,rs.include_type_orig);
	} else if (rs.type_import_type != "none") {
		config_error_oa("type_import", rs.type_import_type);
		return false;
	}

	if (rs.group_import_type != "none") {
		if (!config_is_emulator(rs.emu,rs.group_import_sub)) {
			config_error_oa("group_import",rs.group_import_sub);
			return false;
		}
	}
	if (rs.group_import_type == "ini") {
		if (opt_verbose)
			cerr << "log: importing from " << rs.group_import_file << endl;
		rs.group.import_ini(rs.gar,rs.group_import_file,rs.group_import_section,rs.group_import_sub,&game::auto_group_set,rs.include_group_orig);
	} else if (rs.group_import_type == "mac") {
		if (opt_verbose)
			cerr << "log: importing from " << rs.group_import_file << endl;
		rs.group.import_mac(rs.gar,rs.group_import_file,rs.group_import_section,rs.group_import_sub,&game::auto_group_set,rs.include_group_orig);
	} else if (rs.group_import_type != "none") {
		config_error_oa("group_import", rs.group_import_type);
		return false;
	}

	if (opt_verbose)
		cerr << "log: load background music list" << endl;

	config_load_background_list(rs.sound_background_loop_dir,rs.sound_background);

	if (opt_verbose)
		cerr << "log: start" << endl;

	return true;
}

void config_default(struct conf_context* config_context) {
	conf_iterator i;

	conf_iterator_begin(&i, config_context, "emulator");
	if (conf_iterator_is_end(&i)) {
#ifdef __MSDOS__
		if (file_exists("mamedos.exe"))
			conf_set(config_context,"","emulator", "\"mamedos\" mame \"mamedos.exe\" \"\"");
		if (file_exists("advmame.exe"))
			conf_set(config_context,"","emulator", "\"advmame\" advmame \"advmame.exe\" \"\"");
		if (file_exists("advmess.exe"))
			conf_set(config_context,"","emulator", "\"advmess\" advmess \"advmess.exe\" \"\"");
		if (file_exists("advpac.exe"))
			conf_set(config_context,"","emulator", "\"advpac\" advpac \"advpac.exe\" \"\"");
		if (file_exists("raine.exe"))
			conf_set(config_context,"","emulator", "\"raine\" raine \"raine.exe\" \"\"");
		if (file_exists("snes9x.exe")) {
			conf_set(config_context,"","emulator", "\"snes9x\" generic \"snes9x.exe\" \"%f\"");
			conf_set(config_context,"","emulator_roms", "\"snes9x\" \"roms\"");
		}
		if (file_exists("zsnes.exe")) {
			conf_set(config_context,"","emulator", "\"zsnes\" generic \"zsnes.exe\" \"-e -m roms\\%f\"");
			conf_set(config_context,"","emulator_roms", "\"zsnes\" \"roms\"");
		}
#else
		conf_set(config_context,"","emulator","\"advmame\" advmame \"advmame\" \"\"");
		conf_set(config_context,"","emulator","\"advmess\" advmess \"advmess\" \"\"");
		conf_set(config_context,"","emulator","\"advpac\" advpac \"advpac\" \"\"");
#endif
	}

	conf_iterator_begin(&i, config_context, "group");
	if (conf_iterator_is_end(&i)) {
		conf_set(config_context,"","group","\"Very Good\"");
		conf_set(config_context,"","group","\"Good\"");
		conf_set(config_context,"","group","\"Bad\"");
		conf_set(config_context,"","group","\"<undefined>\"");
	}

	conf_iterator_begin(&i, config_context, "type");
	if (conf_iterator_is_end(&i)) {
		conf_set(config_context,"","type","\"Computer\"");
		conf_set(config_context,"","type","\"Consolle\"");
		conf_set(config_context,"","type","\"Application\"");
		conf_set(config_context,"","type","\"Arcade\"");
		conf_set(config_context,"","type","\"Shot'em Up\"");
		conf_set(config_context,"","type","\"Bet'em Up\"");
		conf_set(config_context,"","type","\"Fight\"");
		conf_set(config_context,"","type","\"Gun\"");
		conf_set(config_context,"","type","\"Puzzle\"");
		conf_set(config_context,"","type","\"RPG\"");
		conf_set(config_context,"","type","\"Sport\"");
		conf_set(config_context,"","type","\"Breakout\"");
		conf_set(config_context,"","type","\"Filler\"");
		conf_set(config_context,"","type","\"Racing\"");
		conf_set(config_context,"","type","\"Flipper\"");
		conf_set(config_context,"","type","\"<undefined>\"");
	}

	conf_iterator_begin(&i, config_context, "color");
	if (conf_iterator_is_end(&i)) {
		text_color_out(config_context, "color");
	}

	conf_iterator_begin(&i, config_context, "event_assign");
	if (conf_iterator_is_end(&i)) {
		text_key_out(config_context, "event_assign");
	}
}

// -------------------------------------------------------------------------
// Configuration save

static string config_out(const string& a0) {
	return "\"" + a0 + "\"";
}

void config_save(const config_state& rs, struct conf_context* config_context) {
	conf_int_set(config_context,"","mode",rs.mode_orig);
	conf_int_set(config_context,"","menu_base",rs.menu_base_orig);
	conf_int_set(config_context,"","menu_rel",rs.menu_rel_orig);
	conf_int_set(config_context,"","sort",rs.sort_orig);
	conf_int_set(config_context,"","preview",rs.preview_orig);

	conf_remove(config_context,"","emulator_include");
	for(emulator_container::const_iterator i=rs.include_emu_orig.begin();i!=rs.include_emu_orig.end();++i) {
		conf_string_set(config_context,"","emulator_include",config_out(*i).c_str());
	}

	conf_remove(config_context,"","group_include");
	for(category_container::const_iterator i=rs.include_group_orig.begin();i!=rs.include_group_orig.end();++i) {
		conf_string_set(config_context,"","group_include",config_out(*i).c_str());
	}

	conf_remove(config_context,"","type_include");
	for(category_container::const_iterator i=rs.include_type_orig.begin();i!=rs.include_type_orig.end();++i) {
		conf_string_set(config_context,"","type_include",config_out(*i).c_str());
	}

	conf_int_set(config_context,"","select_neogeo",rs.exclude_neogeo_orig);
	conf_int_set(config_context,"","select_deco",rs.exclude_deco_orig);
	conf_int_set(config_context,"","select_playchoice",rs.exclude_playchoice_orig);
	conf_int_set(config_context,"","select_clone",rs.exclude_clone_orig);
	conf_int_set(config_context,"","select_bad",rs.exclude_bad_orig);
	conf_int_set(config_context,"","select_missing",rs.exclude_missing_orig);
	conf_int_set(config_context,"","select_vector",rs.exclude_vector_orig);
	conf_int_set(config_context,"","select_vertical",rs.exclude_vertical_orig);

	string s;
	if ((rs.video_orientation_orig & TEXT_ORIENTATION_SWAP_XY) != 0) {
		if (s.length()) s += " ";
		s += "flip_xy";
	}
	if ((rs.video_orientation_orig & TEXT_ORIENTATION_FLIP_X) != 0) {
		if (s.length()) s += " ";
		s += "mirror_x";
	}
	if ((rs.video_orientation_orig & TEXT_ORIENTATION_FLIP_Y) != 0) {
		if (s.length()) s += " ";
		s += "mirror_y";
	}
	conf_string_set(config_context, "", "video_orientation", s.c_str());

	conf_remove(config_context,"","game");
	for(game_set::const_iterator i=rs.gar.begin();i!=rs.gar.end();++i) {
		if (0
			|| (i->is_user_group_set() && i->group_get().length()!=0)
			|| (i->is_user_type_set() && i->type_get().length()!=0)
			|| (i->is_time_set() && i->time_get()!=0)
			|| (i->is_coin_set() && i->coin_get()!=0)
			|| (i->is_user_description_set() && i->description_get().length()!=0)
		) {
			ostringstream f;
			f << "\"" << i->name_get() << "\"";

			f << " \"";
			if (i->is_user_group_set())
				f << i->group_get();
			f << "\"";

			f << " \"";
			if (i->is_user_type_set())
				f << i->type_get();
			f << "\"";

			f << " " << i->time_get();

			f << " " << i->coin_get();

			f << " \"";
			if (i->is_user_description_set()) {
				 f << i->description_get();
			}
			f << "\"";

			conf_string_set(config_context,"","game",f.str().c_str());
		}
	}

	conf_save(config_context,1);

	// prevent data lost if crashing
	sync();
}

// ------------------------------------------------------------------------
// Configuration restore

void config_restore_load(config_state& rs) {
	rs.mode_effective = rs.mode_orig;
	rs.preview_effective = rs.preview_orig;
	rs.sort_effective = rs.sort_orig;
	rs.exclude_neogeo_effective = rs.exclude_neogeo_orig;
	rs.exclude_deco_effective = rs.exclude_deco_orig;
	rs.exclude_playchoice_effective = rs.exclude_playchoice_orig;
	rs.exclude_clone_effective = rs.exclude_clone_orig;
	rs.exclude_bad_effective = rs.exclude_bad_orig;
	rs.exclude_missing_effective = rs.exclude_missing_orig;
	rs.exclude_vector_effective = rs.exclude_vector_orig;
	rs.exclude_vertical_effective = rs.exclude_vertical_orig;
	rs.include_group_effective = rs.include_group_orig;
	rs.include_type_effective = rs.include_type_orig;
	rs.include_emu_effective = rs.include_emu_orig;
	rs.menu_base_effective = rs.menu_base_orig;
	rs.menu_rel_effective = rs.menu_rel_orig;
	rs.lock_effective = rs.lock_orig;
	rs.video_orientation_effective = rs.video_orientation_orig;
}

void config_restore_save(config_state& rs) {
	rs.mode_orig = rs.mode_effective;
	rs.preview_orig = rs.preview_effective;
	rs.sort_orig = rs.sort_effective;
	rs.exclude_neogeo_orig = rs.exclude_neogeo_effective;
	rs.exclude_deco_orig = rs.exclude_deco_effective;
	rs.exclude_playchoice_orig = rs.exclude_playchoice_effective;
	rs.exclude_clone_orig = rs.exclude_clone_effective;
	rs.exclude_bad_orig = rs.exclude_bad_effective;
	rs.exclude_missing_orig = rs.exclude_missing_effective;
	rs.exclude_vector_orig = rs.exclude_vector_effective;
	rs.exclude_vertical_orig = rs.exclude_vertical_effective;
	rs.include_group_orig = rs.include_group_effective;
	rs.include_type_orig = rs.include_type_effective;
	rs.include_emu_orig = rs.include_emu_effective;
	rs.menu_base_orig = rs.menu_base_effective;
	rs.menu_rel_orig = rs.menu_rel_effective;
	rs.lock_orig = rs.lock_effective;
	rs.video_orientation_orig = rs.video_orientation_effective;
}
