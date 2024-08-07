//
//  Player.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-06-21.
//

#pragma once
#include <Core/StlUtils.h>
#include "Globals.h"


namespace dung
{
  
  enum class Terrain { Default, Water, Sand };
  
  struct Player
  {
    char character = '@';
    Style style = { Color::Magenta, Color::White };
    RC pos, last_pos;
    float los_r = 0.f;
    float los_c = 0.f;
    float last_los_r = 0.f;
    float last_los_c = 0.f;
    bool is_spawned = false;
    BSPNode* curr_room = nullptr;
    Corridor* curr_corridor = nullptr;
    
    int health = globals::max_health;
    int base_ac = 10;
    int thac0 = 1;
    int strength = 10;
    int dexterity = 10;
    int endurance = 10;
    int weakness = 0;
    
    Terrain on_terrain = Terrain::Default;
    
    std::vector<int> key_idcs;
    std::vector<int> lamp_idcs;
    std::vector<int> weapon_idcs;
    std::vector<int> potion_idcs;
    std::vector<int> armour_idcs;
    int inv_hilite_idx = 0; // index over over key_idcs through armour_idcs.
    std::vector<int> inv_select_idcs; // indices over key_idcs through armour_idcs.
    int inv_select_idx_key = -1;
    int inv_select_idx_lamp = -1;
    int inv_select_idx_weapon = -1;
    int inv_select_idx_potion = -1;
    int inv_select_idx_armour = -1;
    bool show_inventory = false;
    float weight_capacity = 50.f;
    
    bool allow_move()
    {
      if (on_terrain == Terrain::Water)
        return !rnd::one_in(strength);
      else if (on_terrain == Terrain::Sand)
        return rnd::one_in(3);
      return true;
    }
    
    void update()
    {
      if (pos != last_pos)
      {
        los_r = pos.r - last_pos.r;
        los_c = pos.c - last_pos.c;
        los_r += last_los_r;
        los_c += last_los_c;
        math::normalize(los_r, los_c);
        last_los_r = los_r;
        last_los_c = los_c;
      }
      last_pos = pos;
      
      if (on_terrain == Terrain::Water)
      {
        if (rnd::one_in(endurance) && weakness < endurance)
          weakness++;
      
        if (rnd::one_in(endurance - weakness))
        {
          strength--;
          if (strength <= 0)
            health -= math::roundI(globals::max_health*0.002f);
        }
      }
      else
      {
        if (rnd::one_in(2) && 0 < weakness)
          weakness--;
      }
    }
    
    int calc_armour_class(const std::vector<std::unique_ptr<Armour>>& all_armour) const
    {
      int tot_protection = 0;
      for (const auto& a_idx : armour_idcs)
        tot_protection += all_armour[a_idx]->protection;
      return base_ac + tot_protection + (dexterity / 2); // Example: Include dexterity bonus
    }
    
    // Function to calculate melee attack bonus
    int get_melee_attack_bonus() const
    {
        return strength / 2; // Example: Strength bonus to attack rolls
    }

    // Function to calculate melee damage bonus
    int get_melee_damage_bonus() const
    {
        return strength / 2; // Example: Strength bonus to damage
    }
    
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
      if (inv_select_idx_key != -1)
      {
        stlutils::erase(inv_select_idcs, inv_select_idx_key);
        inv_select_idx_key = -1;
      }
    }
    
    void remove_selected_potion(std::vector<Potion>& all_potions)
    {
      if (inv_select_idx_potion == -1)
        return;
      stlutils::erase_at(all_potions, potion_idcs[inv_select_idx_potion - start_inv_idx_potions()]);
      stlutils::erase_at(potion_idcs, inv_select_idx_potion - start_inv_idx_potions());
      if (inv_select_idx_potion != -1)
      {
        stlutils::erase(inv_select_idcs, inv_select_idx_potion);
        inv_select_idx_potion = -1;
      }
    }
    
    Key* get_selected_key(std::vector<Key>& all_keys) const
    {
      if (in_keys_range(inv_select_idx_key))
        return &all_keys[key_idcs[inv_select_idx_key - start_inv_idx_keys()]];
      return nullptr;
    }
    
    Lamp* get_selected_lamp(std::vector<Lamp>& all_lamps) const
    {
      if (in_lamps_range(inv_select_idx_lamp))
        return &all_lamps[lamp_idcs[inv_select_idx_lamp - start_inv_idx_lamps()]];
      return nullptr;
    }
    
    Weapon* get_selected_weapon(std::vector<std::unique_ptr<Weapon>>& all_weapons)
    {
     if (in_weapons_range(inv_select_idx_weapon))
        return all_weapons[weapon_idcs[inv_select_idx_weapon - start_inv_idx_weapons()]].get();
      return nullptr;
    }
    
    Potion* get_selected_potion(std::vector<Potion>& all_potions)
    {
     if (in_potions_range(inv_select_idx_potion))
        return &all_potions[potion_idcs[inv_select_idx_potion - start_inv_idx_potions()]];
      return nullptr;
    }
    
    Armour* get_selected_armour(std::vector<std::unique_ptr<Armour>>& all_armour)
    {
     if (in_armour_range(inv_select_idx_armour))
        return all_armour[armour_idcs[inv_select_idx_armour - start_inv_idx_armour()]].get();
      return nullptr;
    }
    
    int num_items() const
    {
      return static_cast<int>(key_idcs.size() + lamp_idcs.size() + weapon_idcs.size() + potion_idcs.size() + armour_idcs.size());
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
      return start_inv_idx_lamps() - 1;
    }
    
    int start_inv_idx_lamps() const
    {
      return start_inv_idx_keys() + static_cast<int>(key_idcs.size());
    }
    
    int end_inv_idx_lamps() const
    {
      return start_inv_idx_weapons() - 1;
    }
    
    int start_inv_idx_weapons() const
    {
      return start_inv_idx_lamps() + static_cast<int>(lamp_idcs.size());
    }
    
    int end_inv_idx_weapons() const
    {
      return start_inv_idx_potions() - 1;
    }
    
    int start_inv_idx_potions() const
    {
      return start_inv_idx_weapons() + static_cast<int>(weapon_idcs.size());
    }
    
    int end_inv_idx_potions() const
    {
      return start_inv_idx_armour() - 1;
    }
    
    int start_inv_idx_armour() const
    {
      return start_inv_idx_potions() + static_cast<int>(potion_idcs.size());
    }
    
    int end_inv_idx_armour() const
    {
      return num_items() - 1;
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
    
    bool in_potions_range(int idx) const
    {
      return math::in_range<int>(idx, start_inv_idx_potions(), end_inv_idx_potions(), Range::Closed);
    }
    
    bool in_armour_range(int idx) const
    {
      return math::in_range<int>(idx, start_inv_idx_armour(), end_inv_idx_armour(), Range::Closed);
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
