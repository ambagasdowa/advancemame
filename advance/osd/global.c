/*
 * This file is part of the Advance project.
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
 *
 * In addition, as a special exception, Andrea Mazzoleni
 * gives permission to link the code of this program with
 * the MAME library (or with modified versions of MAME that use the
 * same license as MAME), and distribute linked combinations including
 * the two.  You must obey the GNU General Public License in all
 * respects for all of the code used other than MAME.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */

#include "emu.h"
#include "glue.h"
#include "log.h"
#include "target.h"

#include "mame2.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * Called while loading ROMs.
 * It is called a last time with name == 0 to signal that the ROM loading
 * process is finished.
 * \return
 *  - !=0 to abort loading
 *  - ==0 on success
 */
int osd_display_loading_rom_message(const char* name, struct rom_load_data* romdata)
{
	struct advance_global_context* context = &CONTEXT.global;

	log_std(("osd: osd_display_loading_rom_message(name:%s)\n", name));

	if (!romdata && name) {
		/* it's a message */
		if (!context->config.quiet_flag || strstr(name,"ERROR")!=0) {
			target_err("%s",name);
		}
	}

	return 0;
}

void advance_global_message(struct advance_global_context* context, const char* msg)
{
	log_std(("advance:global: set msg %s\n", msg));
	snprintf(context->state.msg, MESSAGE_MAX, "%s", msg);
}

void osd2_message(void)
{
	struct advance_global_context* context = &CONTEXT.global;

	/* display the stored message */
	if (context->state.msg[0]) {
		log_std(("advance:global: display msg %s\n", context->state.msg));
		mame_ui_message("%s", context->state.msg);
		context->state.msg[0] = 0;
	}
}

void osd_customize_inputport_defaults(struct ipd* defaults)
{
	/* nothing */
	(void)defaults;
}

const char* NAME_EASIEST[] = { "Easiest", "Very Easy", 0 };
const char* NAME_EASY[] = { "Easy", "Easier", "Easy?", 0 };
const char* NAME_MEDIUM[] = { "Medium", "Normal", "Normal?", 0 };
const char* NAME_HARD[] = { "Hard", "Harder", "Difficuly", "Hard?", 0 };
const char* NAME_HARDEST[] = { "Hardest", "Very Hard", "Very Difficult", 0 };

void osd_customize_inputport_current(struct InputPort* current)
{
	struct advance_global_context* context = &CONTEXT.global;

	const char** names;
	const char** names_secondary;
	struct InputPort* value;
	struct InputPort* level;
	struct InputPort* begin;
	struct InputPort* end;
	struct InputPort* i;

	names = 0;
	names_secondary = 0;
	value = 0;
	begin = 0;
	end = 0;

	i = current;
	while ((i->type & ~IPF_MASK) != IPT_END) {
		if ((i->type & ~IPF_MASK) == IPT_DIPSWITCH_NAME
			&& strcmp(i->name,"Difficulty")==0) {

			/* the value is stored in the NAME item */
			value = i;

			begin = ++i;

			/* read the value */
			while ((i->type & ~IPF_MASK) == IPT_DIPSWITCH_SETTING) {
				++i;
			}

			end = i;

			break;
		}
		++i;
	}

	if (!value) {
		log_std(("emu:global: difficulty dip switch not found\n"));
		return;
	}

	if (begin == end) {
		log_std(("emu:global: difficulty dip switch empty\n"));
		return;
	}

	/* start the search */
	level = 0;

	/* get the list of names */
	switch (context->config.difficulty) {
	case DIFFICULTY_NONE :
		/* nothing to do */
		return;
	case DIFFICULTY_EASIEST :
		names = NAME_EASIEST;
		names_secondary = NAME_EASY;
		break;
	case DIFFICULTY_EASY :
		names = NAME_EASY;
		names_secondary = 0;
		break;
	case DIFFICULTY_MEDIUM :
		names = NAME_MEDIUM;
		names_secondary = NAME_EASY;
		break;
	case DIFFICULTY_HARD :
		names = NAME_HARD;
		names_secondary = 0;
		break;
	case DIFFICULTY_HARDEST :
		names = NAME_HARDEST;
		names_secondary = NAME_HARD;
		break;
	}

	/* search an exact match */
	if (!level) {
		unsigned j;
		for(j=0;names[j] && !level;++j) {
			for(i=begin;i!=end && !level;++i) {
				if (strcmp(names[j],i->name)==0) {
					level = i;
					break;
				}
			}
		}
	}

	/* search an secondaryt match */
	if (!level && names_secondary) {
		unsigned j;
		for(j=0;names[j] && !level;++j) {
			for(i=begin;i!=end && !level;++i) {
				if (strcmp(names_secondary[j],i->name)==0) {
					level = i;
					break;
				}
			}
		}
	}

	/* interpolate */
	if (!level) {
		unsigned n = end - begin;
		switch (context->config.difficulty) {
		case DIFFICULTY_EASIEST :
			level = begin;
			break;
		case DIFFICULTY_EASY :
			level = begin + n / 4;
				break;
		case DIFFICULTY_MEDIUM :
			level = begin + n * 2 / 4;
			break;
		case DIFFICULTY_HARD :
			level = begin + n * 3 / 4;
			break;
		case DIFFICULTY_HARDEST :
			level = end - 1;
			break;
		}
	}

	if (!level) {
		log_std(("emu:global: difficulty dip switch unknown\n"));
		return;
	}

	/* set the difficulty */
	value->default_value = level->default_value;

	log_std(("emu:global: difficulty dip switch set to '%s'\n", level->name));
}

static adv_conf_enum_int OPTION_DIFFICULTY[] = {
{ "none", DIFFICULTY_NONE },
{ "easiest", DIFFICULTY_EASIEST },
{ "easy", DIFFICULTY_EASY },
{ "normal", DIFFICULTY_MEDIUM },
{ "hard", DIFFICULTY_HARD },
{ "hardest", DIFFICULTY_HARDEST }
};

adv_error advance_global_init(struct advance_global_context* context, adv_conf* cfg_context)
{
	conf_bool_register_default(cfg_context, "misc_quiet", 0);
	conf_int_register_enum_default(cfg_context, "misc_difficulty", conf_enum(OPTION_DIFFICULTY), DIFFICULTY_NONE);
	return 0;
}

adv_error advance_global_config_load(struct advance_global_context* context, adv_conf* cfg_context)
{
	context->config.quiet_flag = conf_bool_get_default(cfg_context, "misc_quiet");
	context->config.difficulty = conf_int_get_default(cfg_context, "misc_difficulty");
	return 0;
}

void advance_global_done(struct advance_global_context* context)
{
}

