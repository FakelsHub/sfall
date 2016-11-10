/*
 *	sfall
 *	Copyright (C) 2008, 2009, 2010, 2011, 2012  The sfall team
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "..\main.h"

#include <cassert>
#include <hash_map>
#include <set>
#include <string>

#include "..\FalloutEngine\Fallout2.h"
#include "..\InputFuncs.h"
#include "..\Version.h"
#include "BarBoxes.h"
#include "Console.h"
#include "Explosions.h"
#include "HookScripts.h"
#include "LoadGameHook.h"
#include "..\Logging.h"
#include "Scripting\Arrays.h"
#include "Scripting\OpcodeHandler.h"

#include "ScriptExtender.h"


void _stdcall HandleMapUpdateForScripts(DWORD procId);

// TODO: move highligting-related code to separate file
static DWORD highlightingToggled = 0;
static DWORD MotionSensorMode;
static BYTE toggleHighlightsKey;
static DWORD HighlightContainers;
static DWORD Color_Containers;
static int idle;
static char HighlightFail1[128];
static char HighlightFail2[128];

struct sGlobalScript {
	sScriptProgram prog;
	int count;
	int repeat;
	int mode; //0 - local map loop, 1 - input loop, 2 - world map loop, 3 - local and world map loops

	sGlobalScript() {}
	sGlobalScript(sScriptProgram script) {
		prog = script;
		count = 0;
		repeat = 0;
		mode = 0;
	}
};

struct sExportedVar {
	int type; // in scripting engine terms, eg. VAR_TYPE_*
	int val;
	sExportedVar() : val(0), type(VAR_TYPE_INT) {}
};

static std::vector<TProgram*> checkedScripts;
static std::vector<sGlobalScript> globalScripts;
// a map of all sfall programs (global and hook scripts) by thier scriptPtr
typedef stdext::hash_map<TProgram*, sScriptProgram> SfallProgsMap;
static SfallProgsMap sfallProgsMap;
// a map scriptPtr => self_obj  to override self_obj for all script types using set_self
stdext::hash_map<TProgram*, TGameObj*> selfOverrideMap;

typedef std::tr1::unordered_map<std::string, sExportedVar> ExportedVarsMap;
static ExportedVarsMap globalExportedVars;
DWORD isGlobalScriptLoading = 0;

stdext::hash_map<__int64, int> globalVars;
typedef stdext::hash_map<__int64, int> :: iterator glob_itr;
typedef stdext::hash_map<__int64, int> :: const_iterator glob_citr;
typedef std::pair<__int64, int> glob_pair;

static void* opcodes[0x300];
DWORD AddUnarmedStatToGetYear = 0;
DWORD AvailableGlobalScriptTypes = 0;
bool isGameLoading;

TScript OverrideScriptStruct;

// handler for all new sfall opcodes
static OpcodeHandler opHandler;

#include "Scripting\AsmMacros.h"

#include "Scripting\AnimOps.hpp"
#include "Scripting\FileSystemOps.hpp"
#include "Scripting\GraphicsOp.hpp"
#include "Scripting\InterfaceOp.hpp"
#include "Scripting\MemoryOp.hpp"
#include "Scripting\MiscOps.hpp"
#include "Scripting\ObjectsOps.hpp"
#include "Scripting\PerksOp.hpp"
#include "Scripting\StatsOp.hpp"
#include "Scripting\ScriptArrays.hpp"
#include "Scripting\ScriptUtils.hpp"
#include "Scripting\WorldmapOps.hpp"

#include "Scripting\MetaruleOp.hpp"
#include "Scripting\Opcodes.hpp"

//END OF SCRIPT FUNCTIONS

static const DWORD scr_find_sid_from_program = FuncOffs::scr_find_sid_from_program_ + 5;
static const DWORD scr_ptr_back = FuncOffs::scr_ptr_ + 5;
static const DWORD scr_find_obj_from_program = FuncOffs::scr_find_obj_from_program_ + 7;

DWORD _stdcall FindSidHook2(TProgram* script) {
	stdext::hash_map<TProgram*, TGameObj*>::iterator overrideIt = selfOverrideMap.find(script);
	if (overrideIt != selfOverrideMap.end()) {
		DWORD scriptId = overrideIt->second->scriptID;
		if (scriptId != -1) {
			selfOverrideMap.erase(overrideIt);
			return scriptId; // returns the real scriptId of object if it is scripted
		}
		OverrideScriptStruct.self_obj = overrideIt->second;
		OverrideScriptStruct.target_obj = overrideIt->second;
		selfOverrideMap.erase(overrideIt); // this reverts self_obj back to original value for next function calls
		return -2; // override struct
	}
	// this will allow to use functions like roll_vs_skill, etc without calling set_self (they don't really need self object)
	if (sfallProgsMap.find(script) != sfallProgsMap.end()) {
		OverrideScriptStruct.target_obj = OverrideScriptStruct.self_obj = 0;
		return -2; // override struct
	}
	return -1; // change nothing
}

static void __declspec(naked) FindSidHook() {
	__asm {
		sub esp, 4;
		pushad;
		push eax;
		call FindSidHook2;
		mov [esp+32], eax; // to save value after popad
		popad;
		add esp, 4;
		cmp [esp-4], -1;
		jz end;
		cmp [esp-4], -2;
		jz override_script;
		mov eax, [esp-4];
		retn;
override_script:
		test edx, edx;
		jz end;
		lea eax, OverrideScriptStruct;
		mov [edx], eax;
		mov eax, -2;
		retn;
end:
		push ebx;
		push ecx;
		push edx;
		push esi;
		push ebp;
		jmp scr_find_sid_from_program;
	}
}

static void __declspec(naked) ScrPtrHook() {
	__asm {
		cmp eax, -2;
		jnz end;
		xor eax, eax;
		retn;
end:
		push ebx;
		push ecx;
		push esi;
		push edi;
		push ebp;
		jmp scr_ptr_back;
	}
}

static void __declspec(naked) MainGameLoopHook() {
	__asm {
		push ebx;
		push ecx;
		push edx;
		call FuncOffs::get_input_
		push eax;
		call RunGlobalScripts1;
		pop eax;
		pop edx;
		pop ecx;
		pop ebx;
		retn;
	}
}

static void __declspec(naked) CombatLoopHook() {
	__asm {
		pushad;
		call RunGlobalScripts1;
		popad;
		jmp  FuncOffs::get_input_
	}
}

static void __declspec(naked) AfterCombatAttackHook() {
	__asm {
		pushad;
		call AfterAttackCleanup;
		popad;
		mov  eax, 1;
		push 0x4230DA;
		retn;
	}
}

static void __declspec(naked) ExecMapScriptsHook() {
	__asm {
		sub esp, 32;
		mov [esp+12], eax;
		pushad;
		push eax; // int procId
		call HandleMapUpdateForScripts;
		popad;
		push 0x4A67F9; // jump back
		retn;
	}
}

static DWORD __stdcall GetGlobalExportedVarPtr(const char* name) {
	std::string str(name);
	ExportedVarsMap::iterator it = globalExportedVars.find(str);
	//dlog_f("\n Trying to find exported var %s... ", DL_MAIN, name);
	if (it != globalExportedVars.end()) {
		sExportedVar *ptr = &it->second;
		return (DWORD)ptr;
	}
	return 0;
}

static DWORD __stdcall CreateGlobalExportedVar(DWORD scr, const char* name) {
	//dlog_f("\nTrying to export variable %s (%d)\r\n", DL_MAIN, name, isGlobalScriptLoading);
	std::string str(name);
	globalExportedVars[str] = sExportedVar(); // add new
	return 1;
}

/*
	when fetching/storing into exported variables, first search in our own hash table instead, then (if not found), resume normal search

	reason for this: game frees all scripts and exported variables from memory when you exit map
	so if you create exported variable in sfall global script, it will work until you exit map, after that you will get crashes

	with this you can still use variables exported from global scripts even between map changes (per global scripts logic)
*/
static void __declspec(naked) Export_FetchOrStore_FindVar_Hook() {
	__asm {
		push eax;
		push edx;
		push ecx;
		push ebx;
		push edx // char* varName
		call GetGlobalExportedVarPtr;
		test eax, eax
		jz proceedNormal;
		pop ebx;
		pop ecx;
		pop edx;
		pop edx;
		sub eax, 0x24; // adjust pointer for the code
		retn;

	proceedNormal:
		pop ebx;
		pop ecx;
		pop edx;
		pop eax;
		call FuncOffs::findVar_
		retn
	}
}

