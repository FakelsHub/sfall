// List of all engine variables and their types.
// Used to generate fo::var::var_name constants.

// TODO: assign appropriate types (arrays, structs, strings, etc.) for all variables

VAR_(aiInfoList,                 DWORD)
VAR_(ambient_light,              DWORD)
VARA(anim_set,                   AnimationSet, 32)
VARA(art,                        Art, 11)
VAR_(art_name,                   DWORD)
VAR_(art_vault_guy_num,          DWORD)
VAR_(art_vault_person_nums,      DWORD)
VAR_(bckgnd,                     BYTE*)
VAR_(black_palette,              DWORD)
VAR_(BlueColor,                  BYTE)
VAR_(bottom_line,                DWORD)
VAR_(btable,                     DWORD)
VAR_(btncnt,                     DWORD)
VAR_(carCurrentArea,             DWORD)
VAR_(carGasAmount,               long) // from 0 to 80000
VAR_(cmap,                       DWORD)
VAR_(colorTable,                 DWORD)
VAR_(combat_free_move,           DWORD)
VAR_(combat_list,                DWORD)
VAR_(combat_obj,                 GameObject*)
VAR_(combat_state,               DWORD)
VAR_(combat_turn_running,        DWORD)
VAR_(combatNumTurns,             DWORD)
VAR3(crit_succ_eff,              CritInfo, 20, 9, 6)  // 20 critters with 9 body parts and 6 effects each
VAR_(critter_db_handle,          DWORD)
VAR_(critterClearObj,            DWORD)
VAR_(crnt_func,                  DWORD)
VAR_(curr_font_num,              DWORD)
VARA(curr_pc_stat,               long, PCSTAT_max_pc_stat)
VAR_(curr_stack,                 DWORD)
VAR_(cursor_line,                DWORD)
VAR_(DARK_GREY_Color,             BYTE)
VAR_(DarkGreenColor,              BYTE)
VAR_(DarkGreenGreyColor,          BYTE)
VAR_(dialogue_state,             DWORD)
VAR_(dialogue_switch_mode,       DWORD)
VAR_(dialog_target,              DWORD)
VAR_(dialog_target_is_party,     DWORD)
VAR_(dropped_explosive,          DWORD)
VAR_(drugInfoList,               DWORD)
VAR_(edit_win,                   DWORD)
VAR_(Educated,                   DWORD)
VAR_(elevation,                  DWORD)
VAR_(Experience_,                DWORD)
VAR_(fallout_game_time,          DWORD)
VAR_(flptr,                      DWORD)
VAR_(folder_card_desc,           DWORD)
VAR_(folder_card_fid,            DWORD)
VAR_(folder_card_title,          DWORD)
VAR_(folder_card_title2,         DWORD)
VAR_(frame_time,                 DWORD)
VAR_(free_perk,                  char)
VAR_(game_global_vars,           long*)  // dynamic array of size == num_game_global_vars
VAR_(game_user_wants_to_quit,    DWORD)
VAR_(gcsd,                       DWORD)
VAR_(gdBarterMod,                DWORD)
VAR_(gdNumOptions,               DWORD)
VAR_(gIsSteal,                   DWORD)
VAR_(glblmode,                   DWORD)
VAR_(gmouse_current_cursor,      long)
VARA(gmovie_played_list,         BYTE, 17)
VAR_(GoodColor,                  BYTE)
VAR_(GreenColor,                 BYTE)
VAR_(gsound_initialized,         DWORD)
VARA(hit_location_penalty,       long, 9)
VAR_(holo_flag,                  DWORD)
VAR_(holopages,                  DWORD)
VAR_(hot_line_count,             DWORD)
VAR_(i_fid,                      DWORD)
VAR_(i_lhand,                    GameObject*)
VAR_(i_rhand,                    GameObject*)
VAR_(i_wid,                      DWORD)
VAR_(i_worn,                     GameObject*)
VAR_(idle_func,                  DWORD)
VAR_(In_WorldMap,                DWORD)
VAR_(info_line,                  DWORD)
VAR_(interfaceWindow,            DWORD)
VAR_(intfaceEnabled,             DWORD)
VAR_(intotal,                    DWORD)
VAR_(inven_dude,                 GameObject*)
VAR_(inven_pid,                  DWORD)
VAR_(inven_scroll_dn_bid,        DWORD)
VAR_(inven_scroll_up_bid,        DWORD)
VAR_(inventry_message_file,      MessageList)
VARA(itemButtonItems,            ItemButtonItem, 2) // 0 - left, 1 - right
VAR_(itemCurrentItem,            long)  // 0 - left, 1 - right
VAR_(kb_lock_flags,              DWORD)
VAR_(last_buttons,               DWORD)
VAR_(last_button_winID,          DWORD)
VAR_(last_level,                 DWORD)
VAR_(Level_,                     DWORD)
VAR_(Lifegiver,                  DWORD)
VAR_(LIGHT_GREY_Color,            BYTE)
VAR_(list_com,                   DWORD)
VAR_(list_total,                 DWORD)
VAR_(loadingGame,                DWORD)
VAR_(LSData,                     DWORD)
VAR_(lsgwin,                     DWORD)
VAR_(main_ctd,                   DWORD)
VAR_(main_window,                DWORD)
VAR_(map_elevation,              DWORD)
VAR_(map_global_vars,            long*)  // array
VAR_(master_db_handle,           DWORD)
VAR_(max,                        DWORD)
VAR_(maxScriptNum,               long)
VAR_(Meet_Frank_Horrigan,        bool)
VAR_(mouse_hotx,                 DWORD)
VAR_(mouse_hoty,                 DWORD)
VAR_(mouse_is_hidden,            DWORD)
VAR_(mouse_x_,                   DWORD)
VAR_(mouse_y,                    DWORD)
VAR_(mouse_y_,                   DWORD)
VAR_(Mutate_,                    DWORD)
VAR_(name_color,                 DWORD)
VAR_(name_font,                  DWORD)
VAR_(name_sort_list,             DWORD)
VAR_(num_game_global_vars,       DWORD)
VAR_(num_map_global_vars,        DWORD)
VAR_(obj_dude,                   GameObject*)
VAR_(objectTable,                DWORD)
VAR_(objItemOutlineState,        DWORD)
VAR_(optionRect,                 DWORD)
VAR_(optionsButtonDown,          DWORD)
VAR_(optionsButtonDown1,         DWORD)
VAR_(optionsButtonDownKey,       DWORD)
VAR_(optionsButtonUp,            DWORD)
VAR_(optionsButtonUp1,           DWORD)
VAR_(optionsButtonUpKey,         DWORD)
VAR_(outlined_object,            GameObject*)
VAR_(partyMemberAIOptions,       DWORD)
VAR_(partyMemberCount,           DWORD)
VAR_(partyMemberLevelUpInfoList, DWORD)
VAR_(partyMemberList,            DWORD*) // each struct - 4 integers, first integer - objPtr
VAR_(partyMemberMaxCount,        DWORD)
VAR_(partyMemberPidList,         DWORD)
VAR_(patches,                    char*)
VAR_(paths,                      PathNode*)  // array
VAR2(pc_crit_succ_eff,           CritInfo, 9, 6)  // 9 body parts, 6 effects
VAR_(pc_kill_counts,             DWORD)
VARA(pc_name,                    char, 32)
VAR_(pc_proto,                   Proto)
VARA(pc_trait,                   long, 2)  // 2 of them
VAR_(PeanutButter,               BYTE)
VARA(perk_data,                  PerkInfo, PERK_count)
VAR_(perkLevelDataList,          long*) // dynamic array, limited to PERK_Count
VAR_(pip_win,                    DWORD)
VAR_(pipboy_message_file,        MessageList)
VAR_(pipmesg,                    DWORD)
VAR_(preload_list_index,         DWORD)
VARA(procTableStrs,              const char*, (int)ScriptProc::count)  // table of procId (from define.h) => procName map
VARA(proto_msg_files,            MessageList, 6)  // array of 6 elements
VAR_(proto_main_msg_file,        MessageList)  
VAR_(ptable,                     DWORD)
VAR_(pud,                        DWORD)
VAR_(queue,                      DWORD)
VAR_(quick_done,                 DWORD)
VAR_(read_callback,              DWORD)
VAR_(RedColor,                   BYTE)
VAR_(rotation,                   DWORD)
VAR2(retvals,                    ElevatorExit, 24, 4)  // 24 elevators, 4 exits each
VAR_(script_path_base,           const char*)
VAR_(scr_size,                   DWORD)
VAR_(scriptListInfo,             ScriptListInfoItem*)  // dynamic array
VARA(skill_data,                 SkillInfo, SKILL_count)
VAR_(slot_cursor,                DWORD)
VAR_(sneak_working,              DWORD) // DWORD var
VAR_(sound_music_path1,          char*)
VAR_(sound_music_path2,          char*)
VAR_(square,                     DWORD)
VAR_(squares,                    DWORD*)
VAR_(stack,                      DWORD)
VAR_(stack_offset,               DWORD)
VARA(stat_data,                  StatInfo, STAT_real_max_stat) // dynamic array
VAR_(stat_flag,                  DWORD)
VAR_(Tag_,                       DWORD)
VAR_(tag_skill,                  DWORD)
VAR_(target_curr_stack,          DWORD)
VAR_(target_pud,                 DWORD)
VAR_(target_stack,               DWORD)
VAR_(target_stack_offset,        DWORD)
VAR_(target_str,                 DWORD)
VAR_(target_xpos,                DWORD)
VAR_(target_ypos,                DWORD)
VAR_(text_char_width,            DWORD)
VAR_(text_height,                DWORD)
VAR_(text_max,                   DWORD)
VAR_(text_mono_width,            DWORD)
VAR_(text_spacing,               DWORD)
VAR_(text_to_buf,                DWORD)
VAR_(text_width,                 DWORD)
VAR_(tile,                       DWORD)
VAR_(title_color,                DWORD)
VAR_(title_font,                 DWORD)
VARA(trait_data,                 TraitInfo, TRAIT_count)
VAR_(view_page,                  DWORD)
VAR_(wd_obj,                     DWORD)
VAR_(WhiteColor,                 BYTE)
VAR_(wmAreaInfoList,             DWORD)
VAR_(wmLastRndTime,              DWORD)
VAR_(wmWorldOffsetX,             DWORD)
VAR_(wmWorldOffsetY,             DWORD)
VAR_(world_xpos,                 DWORD)
VAR_(world_ypos,                 DWORD)
VAR_(WorldMapCurrArea,           DWORD)
VAR_(YellowColor,                BYTE)

#undef VAR_
#undef VARP
#undef VARA
#undef VAR2
#undef VAR3
