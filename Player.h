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
    
    std::vector<int> key_idcs;
    std::vector<int> lamp_idcs;
    std::vector<int> weapon_idcs;
    int inv_hilite_idx = 0;
    int inv_select_idx = -1;
    bool show_inventory = false;
    RC line_of_sight;
    float weight_capacity = 50.f;
    
    bool using_key_id(const std::vector<Key>& all_keys, int key_id) const
    {
      auto N = static_cast<int>(key_idcs.size());
      if (math::in_range<int>(inv_select_idx, 0, N, Range::ClosedOpen))
        if (all_keys[key_idcs[inv_select_idx]].key_id == key_id)
          return true;
      return false;
    }
    
    void remove_key_by_key_id(std::vector<Key>& all_keys, int key_id)
    {
      stlutils::erase_if(key_idcs, [&](int key_idx) { return all_keys[key_idx].key_id == key_id; });
      stlutils::erase_if(all_keys, [&](const auto& key) { return key.key_id == key_id; });
      inv_select_idx = -1;
    }
    
    const Key* get_selected_key(const std::vector<Key>& all_keys) const
    {
      auto N = static_cast<int>(key_idcs.size());
      if (math::in_range<int>(inv_select_idx, 0, N, Range::ClosedOpen))
        return &all_keys[key_idcs[inv_select_idx]];
      return nullptr;
    }
    
    const Lamp* get_selected_lamp(const std::vector<Lamp>& all_lamps) const
    {
      auto Nk = static_cast<int>(key_idcs.size());
      auto Nl = static_cast<int>(lamp_idcs.size());
      if (math::in_range<int>(inv_select_idx, Nk, Nk + Nl, Range::ClosedOpen))
        return &all_lamps[lamp_idcs[inv_select_idx - Nk]];
      return nullptr;
    }
    
    const Weapon* get_selected_weapon(const std::vector<std::unique_ptr<Weapon>>& all_weapons)
    {
      auto Nk = static_cast<int>(key_idcs.size());
      auto Nl = static_cast<int>(lamp_idcs.size());
      auto Nw = static_cast<int>(weapon_idcs.size());
      if (math::in_range<int>(inv_select_idx, Nk + Nl, Nk + Nl + Nw, Range::ClosedOpen))
        return all_weapons[weapon_idcs[inv_select_idx - (Nk + Nl)]].get();
      return nullptr;
    }
    
    int num_items() const
    {
      return static_cast<int>(key_idcs.size() + lamp_idcs.size());
    }
    
    int last_item_idx() const
    {
      return num_items() - 1;
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
