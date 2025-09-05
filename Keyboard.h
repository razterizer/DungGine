//
//  Keyboard.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-09-16.
//

#pragma once
#include "Environment.h"
#include "Inventory.h"
#include "PC.h"
#include "Items.h"
#include <Termin8or/MessageHandler.h>
#include <Core/Utils.h>
#include <functional>

using namespace std::string_literals;


namespace dung
{
  using MessageHandler = t8x::ui::MessageHandler;
  

  class Keyboard
  {
    Environment* m_environment = nullptr;
    Inventory* m_inventory = nullptr;
    MessageHandler* message_handler = nullptr;
    
    PC& m_player;
    
    std::vector<Key>& m_all_keys;
    // Lamps illuminate items and NPCs. If you've already discovered an item or
    //   NPC using a lamp (and after FOW been cleared),
    //   then they will still be visible when the room is not lit.
    // Lamps will not work in surface level rooms.
    std::vector<Lamp>& m_all_lamps;
    std::vector<std::unique_ptr<Weapon>>& m_all_weapons;
    std::vector<Potion>& m_all_potions;
    std::vector<std::unique_ptr<Armour>>& m_all_armour;
    
    std::vector<NPC>& m_all_npcs;
    
    bool& m_trigger_game_save;
    bool& m_trigger_game_load;
    bool& m_trigger_screenshot;
    
    t8x::ui::TextBoxDebug& m_tbd;
    bool& m_debug;
    
    void drop_item(Item* obj, const RC& curr_pos)
    {
      obj->picked_up = false;
      obj->pos = curr_pos;
      obj->curr_floor = m_player.curr_floor;
      if (m_player.is_inside_curr_room())
      {
        obj->is_underground = m_environment->is_underground(m_player.curr_floor, m_player.curr_room);
        obj->curr_room = m_player.curr_room;
        obj->curr_corridor = nullptr;
      }
      else if (m_player.is_inside_curr_corridor())
      {
        obj->is_underground = m_environment->is_underground(m_player.curr_floor, m_player.curr_corridor);
        obj->curr_room = nullptr;
        obj->curr_corridor = m_player.curr_corridor;
      }
    }
    
    template<typename Elem,
             typename LambdaItemType,
             typename LambdaID,
             typename ItemType = std::remove_pointer_t<decltype(utils::get_raw_ptr(std::declval<Elem&>()))>>
    void find_and_drop_item(bool& to_drop_found,
                            std::string& msg,
                            const std::string& group_title,
                            int subgroup_idx,
                            LambdaItemType&& pred_item_type,
                            const RC& curr_pos,
                            bool dropped_over_liquid,
                            LambdaID&& pred_get_id,
                            const std::vector<Elem>& all_type_items,
                            std::vector<int>& held_type_item_idcs)
    {
      if (to_drop_found)
        return;
      auto* inv_group = m_inventory->fetch_group(group_title);
      auto* inv_subgroup = inv_group->fetch_subgroup(subgroup_idx);
      auto* hilited_inv_item = inv_subgroup->get_hilited_item();
      if (hilited_inv_item != nullptr && hilited_inv_item->item != nullptr)
      {
        auto* item = dynamic_cast<ItemType*>(hilited_inv_item->item);
        if (item != nullptr)
        {
          auto idx = stlutils::find_if_idx(all_type_items,
                                           [item](const auto& o) { return utils::get_raw_ptr(o) == item; });
          msg += pred_item_type(item) + ":" + std::to_string(pred_get_id(item, idx)) + "!";
          drop_item(item, curr_pos);
          stlutils::erase(held_type_item_idcs, idx);
          inv_subgroup->remove_item(item);
          if (dropped_over_liquid)
            item->exists = false;
          to_drop_found = true;
        }
      }
    }

    
  public:
    Keyboard(Environment* environment, Inventory* inventory, MessageHandler* msg_handler,
             PC& pc,
             std::vector<Key>& all_keys,
             std::vector<Lamp>& all_lamps,
             std::vector<std::unique_ptr<Weapon>>& all_weapons,
             std::vector<Potion>& all_potions,
             std::vector<std::unique_ptr<Armour>>& all_armour,
             std::vector<NPC>& all_npcs,
             bool& trigger_game_save,
             bool& trigger_game_load,
             bool& trigger_screenshot,
             t8x::ui::TextBoxDebug& tbd, bool& debug)
      : m_environment(environment)
      , m_inventory(inventory)
      , message_handler(msg_handler)
      , m_player(pc)
      , m_all_keys(all_keys)
      , m_all_lamps(all_lamps)
      , m_all_weapons(all_weapons)
      , m_all_potions(all_potions)
      , m_all_armour(all_armour)
      , m_all_npcs(all_npcs)
      , m_trigger_game_save(trigger_game_save)
      , m_trigger_game_load(trigger_game_load)
      , m_trigger_screenshot(trigger_screenshot)
      , m_tbd(tbd)
      , m_debug(debug)
    {}
  