static const DWORD Export_Export_FindVar_back1 = 0x4414CD;
static const DWORD Export_Export_FindVar_back2 = 0x4414AC;
static void __declspec(naked) Export_Export_FindVar_Hook() {
	__asm {
		pushad;
		mov eax, isGlobalScriptLoading;
		test eax, eax;
		jz proceedNormal;
		push edx; // char* - var name
		push ebp; // script ptr
		call CreateGlobalExportedVar;
		popad;
		jmp Export_Export_FindVar_back2; // if sfall exported var, jump to the end of function

proceedNormal:
		popad;
		call FuncOffs::findVar_;  // else - proceed normal
		jmp Export_Export_FindVar_back1;
	}
}

// this hook prevents sfall scripts from being removed after switching to another map, since normal script engine re-loads completely
static void _stdcall FreeProgramHook2(TProgram* progPtr) {
	if (isGameLoading || (sfallProgsMap.find(progPtr) == sfallProgsMap.end())) { // only delete non-sfall scripts or when actually loading the game
		__asm {
			mov eax, progPtr;
			call FuncOffs::interpretFreeProgram_;
		}
	}
}

static void __declspec(naked) FreeProgramHook() {
	__asm {
		pushad;
		push eax;
		call FreeProgramHook2;
		popad;
		retn;
	}
}

