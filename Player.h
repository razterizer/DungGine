//
//  Player.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-06-21.
//

#pragma once
#include <Core/StlUtils.h>


namespace dung
{
  
  struct Player
  {
    char character = '@';
    Style style = { Color::Magenta, Color::White };
    RC pos;
    bool is_spawned = false;
    BSPNode* curr_room = nullptr;
    Corridor* curr_corridor = nullptr;
    
    int health = 100;
    
    std::vector<int> key_idcs;
    std::vector<int> lamp_idcs;
    std::vector<int> weapon_idcs;
    int inv_hilite_idx = 0;
    std::vector<int> inv_select_idcs;
    int inv_select_idx_key = -1;
    int inv_select_idx_lamp = -1;
    int inv_select_idx_weapon = -1;
    bool show_inventory = false;
    RC line_of_sight;
    float weight_capacity = 50.f;
    
    bool using_key_id(const std::vector<Key>& all_keys, int key_id) const
    {
      if (math::in_range<int>(inv_select_idx_key,
                              start_inv_idx_keys(), end_inv_idx_keys(),
                              Range::Closed))
        if (all_keys[key_idcs[inv_select_idx_key]].key_id == key_id)
          return true;
      return false;
    }
    
    void remove_key_by_key_id(std::vector<Key>& all_keys, int key_id)
    {
      stlutils::erase_if(key_idcs, [&](int key_idx) { return all_keys[key_idx].key_id == key_id; });
      stlutils::erase_if(all_keys, [&](const auto& key) { return key.key_id == key_id; });
      inv_select_idx_key = -1;
    }
    
    const Key* get_selected_key(const std::vector<Key>& all_keys) const
    {
      if (math::in_range<int>(inv_select_idx_key,
                              start_inv_idx_keys(), end_inv_idx_keys(),
                              Range::Closed))
        return &all_keys[key_idcs[inv_select_idx_key - start_inv_idx_keys()]];
      return nullptr;
    }
    
    const Lamp* get_selected_lamp(const std::vector<Lamp>& all_lamps) const
    {
      if (math::in_range<int>(inv_select_idx_lamp,
                              start_inv_idx_lamps(), end_inv_idx_lamps(),
                              Range::Closed))
        return &all_lamps[lamp_idcs[inv_select_idx_lamp - start_inv_idx_lamps()]];
      return nullptr;
    }
    
    const Weapon* get_selected_weapon(const std::vector<std::unique_ptr<Weapon>>& all_weapons)
    {
     if (math::in_range<int>(inv_select_idx_weapon,
                              start_inv_idx_weapons(), end_inv_idx_weapons(),
                              Range::Closed))
        return all_weapons[weapon_idcs[inv_select_idx_weapon - start_inv_idx_weapons()]].get();
      return nullptr;
    }
    
    int num_items() const
    {
      return static_cast<int>(key_idcs.size() + lamp_idcs.size() + weapon_idcs.size());
    }
    
    int last_item_idx() const
    {
      return num_items() - 1;
    }
    
    int start_inv_idx_keys() const
    {
      return 0;
    }
    
    int end_inv_idx_keys() const
    {
      return static_cast<int>(key_idcs.size()) - 1;
    }
    
    int start_inv_idx_lamps() const
    {
      return static_cast<int>(key_idcs.size());
    }
    
    int end_inv_idx_lamps() const
    {
      return static_cast<int>(key_idcs.size() + lamp_idcs.size()) - 1;
    }
    
    int start_inv_idx_weapons() const
    {
      return static_cast<int>(key_idcs.size() + lamp_idcs.size());
    }
    
    int end_inv_idx_weapons() const
    {
      return static_cast<int>(key_idcs.size() + lamp_idcs.size() + weapon_idcs.size()) - 1;
    }
    
    bool in_keys_range(int idx) const
    {
      return math::in_range<int>(idx, start_inv_idx_keys(), end_inv_idx_keys(), Range::Closed);
    }
    
    bool in_lamps_range(int idx) const
    {
      return math::in_range<int>(idx, start_inv_idx_lamps(), end_inv_idx_lamps(), Range::Closed);
    }
    
    bool in_weapons_range(int idx) const
    {
      return math::in_range<int>(idx, start_inv_idx_weapons(), end_inv_idx_weapons(), Range::Closed);
    }
    
    bool is_inside_curr_room() const
    {
      if (curr_room != nullptr)
        return curr_room->bb_leaf_room.is_inside_offs(pos, -1);
      return false;
    }
    
    bool is_inside_curr_corridor() const
    {
      if (curr_corridor != nullptr)
        return curr_corridor->is_inside_corridor(pos);
      return false;
    }
  };
  
}
