/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * UniverseStore.cpp
 * The universe store class. This maintains the set of all active universes and
 * saves the settings.
 * Copyright (C) 2005-2008 Simon Newton
 */
#include <string>
#include <sstream>
#include <iostream>

#include <llad/Universe.h>
#include <llad/Preferences.h>
#include "UniverseStore.h"

namespace lla {


/*
 * Lookup a universe from its universe_id
 * @param uid the uid of the required universe
 */
Universe *UniverseStore::GetUniverse(int universe_id) {
  map<int, Universe*>::const_iterator iter;

  iter = m_universe_map.find(universe_id);
  if (iter != m_universe_map.end()) {
     return iter->second;
  }
  return NULL;
}


/*
 * Lookup a universe, or create it if it does not exist
 *
 * @param uid  the universe id
 * @return  the universe, or NULL on error
 */
Universe *UniverseStore::GetUniverseOrCreate(int universe_id) {
  Universe *universe = GetUniverse(universe_id);

  if (!universe) {
    universe = new Universe(universe_id, this);

    if (universe) {
      pair<int , Universe*> pair(universe_id, universe);
      m_universe_map.insert(pair);

      if (m_preferences)
        RestoreUniverseSettings(universe);
    }
  }
  return universe;
}


/*
 * Returns a list of universes. This must be freed when you're
 * done with it.
 *
 * @return a pointer to a vector of Universe*
 */
vector<Universe *> *UniverseStore::GetList() const {
  vector<Universe*> *list = new vector<Universe*>;
  list->reserve(UniverseCount());

  map<int ,Universe*>::const_iterator iter;
  for (iter = m_universe_map.begin(); iter != m_universe_map.end(); ++iter)
    list->push_back(iter->second);
  return list;
}


/*
 * Delete all universes
 */
int UniverseStore::DeleteAll() {
  map<int, Universe*>::iterator iter;

  for (iter = m_universe_map.begin(); iter != m_universe_map.end(); iter++) {
    SaveUniverseSettings(iter->second);
    delete iter->second;
  }
  m_universe_map.clear();
  return 0;
}


/*
 * Delete this universe if it no longer has any ports or clients.
 *
 * @returns true if this universe was deleted, false otherwise.
 */
bool UniverseStore::DeleteUniverseIfInactive(Universe *universe) {
  if (universe->IsActive())
    return false;

  m_universe_map.erase(universe->UniverseId());
  SaveUniverseSettings(universe);
  delete universe;
  return true;
}


/*
 * Check for unused universes and delete them
void UniverseStore::CheckForUnused() {
  map<int ,Universe*>::const_iterator iter;
  vector<Universe*>::iterator iterv;
  vector<Universe*> list;

  for (iter = m_universe_map.begin(); iter != m_universe_map.end(); ++iter) {
    if (!iter->second->IsActive()) {
      list.push_back(iter->second);
    }
  }

  for (iterv = list.begin(); iterv != list.end(); ++iterv) {
    m_universe_map.erase((*iterv)->UniverseId());
    delete *iterv;
  }
}
 */


/*
 * Restore a universe's settings
 *
 * @param uni  the universe to update
 */
int UniverseStore::RestoreUniverseSettings(Universe *universe) const {
  string key, value;
  std::ostringstream oss;

  if (!universe)
    return 0;

  oss << std::dec << universe->UniverseId();

  // load name
  key = "uni_" + oss.str() + "_name";
  value = m_preferences->GetValue(key);

  if (!value.empty())
    universe->SetName(value);

  // load merge mode
  key = "uni_" + oss.str() + "_merge";
  value = m_preferences->GetValue(key);

  if (!value.empty()) {
    if (value == "HTP")
      universe->SetMergeMode(Universe::MERGE_HTP);
    else
      universe->SetMergeMode(Universe::MERGE_LTP);
  }
  return 0;
}


/*
 * Save this universe's settings.
 *
 * @param uni  the universe to save
 */
int UniverseStore::SaveUniverseSettings(Universe *universe) {
  string key, mode;
  std::ostringstream oss;

  if (!universe)
    return 0;

  oss << std::dec << universe->UniverseId();

  // save name
  key = "uni_" + oss.str() + "_name";
  m_preferences->SetValue(key, universe->Name());

  // save merge mode
  key = "uni_" + oss.str() + "_merge";
  mode = (universe->MergeMode() == Universe::MERGE_HTP ? "HTP" : "LTP");
  m_preferences->SetValue(key, mode);

  return 0;
}

} //lla