static void __declspec(naked) obj_outline_all_items_on() {
	__asm {
		pushad
		mov  eax, dword ptr ds:[VARPTR_map_elevation]
		call FuncOffs::obj_find_first_at_
loopObject:
		test eax, eax
		jz   end
		cmp  eax, ds:[VARPTR_outlined_object]
		je   nextObject
		xchg ecx, eax
		mov  eax, [ecx+0x20]
		and  eax, 0xF000000
		sar  eax, 0x18
		test eax, eax                             // This object is an item?
		jnz  nextObject                           // No
		cmp  dword ptr [ecx+0x7C], eax            // Owned by someone?
		jnz  nextObject                           // Yes
		test dword ptr [ecx+0x74], eax            // Already outlined?
		jnz  nextObject                           // Yes
		mov  edx, 0x10                            // yellow
		test byte ptr [ecx+0x25], dl              // NoHighlight_ flag is set (is this a container)?
		jz   NoHighlight                          // No
		cmp  HighlightContainers, eax             // Highlight containers?
		je   nextObject                           // No
		mov  edx, Color_Containers                // NR: should be set to yellow or purple later
NoHighlight:
		mov  [ecx+0x74], edx
nextObject:
		call FuncOffs::obj_find_next_at_
		jmp  loopObject
end:
		call FuncOffs::tile_refresh_display_
		popad
		retn
	}
}

static void __declspec(naked) obj_outline_all_items_off() {
	__asm {
		pushad
		mov  eax, dword ptr ds:[VARPTR_map_elevation]
		call FuncOffs::obj_find_first_at_
loopObject:
		test eax, eax
		jz   end
		cmp  eax, ds:[VARPTR_outlined_object]
		je   nextObject
		xchg ecx, eax
		mov  eax, [ecx+0x20]
		and  eax, 0xF000000
		sar  eax, 0x18
		test eax, eax                             // Is this an item?
		jnz  nextObject                           // No
		cmp  dword ptr [ecx+0x7C], eax            // Owned by someone?
		jnz  nextObject                           // Yes
		mov  dword ptr [ecx+0x74], eax
nextObject:
		call FuncOffs::obj_find_next_at_
		jmp  loopObject
end:
		call FuncOffs::tile_refresh_display_
		popad
		retn
	}
}

