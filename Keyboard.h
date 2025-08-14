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

using namespace std::string_literals;


namespace dung
{

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
    
    ui::TextBoxDebug& m_tbd;
    bool& m_debug;
    
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
             ui::TextBoxDebug& tbd, bool& debug)
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
  
    void handle_keyboard(const keyboard::KeyPressDataPair& kpdp, double real_time_s)
    {
      auto curr_key = keyboard::get_char_key(kpdp.transient);
      auto curr_special_key = keyboard::get_special_key(kpdp.transient);
      //auto curr_key_held = keyboard::get_char_key(kpdp.held);
      //auto curr_special_key_held = keyboard::get_special_key(kpdp.held);
    
      auto& curr_pos = m_player.pos;
      
      auto is_inside_curr_bb = [&](int r, int c) -> bool
      {
        if (m_player.curr_corridor != nullptr && m_player.curr_corridor->is_inside_corridor({r, c}))
          return true;
        if (m_player.curr_room != nullptr && m_player.curr_room->is_inside_room({r, c}))
          return true;
        return false;
      };
      
      if (str::to_lower(curr_key) == 'a' || curr_special_key == keyboard::SpecialKey::Left)
      {
        if (m_player.show_inventory)
        {
        }
        else if (is_inside_curr_bb(curr_pos.r, curr_pos.c - 1) &&
                 m_player.allow_move() &&
                 m_environment->allow_move_to(m_player.curr_floor, curr_pos.r, curr_pos.c - 1))
          curr_pos.c--;
      }
      else if (str::to_lower(curr_key) == 'd' || curr_special_key == keyboard::SpecialKey::Right)
      {
        if (m_player.show_inventory && str::to_lower(curr_key) == 'd')
        {
          auto f_drop_item = [&](auto* obj)
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
          };
          
          m_inventory->cache_hilited_index();
          
          std::string msg = "You dropped an item: ";
          bool to_drop_found = false;
          bool dropped_over_liquid = !is_dry(m_player.on_terrain);
          
          {
            auto* keys_group = m_inventory->fetch_group("Keys:");
            auto* keys_subgroup = keys_group->fetch_subgroup(0);
            auto* hilited_inv_item = keys_subgroup->get_hilited_item();
            if (hilited_inv_item != nullptr && hilited_inv_item->item != nullptr)
            {
              auto* key = dynamic_cast<Key*>(hilited_inv_item->item);
              if (key != nullptr)
              {
                auto idx = stlutils::find_if_idx(m_all_keys, [key](const auto& o) { return &o == key; });
                msg += "key:" + std::to_string(key->key_id) + "!";
                f_drop_item(key);
                stlutils::erase(m_player.key_idcs, idx);
                keys_subgroup->remove_item(key);
                to_drop_found = true;
                if (dropped_over_liquid)
                  key->exists = false;
              }
            }
          }
          if (!to_drop_found)
          {
            auto* lamps_group = m_inventory->fetch_group("Lamps:");
            auto* lamps_subgroup = lamps_group->fetch_subgroup(0);
            auto* hilited_inv_item = lamps_subgroup->get_hilited_item();
            if (hilited_inv_item != nullptr && hilited_inv_item->item != nullptr)
            {
              auto* lamp = dynamic_cast<Lamp*>(hilited_inv_item->item);
              if (lamp != nullptr)
              {
                auto idx = stlutils::find_if_idx(m_all_lamps, [lamp](const auto& o) { return &o == lamp; });
                msg += "lamp:" + std::to_string(idx) + "!";
                f_drop_item(lamp);
                stlutils::erase(m_player.lamp_idcs, idx);
                lamps_subgroup->remove_item(lamp);
                to_drop_found = true;
                if (dropped_over_liquid)
                  lamp->exists = false;
              }
            }
          }
          if (!to_drop_found)
          {
            auto* weapons_group = m_inventory->fetch_group("Weapons:");
            auto* weapons_subgroup_melee = weapons_group->fetch_subgroup(0);
            auto* hilited_inv_item = weapons_subgroup_melee->get_hilited_item();
            if (hilited_inv_item != nullptr && hilited_inv_item->item != nullptr)
            {
              auto* weapon = dynamic_cast<Weapon*>(hilited_inv_item->item);
              if (weapon != nullptr)
              {
                auto idx = stlutils::find_if_idx(m_all_weapons,
                  [weapon](const auto& o) { return o.get() == weapon; });
                msg += weapon->type + ":" + std::to_string(idx) + "!";
                f_drop_item(weapon);
                stlutils::erase(m_player.weapon_idcs, idx);
                weapons_subgroup_melee->remove_item(weapon);
                to_drop_found = true;
                if (dropped_over_liquid)
                  weapon->exists = false;
              }
            }
          }
          if (!to_drop_found)
          {
            auto* potions_group = m_inventory->fetch_group("Potions:");
            auto* potions_subgroup = potions_group->fetch_subgroup(0);
            auto* hilited_inv_item = potions_subgroup->get_hilited_item();
            if (hilited_inv_item != nullptr && hilited_inv_item->item != nullptr)
            {
              auto* potion = dynamic_cast<Potion*>(hilited_inv_item->item);
              if (potion != nullptr)
              {
                auto idx = stlutils::find_if_idx(m_all_potions,
                  [potion](const auto& o) { return &o == potion; });
                msg += "potion:" + std::to_string(idx) + "!";
                f_drop_item(potion);
                stlutils::erase(m_player.potion_idcs, idx);
                potions_subgroup->remove_item(potion);
                to_drop_found = true;
                if (dropped_over_liquid)
                  potion->exists = false;
              }
            }
          }
          if (!to_drop_found)
          {
            auto f_try_drop_armour = [&msg, &f_drop_item, &to_drop_found, dropped_over_liquid]
                                    (auto* subgroup, auto& all_armour, auto& pc_armour_idcs)
            {
              if (to_drop_found)
                return false;
              auto* hilited_inv_item = subgroup->get_hilited_item();
              if (hilited_inv_item != nullptr && hilited_inv_item->item != nullptr)
              {
                auto* armour = dynamic_cast<Armour*>(hilited_inv_item->item);
                if (armour != nullptr)
                {
                  auto idx = stlutils::find_if_idx(all_armour, [armour](const auto& o) { return o.get() == armour; });
                  msg += armour->type + ":" + std::to_string(idx) + "!";
                  f_drop_item(armour);
                  stlutils::erase(pc_armour_idcs, idx);
                  subgroup->remove_item(armour);
                  to_drop_found = true;
                  if (dropped_over_liquid)
                    armour->exists = false;
                  return true;
                }
              }
              return false;
            };
            
            auto* armour_group = m_inventory->fetch_group("Armour:");
            if (!f_try_drop_armour(armour_group->fetch_subgroup(ARMOUR_Shield),
                                   m_all_armour, m_player.armour_idcs))
              if (!f_try_drop_armour(armour_group->fetch_subgroup(ARMOUR_Gambeson),
                                     m_all_armour, m_player.armour_idcs))
                if (!f_try_drop_armour(armour_group->fetch_subgroup(ARMOUR_ChainMailleHauberk),
                                       m_all_armour, m_player.armour_idcs))
                  if (!f_try_drop_armour(armour_group->fetch_subgroup(ARMOUR_PlatedBodyArmour),
                                         m_all_armour, m_player.armour_idcs))
                    if (!f_try_drop_armour(armour_group->fetch_subgroup(ARMOUR_PaddedCoif),
                                           m_all_armour, m_player.armour_idcs))
                      if (!f_try_drop_armour(armour_group->fetch_subgroup(ARMOUR_ChainMailleCoif),
                                             m_all_armour, m_player.armour_idcs))
                        f_try_drop_armour(armour_group->fetch_subgroup(ARMOUR_Helmet),
                                          m_all_armour, m_player.armour_idcs);
          }
          if (!to_drop_found)
          {
            msg += "Invalid Item!";
            std::cerr << "ERROR: Attempted to drop invalid item!" << std::endl;
          }
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
        }
        else if (is_inside_curr_bb(curr_pos.r, curr_pos.c + 1) &&
                 m_player.allow_move() &&
                 m_environment->allow_move_to(m_player.curr_floor, curr_pos.r, curr_pos.c + 1))
          curr_pos.c++;
      }
      else if (str::to_lower(curr_key) == 's' || curr_special_key == keyboard::SpecialKey::Down)
      {
        if (m_player.show_inventory)
          m_inventory->inc_hilite();
        else if (is_inside_curr_bb(curr_pos.r + 1, curr_pos.c) &&
                 m_player.allow_move() &&
                 m_environment->allow_move_to(m_player.curr_floor, curr_pos.r + 1, curr_pos.c))
          curr_pos.r++;
      }
      else if (str::to_lower(curr_key) == 'w' || curr_special_key == keyboard::SpecialKey::Up)
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
                  
                  m_player.remove_key_by_key_id(m_inventory, m_all_keys, door->key_id);
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
                                             "You picked up a key!", MessageHandler::Level::Guide);
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
                                             "You picked up " + str::indef_art(weapon->type) + "!", MessageHandler::Level::Guide);
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
          m_player.remove_selected_potion(m_inventory, m_all_potions);
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