    void handle_keyboard(const t8::input::KeyPressDataPair& kpdp, double real_time_s)
    {
      auto curr_key = t8::input::get_char_key(kpdp.transient);
      auto curr_special_key = t8::input::get_special_key(kpdp.transient);
      //auto curr_key_held = t8::input::get_char_key(kpdp.held);
      //auto curr_special_key_held = t8::input::get_special_key(kpdp.held);
    
      auto& curr_pos = m_player.pos;
      
      auto is_inside_curr_bb = [&](int r, int c) -> bool
      {
        if (m_player.curr_corridor != nullptr && m_player.curr_corridor->is_inside_corridor({r, c}))
          return true;
        if (m_player.curr_room != nullptr && m_player.curr_room->is_inside_room({r, c}))
          return true;
        return false;
      };
      
      if (str::to_lower(curr_key) == 'a' || curr_special_key == t8::input::SpecialKey::Left)
      {
        if (m_player.show_inventory)
        {
        }
        else if (is_inside_curr_bb(curr_pos.r, curr_pos.c - 1) &&
                 m_player.allow_move() &&
                 m_environment->allow_move_to(m_player.curr_floor, curr_pos.r, curr_pos.c - 1))
          curr_pos.c--;
      }
      else if (str::to_lower(curr_key) == 'd' || curr_special_key == t8::input::SpecialKey::Right)
      {
        if (m_player.show_inventory && str::to_lower(curr_key) == 'd')
        {
          m_inventory->cache_hilited_index();
          
          std::string msg = "You dropped an item: ";
          bool dropped_over_liquid = !is_dry(m_player.on_terrain);
          bool to_drop_found = false;
          
          find_and_drop_item(to_drop_found, msg, "Keys:", 0,
                             [](Key* /*key*/) -> std::string { return "key"; },
                             curr_pos, dropped_over_liquid,
                             [](Key* key, int /*idx*/) -> int { return key->key_id; },
                             m_all_keys, m_player.key_idcs);
          find_and_drop_item(to_drop_found, msg, "Lamps:", 0,
                             [](Lamp* /*lamp*/) -> std::string { return "lamp"; },
                             curr_pos, dropped_over_liquid,
                             [](Lamp* /*lamp*/, int idx) -> int { return idx; },
                             m_all_lamps, m_player.lamp_idcs);
          find_and_drop_item(to_drop_found, msg, "Weapons:", 0, // Melee
                             [](Weapon* weapon) -> std::string { return weapon->type; },
                             curr_pos, dropped_over_liquid,
                             [](Weapon* /*weapon*/, int idx) -> int { return idx; },
                             m_all_weapons, m_player.weapon_idcs);
          find_and_drop_item(to_drop_found, msg, "Weapons:", 1, // Ranged
                             [](Weapon* weapon) -> std::string { return weapon->type; },
                             curr_pos, dropped_over_liquid,
                             [](Weapon* /*weapon*/, int idx) -> int { return idx; },
                             m_all_weapons, m_player.weapon_idcs);
          find_and_drop_item(to_drop_found, msg, "Potions:", 0,
                             [](Potion* /*potion*/) -> std::string { return "potion"; },
                             curr_pos, dropped_over_liquid,
                             [](Potion* /*potion*/, int idx) -> int { return idx; },
                             m_all_potions, m_player.potion_idcs);
          for (int a_idx = 0; a_idx < ARMOUR_NUM_ITEMS; ++a_idx)
            find_and_drop_item(to_drop_found, msg, "Armour:", a_idx,
                               [](Armour* armour) -> std::string { return armour->type; },
                               curr_pos, dropped_over_liquid,
                               [](Armour* /*armour*/, int idx) -> int { return idx; },
                               m_all_armour, m_player.armour_idcs);

          if (to_drop_found)
          {
            m_inventory->reset_hilite();
            message_handler->add_message(static_cast<float>(real_time_s),
                                         msg,
                                         MessageHandler::Level::Guide);
            if (dropped_over_liquid)
              message_handler->add_message(static_cast<float>(real_time_s),
                                           "Item was dropped over the " + terrain2str(m_player.on_terrain) + " and is now forever lost.",
                                           MessageHandler::Level::Warning);
          }
          else
          {
            msg += "Invalid Item!";
            std::cerr << "ERROR: Attempted to drop invalid item!" << std::endl;
          }
        }
        else if (is_inside_curr_bb(curr_pos.r, curr_pos.c + 1) &&
                 m_player.allow_move() &&
                 m_environment->allow_move_to(m_player.curr_floor, curr_pos.r, curr_pos.c + 1))
          curr_pos.c++;
      }
      else if (str::to_lower(curr_key) == 's' || curr_special_key == t8::input::SpecialKey::Down)
      {
        if (m_player.show_inventory)
          m_inventory->inc_hilite();
        else if (is_inside_curr_bb(curr_pos.r + 1, curr_pos.c) &&
                 m_player.allow_move() &&
                 m_environment->allow_move_to(m_player.curr_floor, curr_pos.r + 1, curr_pos.c))
          curr_pos.r++;
      }
      else if (str::to_lower(curr_key) == 'w' || curr_special_key == t8::input::SpecialKey::Up)
      {
        if (m_player.show_inventory)
          m_inventory->dec_hilite();
        else if (is_inside_curr_bb(curr_pos.r - 1, curr_pos.c) &&
                 m_player.allow_move() &&
                 m_environment->allow_move_to(m_player.curr_floor, curr_pos.r - 1, curr_pos.c))
          curr_pos.r--;
      }
      else if (curr_key == ' ')
      {
        if (m_player.show_inventory)
        {
          m_inventory->toggle_hilited_selection();
        }
        else
        {
          message_handler->clear_curr_message();
          
          m_tbd.clear();
          
          auto slos_r = math::sgn(m_player.los_r);
          auto slos_c = math::sgn(m_player.los_c);
          //tbd.ref_tmp("slos_r", &slos_r) = slos_r;
          //tbd.ref_tmp("slos_c", &slos_c) = slos_c;
        
          auto f_alter_door_states = [&](Door* door)
          {
            if (door == nullptr)
              return false;
            if (!door->is_door)
              return false;
            auto dr = door->pos.r - curr_pos.r;
            auto dc = door->pos.c - curr_pos.c;
            auto adr = std::abs(dr);
            auto adc = std::abs(dc);
            auto sdr = math::sgn(dr);
            auto sdc = math::sgn(dc);
            //tbd.ref_tmp("adr" + std::to_string(door->key_id), &adr) = adr;
            //tbd.ref_tmp("adc" + std::to_string(door->key_id), &adc) = adc;
            //tbd.ref_tmp("sdr" + std::to_string(door->key_id), &sdr) = sdr;
            //tbd.ref_tmp("sdc" + std::to_string(door->key_id), &sdc) = sdc;
            if (math::length_squared(dr, dc) == 1.f && ((adr == 1 && sdr == slos_r) || (adc == 1 && sdc == slos_c)))
            {
              if (door->is_locked)
              {
                if (m_player.using_key_id(m_inventory, door->key_id))
                {
                  // Currently doesn't support locking the door again.
                  // Not sure if we need that. Maybe do it in the far future...
                  door->is_locked = false;
                  
                  message_handler->add_message(static_cast<float>(real_time_s),
                                               "The door is unlocked!",
                                               MessageHandler::Level::Guide);
                  
                  m_inventory->cache_hilited_index();
                  m_player.remove_key_by_key_id(m_inventory, m_all_keys, door->key_id);
                  m_inventory->reset_hilite();
                  message_handler->add_message(static_cast<float>(real_time_s),
                                               "You cast a vanishing spell on the key!",
                                               MessageHandler::Level::Guide);
                }
                else
                {
                  message_handler->add_message(static_cast<float>(real_time_s),
                                               "The door is locked. You need key:" + std::to_string(door->key_id) + "!",
                                               MessageHandler::Level::Guide);
                }
              }
              else
                math::toggle(door->is_open);
              return true;
            }
            return false;
          };
          
          if (m_player.curr_corridor != nullptr && m_player.curr_corridor->is_inside_corridor(curr_pos))
          {
            auto* door_0 = m_player.curr_corridor->doors[0];
            auto* door_1 = m_player.curr_corridor->doors[1];
            
            f_alter_door_states(door_0);
            f_alter_door_states(door_1);
          }
          else if (m_player.curr_room != nullptr && m_player.curr_room->is_inside_room(curr_pos))
          {
            for (auto* door : m_player.curr_room->doors)
              if (f_alter_door_states(door))
                break;
                
            auto* staircase = m_player.curr_room->staircase;
            if (staircase != nullptr && curr_pos == staircase->pos)
            {
              int floor_dir = 0;
              if (m_player.curr_floor == staircase->floor_A)
              {
                m_player.curr_floor = staircase->floor_B;
                m_player.curr_room = staircase->room_floor_B;
                m_player.curr_corridor = nullptr;
                floor_dir = staircase->floor_B - staircase->floor_A;
              }
              else if (m_player.curr_floor == staircase->floor_B)
              {
                m_player.curr_floor = staircase->floor_A;
                m_player.curr_room = staircase->room_floor_A;
                m_player.curr_corridor = nullptr;
                floor_dir = staircase->floor_A - staircase->floor_B;
              }
              if (floor_dir != 0)
              {
                message_handler->add_message(static_cast<float>(real_time_s),
                                             "You walked "s + (floor_dir == -1 ? "up" : "down") + " one floor",
                                             MessageHandler::Level::Guide);
                if (m_player.curr_floor == 0)
                {
                  message_handler->add_message(static_cast<float>(real_time_s),
                                               "You are now at surface level!",
                                               MessageHandler::Level::Guide);
                }
              }
            }
          }
          
          std::string too_heavy_msg_template = " is too heavy to carry.\nYou need to drop items from your inventory!";
          
          for (size_t key_idx = 0; key_idx < m_all_keys.size(); ++key_idx)
          {
            auto& key = m_all_keys[key_idx];
            if (key.exists && key.pos == curr_pos && !key.picked_up)
            {
              if (m_player.has_weight_capacity(key.weight))
              {
                m_player.key_idcs.emplace_back(static_cast<int>(key_idx));
                key.picked_up = true;
                message_handler->add_message(static_cast<float>(real_time_s),
                                             "You picked up a key!",
                                             MessageHandler::Level::Guide);
              }
              else
                message_handler->add_message(static_cast<float>(real_time_s),
                                             "Key" + too_heavy_msg_template,
                                             MessageHandler::Level::Warning);
            }
          }
          for (size_t lamp_idx = 0; lamp_idx < m_all_lamps.size(); ++lamp_idx)
          {
            auto& lamp = m_all_lamps[lamp_idx];
            if (lamp.exists && lamp.pos == curr_pos && !lamp.picked_up)
            {
              auto lamp_type = lamp.get_type_str();
              if (m_player.has_weight_capacity(lamp.weight))
              {
                m_player.lamp_idcs.emplace_back(static_cast<int>(lamp_idx));
                lamp.picked_up = true;
                message_handler->add_message(static_cast<float>(real_time_s),
                                             "You picked up " + str::indef_art(lamp_type) + "!",
                                             MessageHandler::Level::Guide);
              }
              else
                message_handler->add_message(static_cast<float>(real_time_s),
                                             str::anfangify(lamp_type) + too_heavy_msg_template,
                                             MessageHandler::Level::Warning);
            }
          }
          for (size_t wpn_idx = 0; wpn_idx < m_all_weapons.size(); ++wpn_idx)
          {
            auto& weapon = m_all_weapons[wpn_idx];
            if (weapon->exists && weapon->pos == curr_pos && !weapon->picked_up)
            {
              if (m_player.has_weight_capacity(weapon->weight))
              {
                m_player.weapon_idcs.emplace_back(static_cast<int>(wpn_idx));
                weapon->picked_up = true;
                message_handler->add_message(static_cast<float>(real_time_s),
                                             "You picked up " + str::indef_art(weapon->type) + "!",
                                             MessageHandler::Level::Guide);
              }
              else
                message_handler->add_message(static_cast<float>(real_time_s),
                                             str::anfangify(weapon->type) + too_heavy_msg_template,
                                             MessageHandler::Level::Warning);
            }
          }
          for (size_t pot_idx = 0; pot_idx < m_all_potions.size(); ++pot_idx)
          {
            auto& potion = m_all_potions[pot_idx];
            if (potion.exists && potion.pos == curr_pos && !potion.picked_up)
            {
              if (m_player.has_weight_capacity(potion.weight))
              {
                m_player.potion_idcs.emplace_back(static_cast<int>(pot_idx));
                potion.picked_up = true;
                message_handler->add_message(static_cast<float>(real_time_s),
                                             "You picked up a potion!", MessageHandler::Level::Guide);
              }
              else
                message_handler->add_message(static_cast<float>(real_time_s),
                                             "Potion" + too_heavy_msg_template,
                                             MessageHandler::Level::Warning);
            }
          }
          for (size_t a_idx = 0; a_idx < m_all_armour.size(); ++a_idx)
          {
            auto& armour = m_all_armour[a_idx];
            if (armour->exists && armour->pos == curr_pos && !armour->picked_up)
            {
              if (m_player.has_weight_capacity(armour->weight))
              {
                m_player.armour_idcs.emplace_back(static_cast<int>(a_idx));
                armour->picked_up = true;
                message_handler->add_message(static_cast<float>(real_time_s),
                                             "You picked up " + str::indef_art(armour->type) + "!",
                                             MessageHandler::Level::Guide);
              }
              else
                message_handler->add_message(static_cast<float>(real_time_s),
                                             str::anfangify(armour->type) + too_heavy_msg_template,
                                             MessageHandler::Level::Warning);
            }
          }
        }
      }
      else if (curr_key == '-')
      {
        math::toggle(m_player.show_inventory);
      }
      else if (curr_key == '+')
      {
        for (auto& npc : m_all_npcs)
          math::toggle(npc.debug);
      }
      else if (curr_key == '?')
        math::toggle(m_debug);
      else if (str::to_lower(curr_key) == 'i')
      {
        for (const auto& key : m_all_keys)
        {
          if (key.visible_near)
            message_handler->add_message(static_cast<float>(real_time_s),
                                         "You see a key nearby!", MessageHandler::Level::Guide);
        }
        for (const auto& lamp : m_all_lamps)
        {
          if (lamp.visible_near)
          {
            auto lamp_type = lamp.get_type_str();
            message_handler->add_message(static_cast<float>(real_time_s),
                                         "You see " + str::indef_art(lamp_type) + " nearby!", MessageHandler::Level::Guide);
          }
        }
        for (const auto& weapon : m_all_weapons)
        {
          if (weapon->visible_near)
            message_handler->add_message(static_cast<float>(real_time_s),
                                         "You can see " + str::indef_art(weapon->type) + " nearby!", MessageHandler::Level::Guide);
        }
        for (const auto& potion : m_all_potions)
        {
          if (potion.visible_near)
            message_handler->add_message(static_cast<float>(real_time_s),
                                         "You can see a potion nearby!", MessageHandler::Level::Guide);
        }
        for (const auto& armour : m_all_armour)
        {
          if (armour->visible_near)
            message_handler->add_message(static_cast<float>(real_time_s),
                                         "You can see " + str::indef_art(armour->type) + " nearby!", MessageHandler::Level::Guide);
        }
        for (const auto& npc : m_all_npcs)
        {
          if (npc.visible_near)
          {
            auto race = race2str(npc.npc_race);
            if (npc.health <= 0)
              race = "dead " + race;
            message_handler->add_message(static_cast<float>(real_time_s),
                                         "You can see " + str::indef_art(race) + " nearby!", MessageHandler::Level::Guide);
          }
        }
      }
      else if (str::to_lower(curr_key) == 'c')
      {
        auto* potion = m_player.get_selected_potion(m_inventory);
        if (potion != nullptr)
        {
          auto hp = potion->get_hp();
          if (m_player.health + hp > globals::max_health)
            hp = globals::max_health - m_player.health;
          m_player.health += hp;
          
          switch (math::sgn(hp))
          {
            case -1:
              message_handler->add_message(static_cast<float>(real_time_s),
                                           "You drank poison! Your health decreased by " + std::to_string(-hp) + " hp.",
                                           MessageHandler::Level::Warning);
              break;
            case 0:
              message_handler->add_message(static_cast<float>(real_time_s),
                                           "You drank a potion, but nothing appeared to happen.",
                                           MessageHandler::Level::Guide);
            case +1:
              message_handler->add_message(static_cast<float>(real_time_s),
                                           "You drank a health potion. Your health increased by " + std::to_string(hp) + " hp.",
                                           MessageHandler::Level::Guide);
              if (m_player.health == globals::max_health)
                message_handler->add_message(static_cast<float>(real_time_s),
                                             "Your health is now fully restored.",
                                             MessageHandler::Level::Guide);
              break;
          }
          m_inventory->cache_hilited_index();
          m_player.remove_selected_potion(m_inventory, m_all_potions);
          m_inventory->reset_hilite();
          message_handler->add_message(static_cast<float>(real_time_s),
                                       "You throw away the empty vial.",
                                       MessageHandler::Level::Guide);
        }
      }
      else if (str::to_lower(curr_key) == 'f')
      {
        for (auto& npc : m_all_npcs)
          npc.trigger_hostility(m_player.pos);
      }
      else if (curr_key == 'g')
      {
        m_trigger_game_save = true;
      }
      else if (curr_key == 'G')
      {
        m_trigger_game_load = true;
      }
      else if (curr_key == 'z')
      {
        m_trigger_screenshot = true;
      }
    }

  };

}