static void __declspec(naked) obj_remove_outline_hook() {
	__asm {
		call FuncOffs::obj_remove_outline_
		test eax, eax
		jnz  end
		cmp  highlightingToggled, 1
		jne  end
		mov  ds:[VARPTR_outlined_object], eax
		call obj_outline_all_items_on
end:
		retn
	}
}

void ScriptExtenderSetup() {

	toggleHighlightsKey = GetPrivateProfileIntA("Input", "ToggleItemHighlightsKey", 0, ini);
	if (toggleHighlightsKey) {
		MotionSensorMode = GetPrivateProfileIntA("Misc", "MotionScannerFlags", 1, ini);
		HighlightContainers = GetPrivateProfileIntA("Input", "HighlightContainers", 0, ini);
		switch (HighlightContainers) {
		case 1:
			Color_Containers = 0x10; // yellow
			break;
		case 2:
			Color_Containers = 0x40; // purple
			break;
		}
		//HookCall(0x44B9BA, &gmouse_bk_process_hook);
		HookCall(0x44BD1C, &obj_remove_outline_hook);
		HookCall(0x44E559, &obj_remove_outline_hook);
	}
	GetPrivateProfileStringA("Sfall", "HighlightFail1", "You aren't carrying a motion sensor.", HighlightFail1, 128, translationIni);
	GetPrivateProfileStringA("Sfall", "HighlightFail2", "Your motion sensor is out of charge.", HighlightFail2, 128, translationIni);

	idle=GetPrivateProfileIntA("Misc", "ProcessorIdle", -1, ini);
	modifiedIni=GetPrivateProfileIntA("Main", "ModifiedIni", 0, ini);
	
	arraysBehavior = GetPrivateProfileIntA("Misc", "arraysBehavior", 1, ini);
	if (arraysBehavior > 0) {
		arraysBehavior = 1; // only 1 and 0 allowed at this time
		dlogr("New arrays behavior enabled.", DL_SCRIPT);
	} else {
		dlogr("Arrays in backward-compatiblity mode.", DL_SCRIPT);
	}
	
	HookCall(0x480E7B, MainGameLoopHook); //hook the main game loop
	HookCall(0x422845, CombatLoopHook); //hook the combat loop

	MakeCall(0x4A390C, &FindSidHook, true);
	MakeCall(0x4A5E34, &ScrPtrHook, true);
	memset(&OverrideScriptStruct, 0, sizeof(TScript));

	MakeCall(0x4230D5, &AfterCombatAttackHook, true);
	MakeCall(0x4A67F2, &ExecMapScriptsHook, true);

	// this patch makes it possible to export variables from sfall global scripts
	MakeCall(0x4414C8, &Export_Export_FindVar_Hook, true);
	HookCall(0x441285, &Export_FetchOrStore_FindVar_Hook); // store
	HookCall(0x4413D9, &Export_FetchOrStore_FindVar_Hook); // fetch

	// fix vanilla negate operator on float values
	MakeCall(0x46AB63, &NegateFixHook, true);
	// fix incorrect int-to-float conversion
	// op_mult:
	SafeWrite16(0x46A3F4, 0x04DB); // replace operator to "fild 32bit"
	SafeWrite16(0x46A3A8, 0x04DB);
	// op_div:
	SafeWrite16(0x46A566, 0x04DB);
	SafeWrite16(0x46A4E7, 0x04DB);

	HookCall(0x46E141, FreeProgramHook);
	
	InitNewOpcodes();
	InitOpcodeMetaTable();
	InitMetaruleTable();
}


