//
//  Player.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-06-21.
//

#pragma once

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
    
    int num_items() const
    {
      return static_cast<int>(key_idcs.size() + lamp_idcs.size());
    }
    
    int last_item_idx() const
    {
      return num_items() - 1;
    }
  };
  
}
