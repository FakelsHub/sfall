/*
 *    sfall
 *    Copyright (C) 2010  The sfall team
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iterator>

#include "..\main.h"
#include "..\Utils.h"

#include "Reputations.h"

struct CityRep {
	DWORD globalID;
	DWORD cityID;
};

// C-array is neccessary, because it is used by game engine
static CityRep* repList = nullptr;

void Reputations::init() {
	auto cityRepList = GetConfigList("Misc", "CityRepsList", "", 512);
	auto count = cityRepList.size();
	if (count > 0) {
		repList = new CityRep[cityRepList.size()];
		std::vector<std::string> pair;
		pair.reserve(2);
		for (size_t i = 0; i < cityRepList.size(); i++) {
			pair.clear();
			split(cityRepList[i], ':', std::back_inserter(pair), 2);
			repList[i].cityID = atoi(pair[0].c_str());
			if (pair.size() >= 2) {
				repList[i].globalID = atoi(pair[1].c_str());
			}
		}

		SafeWrite32(0x43BEA5, (DWORD)&repList[0].cityID);
		SafeWrite32(0x43BF3C, (DWORD)&repList[0].cityID);
		SafeWrite32(0x43BF4C, (DWORD)&repList[0].globalID);
		SafeWrite32(0x43C03E, count * 8);
	}
}

void Reputations::exit() {
	if (repList != nullptr) {
		delete[] repList;
	}
}