// loads script from .int file into a sScriptProgram struct, filling script pointer and proc lookup table
void LoadScriptProgram(sScriptProgram &prog, const char* fileName) {
	TProgram* scriptPtr = Wrapper::loadProgram(fileName);
	if (scriptPtr) {
		const char** procTable = VarPtr::procTableStrs;
		prog.ptr = scriptPtr;
		// fill lookup table
		for (int i=0; i<=SCRIPT_PROC_MAX; i++) {
			prog.procLookup[i] = Wrapper::interpretFindProcedure(prog.ptr, procTable[i]);
		}
		prog.initialized = 0;
	} else {
		prog.ptr = NULL;
	}
}

void InitScriptProgram(sScriptProgram &prog) {
	if (prog.initialized == 0) {
		Wrapper::runProgram(prog.ptr);
		Wrapper::interpret(prog.ptr, -1);
		prog.initialized = 1;
	}
}

void AddProgramToMap(sScriptProgram &prog) {
	sfallProgsMap[prog.ptr] = prog;
}

sScriptProgram* GetGlobalScriptProgram(TProgram* scriptPtr) {
	for (std::vector<sGlobalScript>::iterator it = globalScripts.begin(); it != globalScripts.end(); it++) {
		if (it->prog.ptr == scriptPtr) return &it->prog;
	}
	return nullptr;
}

bool _stdcall IsGameScript(const char* filename) {
	// TODO: write better solution
	for (DWORD i = 0; i < *VarPtr::maxScriptNum; i++) {
		if (strcmp(filename, (char*)(*VarPtr::scriptListInfo + i*20)) == 0) return true;
	}
	return false;
}

// this runs after the game was loaded/started
void LoadGlobalScripts() {
	isGameLoading = false;
	HookScriptInit();
	dlogr("Loading global scripts", DL_SCRIPT|DL_INIT);

	char* name = "scripts\\gl*.int";
	char* *filenames;
	int count = Wrapper::db_get_file_list(name, &filenames, 0, 0);

	// TODO: refactor script programs
	sScriptProgram prog;
	for (int i = 0; i < count; i++) {
		name = _strlwr(filenames[i]);
		name[strlen(name) - 4] = 0;
		if (!IsGameScript(name)) {
			dlog(">", DL_SCRIPT);
			dlog(name, DL_SCRIPT);
			isGlobalScriptLoading = 1;
			LoadScriptProgram(prog, name);
			if (prog.ptr) {
				dlogr(" Done", DL_SCRIPT);
				DWORD idx;
				sGlobalScript gscript = sGlobalScript(prog);
				idx = globalScripts.size();
				globalScripts.push_back(gscript);
				AddProgramToMap(prog);
				// initialize script (start proc will be executed for the first time) -- this needs to be after script is added to "globalScripts" array
				InitScriptProgram(prog);
			} else {
				dlogr(" Error!", DL_SCRIPT);
			}
			isGlobalScriptLoading = 0;
		}
	}
	Wrapper::db_free_file_list(&filenames, 0);
	dlogr("Finished loading global scripts", DL_SCRIPT|DL_INIT);
	//ButtonsReload();
}

static bool _stdcall ScriptHasLoaded(TProgram* script) {
	for (size_t d = 0; d < checkedScripts.size(); d++) {
		if (checkedScripts[d] == script) {
			return 0;
		}
	}
	checkedScripts.push_back(script);
	return 1;
}

// this runs before actually loading/starting the game
void ClearGlobalScripts() {
	isGameLoading = true;
	checkedScripts.clear();
	sfallProgsMap.clear();
	globalScripts.clear();
	selfOverrideMap.clear();
	globalExportedVars.clear();
	HookScriptClear();

	//Reset some settable game values back to the defaults
	//Pyromaniac bonus
	SafeWrite8(0x424AB6, 5);
	//xp mod
	SafeWrite8(0x004AFAB8, 0x53);
	SafeWrite32(0x004AFAB9, 0x55575651);
	//Perk level mod
	SafeWrite32(0x00496880, 0x00019078);
	//HP bonus
	SafeWrite8(0x4AFBC1, 2);
	//Stat ranges
	StatsReset();
	//Bodypart hit chances
	*((DWORD*)0x510954) = GetPrivateProfileIntA("Misc", "BodyHit_Head",      0xFFFFFFD8, ini);
	*((DWORD*)0x510958) = GetPrivateProfileIntA("Misc", "BodyHit_Left_Arm",  0xFFFFFFE2, ini);
	*((DWORD*)0x51095C) = GetPrivateProfileIntA("Misc", "BodyHit_Right_Arm", 0xFFFFFFE2, ini);
	*((DWORD*)0x510960) = GetPrivateProfileIntA("Misc", "BodyHit_Torso",     0x00000000, ini);
	*((DWORD*)0x510964) = GetPrivateProfileIntA("Misc", "BodyHit_Right_Leg", 0xFFFFFFEC, ini);
	*((DWORD*)0x510968) = GetPrivateProfileIntA("Misc", "BodyHit_Left_Leg",  0xFFFFFFEC, ini);
	*((DWORD*)0x51096C) = GetPrivateProfileIntA("Misc", "BodyHit_Eyes",      0xFFFFFFC4, ini);
	*((DWORD*)0x510970) = GetPrivateProfileIntA("Misc", "BodyHit_Groin",     0xFFFFFFE2, ini);
	*((DWORD*)0x510974) = GetPrivateProfileIntA("Misc", "BodyHit_Torso",     0x00000000, ini);
	//skillpoints per level mod
	SafeWrite8(0x43C27a, 5);
}



void RunScriptProc(sScriptProgram* prog, const char* procName) {
	TProgram* sptr = prog->ptr;
	int procNum = Wrapper::interpretFindProcedure(sptr, procName);
	if (procNum != -1) {
		Wrapper::executeProcedure(sptr, procNum);
	}
}

void RunScriptProc(sScriptProgram* prog, int procId) {
	if (procId > 0 && procId <= SCRIPT_PROC_MAX) {
		TProgram* sptr = prog->ptr;
		int procNum = prog->procLookup[procId];
		if (procNum != -1) {
			Wrapper::executeProcedure(sptr, procNum);
		}
	}
}

static void RunScript(sGlobalScript* script) {
	script->count=0;
	RunScriptProc(&script->prog, start); // run "start"
}

/**
	Do some clearing after each frame:
	- delete all temp arrays
	- reset reg_anim_* combatstate checks
*/
static void ResetStateAfterFrame() {
	if (tempArrays.size()) {
 		for (std::set<DWORD>::iterator it = tempArrays.begin(); it != tempArrays.end(); ++it)
			FreeArray(*it);
		tempArrays.clear();
	}
	RegAnimCombatCheck(1);
}

/**
	Do some cleaning after each combat attack action
*/
void AfterAttackCleanup() {
	ResetExplosionSettings();
}

static void RunGlobalScripts1() {
	if (idle >- 1) Sleep(idle);
	if (toggleHighlightsKey) {
		//0x48C294 to toggle
		if (KeyDown(toggleHighlightsKey)) {
			if (!highlightingToggled) {
				if (MotionSensorMode&4) {
					TGameObj* scanner = Wrapper::inven_pid_is_carried_ptr(*VarPtr::obj_dude, PID_MOTION_SENSOR);
					if (scanner != nullptr) {
						if (MotionSensorMode & 2) {
							highlightingToggled = Wrapper::item_m_dec_charges(scanner) + 1;
							if (!highlightingToggled) {
								Wrapper::display_print(HighlightFail2);
							}
						} else highlightingToggled = 1;
					} else {
						Wrapper::display_print(HighlightFail1);
					}
				} else {
					highlightingToggled = 1;
				}
				if (highlightingToggled) {
					obj_outline_all_items_on();
				} else {
					highlightingToggled = 2;
				}
			}
		} else if (highlightingToggled) {
			if (highlightingToggled == 1) {
				obj_outline_all_items_off();  
			} 
			highlightingToggled = 0;
		}
	}
	for (DWORD d=0; d<globalScripts.size(); d++) {
		if (!globalScripts[d].repeat || (globalScripts[d].mode != 0 && globalScripts[d].mode != 3)) continue;
		if (++globalScripts[d].count >= globalScripts[d].repeat) {
			RunScript(&globalScripts[d]);
		}
	}
	ResetStateAfterFrame();
}

void RunGlobalScripts2() {
	if (idle >- 1) {
		Sleep(idle);
	}
	for (size_t d = 0; d < globalScripts.size(); d++) {
		if (!globalScripts[d].repeat || globalScripts[d].mode != 1) continue;
		if (++globalScripts[d].count >= globalScripts[d].repeat) {
			RunScript(&globalScripts[d]);
		}
	}
	ResetStateAfterFrame();
}

void RunGlobalScripts3() {
	if (idle >- 1) {
		Sleep(idle);
	}
	for (size_t d=0; d < globalScripts.size(); d++) {
		if (!globalScripts[d].repeat || (globalScripts[d].mode != 2 && globalScripts[d].mode != 3)) continue;
		if (++globalScripts[d].count >= globalScripts[d].repeat) {
			RunScript(&globalScripts[d]);
		}
	}
	ResetStateAfterFrame();
}

void _stdcall HandleMapUpdateForScripts(DWORD procId) {
	if (procId == map_enter_p_proc) {
		// map changed, all game objects were destroyed and scripts detached, need to re-insert global scripts into the game
		for (SfallProgsMap::iterator it = sfallProgsMap.begin(); it != sfallProgsMap.end(); it++) {
			Wrapper::runProgram(it->second.ptr);
		}
	}
	RunGlobalScriptsAtProc(procId); // gl* scripts of types 0 and 3
	RunHookScriptsAtProc(procId); // all hs_ scripts
}

// run all global scripts of types 0 and 3 at specific procedure (if exist)
void _stdcall RunGlobalScriptsAtProc(DWORD procId) {
	for (DWORD d = 0; d < globalScripts.size(); d++) {
		if (globalScripts[d].mode != 0 && globalScripts[d].mode != 3) continue;
		RunScriptProc(&globalScripts[d].prog, procId);
	}
}

void LoadGlobals(HANDLE h) {
	DWORD count, unused;
	ReadFile(h, &count, 4, &unused, 0);
	if (unused!=4) return;
	sGlobalVar var;
	for (DWORD i = 0; i<count; i++) {
		ReadFile(h, &var, sizeof(sGlobalVar), &unused, 0);
		globalVars.insert(glob_pair(var.id, var.val));
	}
}

void SaveGlobals(HANDLE h) {
	DWORD count, unused;
	count = globalVars.size();
	WriteFile(h, &count, 4, &unused, 0);
	sGlobalVar var;
	glob_citr itr = globalVars.begin();
	while (itr != globalVars.end()) {
		var.id = itr->first;
		var.val = itr->second;
		WriteFile(h, &var, sizeof(sGlobalVar), &unused, 0);
		itr++;
	}
}

void ClearGlobals() {
	globalVars.clear();
	for (array_itr it = arrays.begin(); it != arrays.end(); ++it) {
		it->second.clear();
	}
	arrays.clear();
	savedArrays.clear();
}

int GetNumGlobals() { 
	return globalVars.size(); 
}

void GetGlobals(sGlobalVar* globals) {
	glob_citr itr = globalVars.begin();
	int i = 0;
	while (itr != globalVars.end()) {
		globals[i].id = itr->first;
		globals[i++].val = itr->second;
		itr++;
	}
}

void SetGlobals(sGlobalVar* globals) {
	glob_itr itr = globalVars.begin();
	int i = 0;
	while(itr != globalVars.end()) {
		itr->second = globals[i++].val;
		itr++;
	}
}

//fuctions to load and save appearance globals
void SetAppearanceGlobals(int race, int style) {
	SetGlobalVar2("HAp_Race", race);
	SetGlobalVar2("HApStyle", style);
}

void GetAppearanceGlobals(int *race, int *style) {
	*race=GetGlobalVar2("HAp_Race");
	*style=GetGlobalVar2("HApStyle");
}