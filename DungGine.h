//
//  DungGine.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-06-13.
//

#pragma once
#include "BSPTree.h"
#include "Dungeon.h"
#include "Environment.h"
#include "ScreenHelper.h"
#include "DungGineStyles.h"
#include "RoomStyle.h"
#include "Items.h"
#include "PC.h"
#include "NPC.h"
#include "SolarMotionPatterns.h"
#include "Globals.h"
#include "DungGineListener.h"
#include "Inventory.h"
#include "Keyboard.h"
#include "SaveGame.h"
#include <Termin8or/Keyboard.h>
#include <Termin8or/MessageHandler.h>
#include <Core/FolderHelper.h>
#include <Core/events/EventBroadcaster.h>
#include <Core/Utils.h>
#include <Core/System.h>
#include <Core/Timer.h>

using namespace utils::literals;

namespace dung
{

  using namespace std::string_literals;

  class DungGine final : public EventBroadcaster<DungGineListener>
  {
    std::unique_ptr<Environment> m_environment;
    
    bool m_use_per_room_lat_long_for_sun_dir = true;
    SolarDirection m_sun_dir = SolarDirection::E;
    Latitude m_latitude = Latitude::NorthernHemisphere;
    Longitude m_longitude = Longitude::F;
    Season m_season = Season::Spring;
    SolarMotionPatterns m_solar_motion;
    float m_sun_minutes_per_day = 20.f;
    float m_sun_day_t_offs = 0.f;
    float m_sun_minutes_per_year = 120.f;
    float m_sun_year_t_offs = 0.f;
    float m_t_solar_period = 0.f;
    
    bool debug = false;
    
    PC m_player;
    std::vector<NPC> all_npcs;
    
    std::unique_ptr<ScreenHelper> m_screen_helper;
    
    std::unique_ptr<Inventory> m_inventory;
    OneShot sort_inventory { false };
    
    std::unique_ptr<Keyboard> m_keyboard;
    
    std::vector<Key> all_keys;
    // Lamps illuminate items and NPCs. If you've already discovered an item or
    //   NPC using a lamp (and after FOW been cleared),
    //   then they will still be visible when the room is not lit.
    // Lamps will not work in surface level rooms.
    std::vector<Lamp> all_lamps;
    std::vector<std::unique_ptr<Weapon>> all_weapons;
    std::vector<Potion> all_potions;
    std::vector<std::unique_ptr<Armour>> all_armour;
    
    std::unique_ptr<MessageHandler> message_handler;
    bool use_fog_of_war = false;
    
    const std::vector<std::string> c_fight_strings { "(", "#", ")", "%", "*" };
    const std::vector<Color> c_fight_colors { Color::Red, Color::Yellow, Color::Blue, Color::Magenta, Color::White, Color::Black, Color::LightGray, Color::DarkGray };
    enum class FightDir { NW, W, SW, S, SE, E, NE, N, NUM_ITEMS };
    const std::vector<int> fight_r_offs = { 1, 0, -1, -1, -1, 0, 1, 1 };
    const std::vector<int> fight_c_offs = { 1, 1, 1, 0, -1, -1, -1, 0 };
    
    struct Projectile
    {
      Vec2 pos;
      Vec2 dir { 0.f, 0.f };
      float speed = 0.f;
      int ang_idx = 0;
      Timer travel_time { 3.f };
      PlayerBase* shooter = nullptr;
      const Weapon* weapon = nullptr;       // for damage calculation and projectile rendering.
      int curr_floor = 0;
      BSPNode* curr_room = nullptr;
      Corridor* curr_corridor = nullptr;
      bool hit = false;
    };
    std::vector<Projectile> active_projectiles;
    
    ui::TextBox tb_health, tb_strength;
    ui::TextBoxDebug tbd;
    
    bool stall_game = false;
    
    bool trigger_game_save = false;
    bool trigger_game_load = false;
    bool trigger_screenshot = false;
    bool use_save_game_git_hash_check = false;
    std::string path_to_dunggine_repo; // Path to where the DungGine repo is checked out.
    
    // /////////////////////
    
    void update_sun(float real_time_s)
    {
      m_t_solar_period = std::fmod(m_sun_day_t_offs + (real_time_s / 60.f) / m_sun_minutes_per_day, 1.f);
      m_sun_dir = m_solar_motion.get_solar_direction(m_latitude, m_longitude, m_season, m_t_solar_period);
      
      float t_season_period = std::fmod(m_sun_year_t_offs + (real_time_s / 60.f) / m_sun_minutes_per_year, 1.f);
      m_season = static_cast<Season>(math::roundI(7*t_season_period));
    }
    
    void update_inventory()
    {
      auto num_inv_keys = stlutils::sizeI(m_player.key_idcs);
      auto num_inv_lamps = stlutils::sizeI(m_player.lamp_idcs);
      auto num_inv_wpns = stlutils::sizeI(m_player.weapon_idcs);
      auto num_inv_potions = stlutils::sizeI(m_player.potion_idcs);
      auto num_inv_armour = stlutils::sizeI(m_player.armour_idcs);
      
      m_player.curr_tot_inv_weight = 0.f;
      
      auto f_format_item_str = [](std::string& item_str, float weight, float price, int hp)
      {
        std::ostringstream oss;
        oss << std::setprecision(1) << std::fixed << weight;
        std::string weight_str = oss.str() + " kg";
        item_str += str::rep_char(' ', 25 - stlutils::sizeI(item_str) - stlutils::sizeI(weight_str)) + weight_str;
        
        oss.str("");
        oss.clear();
        oss << std::setprecision(2) << std::fixed << price;
        std::string price_str = oss.str() + " FK"; // Fantasy-Kronor.
        item_str += str::rep_char(' ', 42 - stlutils::sizeI(item_str) - stlutils::sizeI(price_str)) + price_str;
        
        if (hp > 0)
        {
          oss.str("");
          oss.clear();
          oss << std::setprecision(0) << std::fixed << hp;
          std::string hp_str = oss.str() + " hp";
          item_str += str::rep_char(' ', 52 - stlutils::sizeI(item_str) - stlutils::sizeI(hp_str)) + hp_str;
        }
      };
      
      auto* keys_group = m_inventory->fetch_group("Keys:");
      auto* keys_subgroup = keys_group->fetch_subgroup(0);
      for (int inv_key_idx = 0; inv_key_idx < num_inv_keys; ++inv_key_idx)
      {
        auto key_idx = m_player.key_idcs[inv_key_idx];
        auto& key = all_keys[key_idx];
        std::string item_str = "  Key:" + std::to_string(key.key_id);
        f_format_item_str(item_str, key.weight, key.price, 0);
        if (keys_subgroup->find_item(&key) == nullptr)
          keys_subgroup->add_item(item_str, &key, key_idx);
        m_player.curr_tot_inv_weight += key.weight;
      }
      
      auto* lamps_group = m_inventory->fetch_group("Lamps:");
      auto* lamps_subgroup = lamps_group->fetch_subgroup(0);
      for (int inv_lamp_idx = 0; inv_lamp_idx < num_inv_lamps; ++inv_lamp_idx)
      {
        auto lamp_idx = m_player.lamp_idcs[inv_lamp_idx];
        auto& lamp = all_lamps[lamp_idx];
        auto lamp_type = lamp.get_type_str();
        str::to_upper(lamp_type[0]);
        std::string item_str = "  "s + lamp_type + ":" + std::to_string(lamp_idx);
        f_format_item_str(item_str, lamp.weight, lamp.price, 0);
        if (lamps_subgroup->find_item(&lamp) == nullptr)
          lamps_subgroup->add_item(item_str, &lamp, lamp_idx);
        m_player.curr_tot_inv_weight += lamp.weight;
      }
      
      auto* weapons_group = m_inventory->fetch_group("Weapons:");
      auto* weapons_subgroup_melee = weapons_group->fetch_subgroup(0);
      weapons_subgroup_melee->set_title("Melee:");
      auto* weapons_subgroup_ranged = weapons_group->fetch_subgroup(1);
      weapons_subgroup_ranged->set_title("Ranged:");
      for (int inv_wpn_idx = 0; inv_wpn_idx < num_inv_wpns; ++inv_wpn_idx)
      {
        auto wpn_idx = m_player.weapon_idcs[inv_wpn_idx];
        auto& weapon = all_weapons[wpn_idx];
        std::string item_str = "  ";
        bool melee = dynamic_cast<Sword*>(weapon.get()) != nullptr
          || dynamic_cast<Dagger*>(weapon.get()) != nullptr
          || dynamic_cast<Flail*>(weapon.get()) != nullptr
          || dynamic_cast<MorningStar*>(weapon.get()) != nullptr;
        item_str += str::anfangify(weapon->type);
        item_str += ":";
        item_str += std::to_string(wpn_idx);
        f_format_item_str(item_str, weapon->weight, weapon->price, weapon->damage);
        if (melee && weapons_subgroup_melee->find_item(weapon.get()) == nullptr)
          weapons_subgroup_melee->add_item(item_str, weapon.get(), wpn_idx);
        else if (!melee && weapons_subgroup_ranged->find_item(weapon.get()) == nullptr)
          weapons_subgroup_ranged->add_item(item_str, weapon.get(), wpn_idx);
        m_player.curr_tot_inv_weight += weapon->weight;
      }
      
      auto* potions_group = m_inventory->fetch_group("Potions:");
      auto* potions_subgroup = potions_group->fetch_subgroup(0);
      for (int inv_pot_idx = 0; inv_pot_idx < num_inv_potions; ++inv_pot_idx)
      {
        auto pot_idx = m_player.potion_idcs[inv_pot_idx];
        auto& potion = all_potions[pot_idx];
        std::string item_str = "  Potion:" + std::to_string(pot_idx);
        f_format_item_str(item_str, potion.weight, potion.price, 0);
        if (potions_subgroup->find_item(&potion) == nullptr)
          potions_subgroup->add_item(item_str, &potion, pot_idx);
        m_player.curr_tot_inv_weight += potion.weight;
      }
      
      auto* armour_group = m_inventory->fetch_group("Armour:");
      auto* armour_subgroup_shields = armour_group->fetch_subgroup(ARMOUR_Shield);
      armour_subgroup_shields->set_title("Shields:");
      auto* armour_subgroup_gambesons = armour_group->fetch_subgroup(ARMOUR_Gambeson);
      armour_subgroup_gambesons->set_title("Gambesons:");
      auto* armour_subgroup_chainmaillehauberks = armour_group->fetch_subgroup(ARMOUR_ChainMailleHauberk);
      armour_subgroup_chainmaillehauberks->set_title("Chain-Maille Hauberks:");
      auto* armour_subgroup_platedbodyarmour = armour_group->fetch_subgroup(ARMOUR_PlatedBodyArmour);
      armour_subgroup_platedbodyarmour->set_title("Plated Body Armour:");
      auto* armour_subgroup_paddedcoifs = armour_group->fetch_subgroup(ARMOUR_PaddedCoif);
      armour_subgroup_paddedcoifs->set_title("Padded Coifs:");
      auto* armour_subgroup_chainmaillecoifs = armour_group->fetch_subgroup(ARMOUR_ChainMailleCoif);
      armour_subgroup_chainmaillecoifs->set_title("Chain-Maille Coifs:");
      auto* armour_subgroup_helmets = armour_group->fetch_subgroup(ARMOUR_Helmet);
      armour_subgroup_helmets->set_title("Helmets:");
      for (int inv_a_idx = 0; inv_a_idx < num_inv_armour; ++inv_a_idx)
      {
        auto a_idx = m_player.armour_idcs[inv_a_idx];
        const auto& armour = all_armour[a_idx];
        std::string item_str = "  ";
        InvSubGroup* armour_subgroup = nullptr;
        if (dynamic_cast<Shield*>(armour.get()) != nullptr)
        {
          item_str += "Shield";
          armour_subgroup = armour_subgroup_shields;
        }
        else if (dynamic_cast<Gambeson*>(armour.get()) != nullptr)
        {
          item_str += "Gambeson";
          armour_subgroup = armour_subgroup_gambesons;
        }
        else if (dynamic_cast<ChainMailleHauberk*>(armour.get()) != nullptr)
        {
          item_str += "C.M.H.";
          armour_subgroup = armour_subgroup_chainmaillehauberks;
        }
        else if (dynamic_cast<PlatedBodyArmour*>(armour.get()) != nullptr)
        {
          item_str += "P.B.A.";
          armour_subgroup = armour_subgroup_platedbodyarmour;
        }
        else if (dynamic_cast<PaddedCoif*>(armour.get()) != nullptr)
        {
          item_str += "P. Coif";
          armour_subgroup = armour_subgroup_paddedcoifs;
        }
        else if (dynamic_cast<ChainMailleCoif*>(armour.get()) != nullptr)
        {
          item_str += "C.M. Coif";
          armour_subgroup = armour_subgroup_chainmaillecoifs;
        }
        else if (dynamic_cast<Helmet*>(armour.get()) != nullptr)
        {
          item_str += "Helmet";
          armour_subgroup = armour_subgroup_helmets;
        }
        else
          item_str += "<Armour>";
        item_str += ":";
        item_str += std::to_string(a_idx);
        f_format_item_str(item_str, armour->weight, armour->price, armour->protection);
        if (armour_subgroup != nullptr)
          if (armour_subgroup->find_item(armour.get()) == nullptr)
            armour_subgroup->add_item(item_str, armour.get(), a_idx);
        m_player.curr_tot_inv_weight += armour->weight;
      }
      
      if (sort_inventory.once())
        m_inventory->set_sort_mode(true);
      m_inventory->apply_deserialization_changes();
    }
    
    template<typename Lambda>
    void clear_field(Lambda get_field_ptr, bool clear_val)
    {
      for (auto& key : all_keys)
        *get_field_ptr(&key) = clear_val;
        
      for (auto& lamp : all_lamps)
        *get_field_ptr(&lamp) = clear_val;
      
      for (auto& weapon : all_weapons)
        *get_field_ptr(weapon.get()) = clear_val;
        
      for (auto& potion : all_potions)
        *get_field_ptr(&potion) = clear_val;
      
      for (auto& armour : all_armour)
        *get_field_ptr(armour.get()) = clear_val;
        
      for (auto& bs : m_player.blood_splats)
        *get_field_ptr(&bs) = clear_val;
        
      for (auto& npc : all_npcs)
        for (auto& bs : npc.blood_splats)
          *get_field_ptr(&bs) = clear_val;
        
      // #NOTE: fog_of_war and light vars set by NPC class itself.
      //for (auto& npc : all_npcs)
      //  *get_field_ptr(&npc) = clear_val;
      
      //ttl::Rectangle bb;
      bool_vector* field = nullptr;
      
      if (m_player.curr_corridor != nullptr)
      {
        //bb = m_player.curr_corridor->bb;
        field = get_field_ptr(m_player.curr_corridor);
        
        auto* door_0 = m_player.curr_corridor->doors[0];
        auto* door_1 = m_player.curr_corridor->doors[1];
        //stlutils::memset(*field, clear_val);
        stlutils::fill(*field, clear_val);
        
        *get_field_ptr(door_0) = clear_val;
        *get_field_ptr(door_1) = clear_val;
      }
      if (m_player.curr_room != nullptr)
      {
        //bb = m_player.curr_room->bb_leaf_room;
        field = get_field_ptr(m_player.curr_room);
        //stlutils::memset(*field, clear_val);
        stlutils::fill(*field, clear_val);
        
        for (auto* door : m_player.curr_room->doors)
          *get_field_ptr(door) = clear_val;
          
        auto* staircase = m_player.curr_room->staircase;
        if (staircase != nullptr)
          *get_field_ptr(staircase) = clear_val;
      }

    }
    
    template<typename Lambda>
    void update_field(const RC& curr_pos, Lambda get_field_ptr, bool set_val, float radius, float angle_deg,
                      Lamp::LightType src_type)
    {
      const auto c_fow_dist = radius; //2.3f;
      
      auto f_set_item_field = [&](auto& obj)
      {
        if (m_player.curr_room == obj.curr_room || m_player.curr_corridor == obj.curr_corridor)
          if (distance(obj.pos, curr_pos) <= radius)
          {
            if (src_type == Lamp::LightType::Directional)
            {
              // #FIXME: Move parts into math functions for better reuse.
              // Rotating dir vector CW and CCW using a rotation matrix.
              auto a = math::deg2rad(angle_deg*0.5f);
              auto dir_r = m_player.los_r;
              auto dir_c = m_player.los_c;
              auto Clo = std::cos(-a);
              auto Slo = std::sin(-a);
              auto Chi = std::cos(+a);
              auto Shi = std::sin(+a);
              math::normalize(dir_r, dir_c);
              float dir_lo_r = (dir_r*Clo - dir_c*Slo);
              float dir_lo_c = dir_r*Slo + dir_c*Clo;
              float dir_hi_r = (dir_r*Chi - dir_c*Shi);
              float dir_hi_c = dir_r*Shi + dir_c*Chi;
    
              float lo_angle_rad = math::atan2n(-dir_lo_r, dir_lo_c);
              float hi_angle_rad = math::atan2n(-dir_hi_r, dir_hi_c);
              if (lo_angle_rad > hi_angle_rad)
                hi_angle_rad += math::c_2pi;
                
              float curr_angle_rad = static_cast<float>(math::atan2n<float>(-(obj.pos.r - curr_pos.r), obj.pos.c - curr_pos.c));
              if (curr_angle_rad < lo_angle_rad)
                curr_angle_rad += math::c_2pi;
              if (math::in_range<float>(curr_angle_rad, lo_angle_rad, hi_angle_rad, Range::Closed))
                *get_field_ptr(&obj) = set_val;
            }
            else
              *get_field_ptr(&obj) = set_val;
          }
      };
      
      for (auto& key : all_keys)
        f_set_item_field(key);
          
      for (auto& lamp : all_lamps)
        f_set_item_field(lamp);
          
      for (auto& weapon : all_weapons)
        f_set_item_field(*weapon);
          
      for (auto& potion : all_potions)
        f_set_item_field(potion);
          
      for (auto& armour : all_armour)
        f_set_item_field(*armour);
        
      for (auto& bs : m_player.blood_splats)
        f_set_item_field(bs);
        
      for (auto& npc : all_npcs)
        for (auto& bs : npc.blood_splats)
          f_set_item_field(bs);
      
      // #NOTE: fog_of_war and light vars set by NPC class itself.
      //for (auto& npc : all_npcs)
      //  if (distance(npc.pos, curr_pos) <= c_fow_dist)
      //    *get_field_ptr(&npc) = set_val;
      
      ttl::Rectangle bb;
      RC local_pos;
      RC size;
      bool_vector* field = nullptr;
      
      auto set_field = [&](const RC& p)
      {
        if (field == nullptr)
          return;
        if (p.r < 0 || p.c < 0 || p.r >= bb.r_len || p.c >= bb.c_len)
          return;
        // ex:
        // +---+ 01234
        // |   | 56789
        // +---+ ABCDE
        // r_len = 3, c_len = 5
        // FOW size = 3*5
        // idx = 0 .. 14
        // r = 2, c = 4 => idx = r * c_len + c = 2*5 + 4 = 14.
        int idx = p.r * size.c + p.c;
        if (0 <= idx && idx < static_cast<int>(field->size()))
          (*field)[idx] = set_val;
      };
      
      auto update_rect_field = [&]() // #FIXME: FHXFTW
      {
        local_pos = curr_pos - bb.pos();
        size = bb.size();
        
        std::vector<RC> positions;
        switch (src_type)
        {
          case Lamp::LightType::Isotropic:
            positions = drawing::filled_circle_positions(local_pos, radius, globals::px_aspect);
            break;
          case Lamp::LightType::Directional:
            positions = drawing::filled_arc_positions(local_pos, radius, math::deg2rad(angle_deg), m_player.los_r, m_player.los_c, globals::px_aspect);
            break;
          case Lamp::LightType::NUM_ITEMS:
            break;
        }
        for (const auto& pos : positions)
          set_field(pos);
        
        int r_room = -1;
        int c_room = -1;
        if (curr_pos.r - bb.top() <= 1)
          r_room = 0;
        else if (bb.bottom() - curr_pos.r <= 1)
          r_room = bb.r_len - 1;
        if (curr_pos.c - bb.left() <= 1)
          c_room = 0;
        else if (bb.right() - curr_pos.c <= 1)
          c_room = bb.c_len - 1;
        
        if (r_room >= 0 && c_room >= 0)
          set_field({ r_room, c_room });
      };
      
      if (m_player.curr_corridor != nullptr && m_player.curr_corridor->is_inside_corridor(curr_pos))
      {
        bb = m_player.curr_corridor->bb;
        field = get_field_ptr(m_player.curr_corridor);
        
        auto* door_0 = m_player.curr_corridor->doors[0];
        auto* door_1 = m_player.curr_corridor->doors[1];
        update_rect_field();
        
        if (distance(door_0->pos, curr_pos) <= c_fow_dist)
          *get_field_ptr(door_0) = set_val;
        if (distance(door_1->pos, curr_pos) <= c_fow_dist)
          *get_field_ptr(door_1) = set_val;
      }
      if (m_player.curr_room != nullptr && m_player.curr_room->is_inside_room(curr_pos))
      {
        bb = m_player.curr_room->bb_leaf_room;
        field = get_field_ptr(m_player.curr_room);
        update_rect_field();
        
        for (auto* door : m_player.curr_room->doors)
          if (distance(door->pos, curr_pos) <= c_fow_dist)
            *get_field_ptr(door) = set_val;
            
        auto* staircase = m_player.curr_room->staircase;
        if (staircase != nullptr)
          if (distance(staircase->pos, curr_pos) <= c_fow_dist)
            *get_field_ptr(staircase) = set_val;
      }
    }
    
    template<typename T>
    bool calc_night(const T& obj)
    {
      bool is_night = false;
      if (m_use_per_room_lat_long_for_sun_dir)
      {
        auto f_set_night = [&](const RoomStyle& rs)
        {
          if (m_solar_motion.get_solar_direction(rs.latitude, rs.longitude, m_season, m_t_solar_period) == SolarDirection::Nadir)
            is_night = true;
        };
        
        auto room_style = m_environment->find_room_style(obj.curr_floor, obj.curr_room);
        if (room_style.has_value())
          f_set_night(room_style.value());
        else
        {
          auto corr_style = m_environment->find_corridor_style(obj.curr_floor, obj.curr_corridor);
          if (corr_style.has_value())
            f_set_night(corr_style.value());
        }
      }
      else
        is_night = m_sun_dir == SolarDirection::Nadir;
      
      return is_night;
    };
    
    void set_visibilities(float fow_radius, const RC& pc_pos)
    {
      const auto c_fow_radius_sq = math::sq(fow_radius);
      
      auto f_fow_near = [&pc_pos, c_fow_radius_sq](const auto& obj) -> bool
      {
        return distance_squared(obj.pos, pc_pos) <= c_fow_radius_sq;
      };
            
      for (auto& key : all_keys)
        key.set_visibility(use_fog_of_war, f_fow_near(key), calc_night(key));
      
      for (auto& lamp : all_lamps)
        lamp.set_visibility(use_fog_of_war, f_fow_near(lamp), calc_night(lamp));
      
      for (auto& weapon : all_weapons)
        weapon->set_visibility(use_fog_of_war, f_fow_near(*weapon), calc_night(*weapon));
      
      for (auto& potion : all_potions)
        potion.set_visibility(use_fog_of_war, f_fow_near(potion), calc_night(potion));
        
      for (auto& armour : all_armour)
        armour->set_visibility(use_fog_of_war, f_fow_near(*armour), calc_night(*armour));
        
      for (auto& npc : all_npcs)
        npc.set_visibility(use_fog_of_war, f_fow_near(npc), calc_night(npc));
        
      for (auto& bs : m_player.blood_splats)
        bs.set_visibility(use_fog_of_war, calc_night(bs));
      
      for (auto& npc : all_npcs)
        for (auto& bs : npc.blood_splats)
          bs.set_visibility(use_fog_of_war, calc_night(bs));
    }
    
    Weapon* get_selected_melee_weapon(PlayerBase* player) const
    {
      Weapon* melee_weapon = nullptr;
      if (auto* pc = dynamic_cast<PC*>(player); pc != nullptr)
        melee_weapon = pc->get_selected_weapon(m_inventory.get(), WeaponDistType_Melee);
      else if (auto* npc = dynamic_cast<NPC*>(player); npc != nullptr)
        melee_weapon = npc->melee_weapon_idx == -1 ? nullptr : all_weapons[npc->melee_weapon_idx].get();
      return melee_weapon;
    }
    Weapon* get_selected_ranged_weapon(PlayerBase* player) const
    {
      Weapon* ranged_weapon = nullptr;
      if (auto* pc = dynamic_cast<PC*>(player); pc != nullptr)
        ranged_weapon = pc->get_selected_weapon(m_inventory.get(), WeaponDistType_Ranged);
      else if (auto* npc = dynamic_cast<NPC*>(player); npc != nullptr)
        ranged_weapon = npc->ranged_weapon_idx == -1 ? nullptr : all_weapons[npc->ranged_weapon_idx].get();
      return ranged_weapon;
    }
    
    std::pair<bool, bool> get_pc_attack_modes(const NPC& npc)
    {
      auto dist_pc_npc = npc.distance_to_pc();
      bool pc_melee_attack = npc.wants_to_attack() && (dist_pc_npc < npc.c_dist_fight_melee);
      bool pc_ranged_attack = !pc_melee_attack && npc.wants_to_attack() && (dist_pc_npc < npc.c_dist_fight_ranged) && get_selected_ranged_weapon(&m_player) != nullptr;
      return { pc_melee_attack, pc_ranged_attack };
    }
    
    template<int NR, int NC>
    void draw_health_bars(ScreenHandler<NR, NC>& sh, bool framed_mode)
    {
      std::vector<std::string> health_bars;
      std::vector<Style> styles;
      std::vector<std::pair<RC, Style>> per_textel_styles;
      std::string pc_hb = str::rep_char(' ', 10);
      float pc_ratio = globals::max_health / 10;
      for (int i = 0; i < 10; ++i)
        pc_hb[i] = m_player.health > static_cast<int>(i*pc_ratio) ? '#' : ' ';
      pc_hb = std::string(1, m_player.character) + ' ' + pc_hb;
      per_textel_styles.emplace_back(RC { 0, 0 }, m_player.style);
      health_bars.emplace_back(pc_hb);
      styles.emplace_back(Style { Color::Magenta, Color::Transparent2 });
      
      int line = 1;
      for (const auto& npc : all_npcs)
      {
        auto [pc_melee_attack, pc_ranged_attack] = get_pc_attack_modes(npc);
        if (npc.health > 0 && (npc.state == State::FightMelee || npc.state == State::FightRanged || pc_melee_attack || pc_ranged_attack))
        {
          std::string npc_hb = str::rep_char(' ', 10);
          float npc_ratio = globals::max_health / 10;
          for (int i = 0; i < 10; ++i)
            npc_hb[i] = npc.health > static_cast<int>(i*npc_ratio) ? 'O' : ' ';
          npc_hb = std::string(1, npc.visible ? npc.character : '?') + ' ' + npc_hb;
          per_textel_styles.emplace_back(RC { line++, 0 }, npc.visible ? npc.style : Style { Color::White, Color::Transparent2 });
          health_bars.emplace_back(npc_hb);
          styles.emplace_back(Style { Color::Red, Color::Transparent2 });
        }
      }
      
      ui::TextBoxDrawingArgsAlign tb_args;
      tb_args.v_align = ui::VerticalAlignment::TOP;
      tb_args.h_align = ui::HorizontalAlignment::LEFT;
      tb_args.base.box_style = { Color::White, Color::DarkBlue };
      tb_args.framed_mode = framed_mode;
      tb_health.set_text(health_bars, styles, per_textel_styles);
      tb_health.calc_pre_draw(str::Adjustment::Left);
      tb_health.draw(sh, tb_args);
    }
    
    template<int NR, int NC>
    void draw_strength_bar(ScreenHandler<NR, NC>& sh, bool framed_mode)
    {
      ui::TextBoxDrawingArgsPos tb_args;
      int offs = framed_mode ? 1 : 0;
      tb_args.pos = { 1 + offs, 12 + offs };
      tb_args.base.box_style = { Color::White, Color::DarkBlue };
    
      std::string strength_bar = str::rep_char(' ', 10);
      float pc_ratio = m_player.strength / 10.f;
      for (int i = 0; i < 10; ++i)
        strength_bar[i] = (m_player.strength - m_player.weakness) > static_cast<int>(i*pc_ratio)
        ? '=' : ' ';
      Style style { Color::Green, Color::Transparent2 };
      tb_strength.set_text(strength_bar, style);
      tb_strength.calc_pre_draw(str::Adjustment::Left);
      tb_strength.draw(sh, tb_args);
    }
    
    void fire_projectile(PlayerBase* shooter, PlayerBase* target, const Weapon* ranged_weapon, float sim_time_s, float projectile_speed_factor)
    {
      if (ranged_weapon == nullptr)
        return;
    
      Projectile p;
      p.pos = to_Vec2(shooter->pos);
      auto target_pos = to_Vec2(target->pos);
      p.dir = math::normalize(target_pos - to_Vec2(shooter->pos));
      p.shooter = shooter;
      p.curr_floor = shooter->curr_floor;
      p.curr_room = shooter->curr_room;
      p.curr_corridor = shooter->curr_corridor;
      p.weapon = ranged_weapon;
      p.speed = projectile_speed_factor * ranged_weapon->projectile_speed;
      
      // convert to angle
      float base_angle = std::atan2(-p.dir.r, p.dir.c);
      
      // add Gaussian noise based on weapon accuracy
      float sigma = ranged_weapon->spread_sigma_rad;
      float noisy_angle = rnd::randn(base_angle, sigma);
      
      // convert back to vector
      p.dir = { -std::sin(noisy_angle), std::cos(noisy_angle) };
      
      // set travel time based on distance / projectile speed
      float dist = math::distance(target_pos, p.pos)*4.f*projectile_speed_factor; // Magic factor?
      p.travel_time.set_delay(dist / p.speed);
      p.travel_time.force_start(sim_time_s);
      
      // optional: pick angle index if your rendering uses direction sprites
      p.ang_idx = math::roundI(8.f * math::normalize_angle(noisy_angle) / math::c_2pi) % 8;
      
      active_projectiles.emplace_back(p);
    }
    
    void update_fighting(float real_time_s, float sim_time_s, float sim_dt_s,
                         float projectile_speed_factor,
                         int melee_attack_dice, int ranged_attack_dice)
    {
      auto f_calc_melee_damage = [](const Weapon* weapon, int bonus)
      {
        if (weapon == nullptr)
          return 1 + bonus; // Fists with strength bonus
        return weapon->damage + bonus;
      };
      
      if (m_player.health <= 0)
        return;
      
      const auto* pc_melee_weapon = get_selected_melee_weapon(&m_player);
      const auto* pc_ranged_weapon = get_selected_ranged_weapon(&m_player);
      
      // Calculate the player's total armor class.
      int pc_ac = m_player.calc_armour_class(m_inventory.get());
      
      for (auto& npc : all_npcs)
      {
        if (npc.health <= 0)
          continue;
        
        int npc_ac = npc.calc_armour_class(all_armour);
        
        int blind_attack_penalty = (npc.visible ? 0 : (npc.state == State::FightMelee ? 12 : 15)) + rnd::rand_int(0, 8);
        
        // /////// PC
        
        auto [pc_melee_attack, pc_ranged_attack] = get_pc_attack_modes(npc);
        
        // Fire shots.
        if (pc_ranged_attack)
        {
          if (m_player.attack_timer.start_if_stopped(sim_time_s))
            m_player.attack_timer.set_delay(1.f / pc_ranged_weapon->attack_speed);
          else
          {
            int pc_attack_roll = rnd::dice(ranged_attack_dice)
                                 + m_player.thac0
                                 + m_player.get_ranged_attack_bonus()
                                 - blind_attack_penalty;
            if (pc_attack_roll >= npc_ac && m_player.attack_timer.wait_then_reset(sim_time_s))
              fire_projectile(&m_player, &npc, pc_ranged_weapon, sim_time_s, projectile_speed_factor);
          }
        }
        
        int npc_damage = 0;
        for (auto& p : active_projectiles)
        {
          // Check if projectile reached its target.
          if (!p.hit && p.shooter != &npc && to_RC_round(p.pos) == npc.pos)
          {
            p.hit = true; // mark it as impacted
            npc.ranged_weapon_hit = true;
            npc_damage += p.weapon->damage;
          }
        }
        
        if (pc_melee_attack)
        {
          // Roll a d20 for the player's attack roll (if the NPC is visible).
          // If invisible, then roll a d32 instead.
          int pc_attack_roll = rnd::dice(melee_attack_dice)
                               + m_player.thac0
                               + m_player.get_melee_attack_bonus()
                               - blind_attack_penalty;
          
          // Determine if PC hits the NPC.
          if (pc_attack_roll >= npc_ac)
          {
            // PC hits the NPC.
            npc_damage += f_calc_melee_damage(pc_melee_weapon, m_player.get_melee_damage_bonus());
          }
        }
        
        if (npc_damage > 0)
        {
          // Apply damage to the NPC.
          bool was_alive = npc.health > 0;
          npc.health -= npc_damage;
          if (was_alive && npc.health <= 0)
          {
            message_handler->add_message(real_time_s,
                                         "You killed the " + race2str(npc.npc_race) + "!",
                                         MessageHandler::Level::Guide);
            broadcast([](auto* listener) { listener->on_npc_death(); });
          }
        }
        
        // /////// NPC
        
        const auto* npc_melee_weapon = get_selected_melee_weapon(&npc);
        const auto* npc_ranged_weapon = get_selected_ranged_weapon(&npc);
        
        // Fire shots.
        if (npc.state == State::FightRanged)
        {
          if (npc.attack_timer.start_if_stopped(sim_time_s))
            npc.attack_timer.set_delay(1.f / npc_ranged_weapon->attack_speed);
          else
          {
            int npc_attack_roll = rnd::dice(ranged_attack_dice)
                                  + npc.thac0
                                  + npc.get_ranged_attack_bonus()
                                  - blind_attack_penalty;
            if (npc_attack_roll >= pc_ac && npc.attack_timer.wait_then_reset(sim_time_s))
              fire_projectile(&npc, &m_player, npc_ranged_weapon, sim_time_s, projectile_speed_factor);
          }
        }
        
        int pc_damage = 0;
        for (auto& p : active_projectiles)
        {
          // Check if projectile reached its target.
          if (!p.hit && p.shooter != &m_player && to_RC_round(p.pos) == m_player.pos)
          {
            p.hit = true; // mark it as impacted
            m_player.ranged_weapon_hit = true;
            pc_damage += p.weapon->damage;
          }
        }
        
        if (npc.state == State::FightMelee)
        {
          // #TODO: Fix distance penalty for ranged weapons. The closer you get to the target, the less damage you inflict.
          //auto dist = npc.distance_to_pc();
          //auto* lamp = m_player.get_selected_lamp(m_inventory.get());
          //auto ranged_max_attack_dist = lamp == nullptr ? globals::max_fow_radius : lamp->radius;
          
          int npc_melee_attack_bonus = npc.state == State::FightMelee ? npc.get_melee_attack_bonus() : 0;
          
          // NPC attack roll.
          int npc_attack_roll = rnd::dice(melee_attack_dice)
                                + npc.thac0
                                + npc_melee_attack_bonus
                                + (rnd::dice(4) ? npc.fierceness / 2 : 0)
                                - blind_attack_penalty;
          
          // Determine if NPC hits the PC.
          // e.g. d12 + 1 + (2 + 10/2) >= (10 + 10/2).
          // d12 + 8 >= 15.
          if (npc_attack_roll >= pc_ac)
          {
            // NPC hits the PC.
            pc_damage += f_calc_melee_damage(npc_melee_weapon, npc.get_melee_damage_bonus()) + (rnd::dice(6) ? npc.fierceness : 0);
          }
        }
          
        if (pc_damage > 0)
        {
          // Apply damage to the player
          bool was_alive = m_player.health > 0;
          m_player.health -= pc_damage;
          if (was_alive && m_player.health <= 0)
          {
            message_handler->add_message(real_time_s,
                                         "You were killed!",
                                         MessageHandler::Level::Fatal);
            broadcast([](auto* listener) { listener->on_pc_death(); });
          }
        }
      }
      
      for (auto& p : active_projectiles)
      {
        // Move projectile toward target using sim_dt_s.
        float dist_to_move = p.speed * sim_dt_s;
        // Update p.pos here (simple linear interpolation or grid stepping).
        p.pos += p.dir * dist_to_move;
      }
    }
    
    template<int NR, int NC>
    void draw_fighting(ScreenHandler<NR, NC>& sh, const RC& pc_scr_pos, bool do_update_fight, float real_time_s, float sim_time_s,
                       int melee_blood_prob_visible, int melee_blood_prob_invisible)
    {
      auto f_render_pc_blood_splats = [&](const RC& offs)
      {
        auto& bs = m_player.blood_splats.emplace_back(m_environment.get(), m_player.curr_floor, m_player.pos + offs, rnd::dice(4), sim_time_s, offs);
        bs.curr_room = m_player.curr_room;
        bs.curr_corridor = m_player.curr_corridor;
        if (m_player.is_inside_curr_room())
          bs.is_underground = m_environment->is_underground(m_player.curr_floor, m_player.curr_room);
        else if (m_player.is_inside_curr_corridor())
          bs.is_underground = m_environment->is_underground(m_player.curr_floor, m_player.curr_corridor);
      };
      
      auto f_render_npc_blood_splats = [&](NPC& npc, const RC& offs)
      {
        auto& bs = npc.blood_splats.emplace_back(m_environment.get(), npc.curr_floor, npc.pos + offs, rnd::dice(4), sim_time_s, offs);
        bs.curr_room = npc.curr_room;
        bs.curr_corridor = npc.curr_corridor;
        bs.is_underground = npc.is_underground;
      };
      
      if (m_player.health > 0)
      {
        if (m_player.ranged_weapon_hit)
        {
          RC offs = { rnd::rand_int(-1, +1), rnd::rand_int(-1, +1) };
          if (m_environment->is_inside_any_room(m_player.curr_floor, m_player.pos + offs))
            f_render_pc_blood_splats(offs);
          
          m_player.ranged_weapon_hit = false;
        }
      
        for (auto& npc : all_npcs)
        {
          if (npc.health <= 0)
            continue;
          
          if (npc.state == State::FightMelee || npc.state == State::FightRanged)
          {
            if (npc.trg_info_hostile_npc.once())
            {
              std::string message = "You are being attacked";
              std::string race = race2str(npc.npc_race);
              if (npc.visible && !race.empty())
                message += " by " + str::indef_art(race);
              message += "!";
              message_handler->add_message(real_time_s,
                                           message, MessageHandler::Level::Warning);
            }
          }
          else
            npc.trg_info_hostile_npc.reset();
          
          if (npc.state == State::FightMelee)
          {
            auto npc_scr_pos = m_screen_helper->get_screen_pos(npc.pos);
            
            // [side_case, base_case, side_case]
            // Case NW (dp = [1, 1]):
            //O#
            //#*
            // r + [1, 1, 0]
            // c + [0, 1, 1]
            //
            // Case W (dp = [0, 1]):
            // #
            //O*
            // #
            // r + [1, 0, -1]
            // c + [1, 1,  1]
            //
            // Case SW (dp = [-1, 1]):
            //#*
            //O#
            // r + [0, -1, -1]
            // c + [1,  1,  0]
            //
            // Case S (dp = [-1, 0]):
            //#*#
            // O
            // r + [-1, -1, -1]
            // c + [ 1,  0, -1]
            //
            // Case SE (dp = [-1, -1]):
            //*#
            //#O
            // r + [-1, -1,  0]
            // c + [ 0, -1, -1]
            //
            // Case E (dp = [0, -1]):
            //#
            //*O
            //#
            // r + [-1,  0,  1]
            // c + [-1, -1, -1]
            //
            // Case NE (dp = [1, -1]):
            //#O
            //*#
            // r + [ 0,  1, 1]
            // c + [-1, -1, 0]
            //
            // Case N (dp = [1, 0]):
            // O
            //#*#
            // r + [ 1, 1, 1]
            // c + [-1, 0, 1]
            
            auto dp = m_player.pos - npc.pos;
            dp.r = math::sgn(dp.r);
            dp.c = math::sgn(dp.c);
            
            auto f_dp_to_dir = [](const RC& dp)
            {
              if (dp == RC { 1, 1 }) return FightDir::NW;
              if (dp == RC { 0, 1 }) return FightDir::W;
              if (dp == RC { -1, 1 }) return FightDir::SW;
              if (dp == RC { -1, 0 }) return FightDir::S;
              if (dp == RC { -1, -1 }) return FightDir::SE;
              if (dp == RC { 0, -1 }) return FightDir::E;
              if (dp == RC { 1, -1 }) return FightDir::NE;
              if (dp == RC { 1, 0 }) return FightDir::N;
              return FightDir::NUM_ITEMS;
            };
            const auto num_dir = static_cast<int>(FightDir::NUM_ITEMS);
            
            auto f_calc_fight_offs = [&](const RC& dp)
            {
              auto dir = static_cast<int>(f_dp_to_dir(dp));
              
              if (dir >= static_cast<int>(FightDir::NUM_ITEMS))
                return RC { 0, 0 };
                
              auto r_offs = rnd::randn_select(0.f, 1.f, std::vector {
                fight_r_offs[(num_dir + dir - 1)%num_dir],
                fight_r_offs[dir],
                fight_r_offs[(dir + 1)%num_dir] });
              auto c_offs = rnd::randn_select(0.f, 1.f, std::vector {
                fight_c_offs[(num_dir + dir - 1)%num_dir],
                fight_c_offs[dir],
                fight_c_offs[(dir + 1)%num_dir] });
              return RC { r_offs, c_offs };
            };
            auto f_render_fight = [&](PlayerBase* pb, const RC& scr_pos, const RC& offs)
            {
              // #FIXME:
              if (do_update_fight)
              {
                pb->cached_fight_style = styles::Style
                {
                  color::get_random_color(c_fight_colors),
                  Color::Transparent2
                };
                pb->cached_fight_str = rnd::rand_select(c_fight_strings);
              }
              auto fight_style = pb->cached_fight_style;
              auto fight_str = pb->cached_fight_str;
              
              sh.write_buffer(fight_str,
                              scr_pos.r + offs.r,
                              scr_pos.c + offs.c,
                              fight_style);
            };
            if (do_update_fight)
              m_player.cached_fight_offs = f_calc_fight_offs(dp);
            auto offs = m_player.cached_fight_offs;
            if (m_environment->is_inside_any_room(m_player.curr_floor, m_player.pos + offs))
            {
              f_render_fight(&m_player, npc_scr_pos, offs);
              if (do_update_fight && rnd::one_in(npc.visible ? melee_blood_prob_visible : melee_blood_prob_invisible))
                f_render_pc_blood_splats(offs);
            }
            if (npc.visible)
            {
              if (do_update_fight)
                npc.cached_fight_offs = f_calc_fight_offs(-dp);
              auto offs = npc.cached_fight_offs;
              if (m_environment->is_inside_any_room(npc.curr_floor, npc.pos + offs))
              {
                f_render_fight(&npc, pc_scr_pos, offs);
                if (do_update_fight && rnd::one_in(npc.visible ? melee_blood_prob_visible : melee_blood_prob_invisible))
                  f_render_npc_blood_splats(npc, offs);
              }
            }
          }
          else if (npc.ranged_weapon_hit)
          {
            RC offs = { rnd::rand_int(-1, +1), rnd::rand_int(-1, +1) };
            if (m_environment->is_inside_any_room(npc.curr_floor, npc.pos + offs))
              f_render_npc_blood_splats(npc, offs);
          
            npc.ranged_weapon_hit = false;
          }
        }
      }
      for (const auto& p : active_projectiles)
      {
        char p_char = p.weapon->projectile_characters[p.ang_idx];
        
        const RC& wpn_pos = to_RC_round(p.pos);
        
        const RC& wpn_scr_pos = m_screen_helper->get_screen_pos(wpn_pos);
        
        bool light = false;
        bool fog_of_war = true;
        if (p.curr_room != nullptr)
        {
          const auto& bb = p.curr_room->bb_leaf_room;
          int idx = (wpn_pos.r - bb.r)*bb.c_len + (wpn_pos.c - bb.c);
          light = p.curr_room->light[idx];
          fog_of_war = p.curr_room->fog_of_war[idx];
        }
        else if (p.curr_corridor != nullptr)
        {
          const auto& bb = p.curr_corridor->bb;
          int idx = (wpn_pos.r - bb.r)*bb.c_len + (wpn_pos.c - bb.c);
          light = p.curr_corridor->light[idx];
          fog_of_war = p.curr_corridor->fog_of_war[idx];
        }
        bool visible = !((use_fog_of_war && fog_of_war) ||
                      ((m_environment->is_underground(p.curr_floor, p.curr_room) || calc_night(p)) && !light)); // #FIXME: add fow term.
        if (visible)
          sh.write_buffer(std::string(1, p_char), wpn_scr_pos, p.weapon->projectile_fg_color, Color::Transparent2);
      }
      stlutils::erase_if(active_projectiles, [sim_time_s](const auto& p)
      {
        return p.hit
            || p.travel_time.finished(sim_time_s)
            || (p.curr_room != nullptr && p.curr_room->bb_leaf_room.find_location(to_RC_round(p.pos)) != ttl::BBLocation::Inside);
      });
    }
    
    void assign_room_properties(Item& item, const std::optional<RoomStyle>& rs, bool assure_contrasting_fg_colors)
    {
      const auto& room_style = rs.value();
      item.is_underground = room_style.is_underground;
      if (assure_contrasting_fg_colors)
        while (item.style.fg_color == room_style.get_fill_style().bg_color)
          item.change_fg_color();
    }
    
  public:
    DungGine(bool use_fow, bool sorted_inventory_items, DungGineTextureParams texture_params = {})
      : message_handler(std::make_unique<MessageHandler>())
      , use_fog_of_war(use_fow)
    {
      m_screen_helper = std::make_unique<ScreenHelper>();
      m_environment = std::make_unique<Environment>();
      m_environment->load_textures(texture_params);
      m_inventory = std::make_unique<Inventory>();
      m_keyboard = std::make_unique<Keyboard>(m_environment.get(), m_inventory.get(), message_handler.get(),
                                              m_player,
                                              all_keys, all_lamps, all_weapons, all_potions, all_armour,
                                              all_npcs,
                                              trigger_game_save, trigger_game_load, trigger_screenshot,
                                              tbd, debug);
      if (sorted_inventory_items)
        sort_inventory.reset();
    }
    
    void load_dungeon(Dungeon& dungeon)
    {
      m_environment->load_dungeon(dungeon);
      all_npcs.clear();
      all_keys.clear();
      all_lamps.clear();
      all_weapons.clear();
      all_potions.clear();
      all_armour.clear();
    }
    
    void style_dungeon(WallShadingType wall_shading_surface_level,
                       WallShadingType wall_shading_underground)
    {
      m_environment->style_dungeon(m_latitude, m_longitude,
                                   wall_shading_surface_level,
                                   wall_shading_underground);
    }
    
    void set_player_character(char ch) { m_player.character = ch; }
    void set_player_style(const Style& style) { m_player.style = style; }
    bool place_player(const RC& screen_size, std::optional<RC> world_pos = std::nullopt)
    {
      auto curr_floor = m_environment->get_init_floor(); // #NOTE: get_init_floor() should only be called here!
      const auto& room_corridor_map = m_environment->get_room_corridor_map(curr_floor);
      const auto world_size = m_environment->get_world_size(curr_floor);
      m_screen_helper->set_screen_size(screen_size);
      
      m_player.curr_floor = curr_floor;
      
      if (!world_pos.has_value())
      {
        const int c_max_num_iters = 1e2_i;
        int num_iters = 0;
        int rnd_sel = 0;
        auto f_find_rnd_corr = [&room_corridor_map](int rnd_sel) -> Corridor*
        {
          int ctr = 0;
          for (auto it = room_corridor_map.begin(); it != room_corridor_map.end(); ++it)
            if (rnd_sel == ctr++)
              return it->second;
          return nullptr;
        };
        do
        {
          rnd_sel = rnd::rand_idx(room_corridor_map.size());
          auto* rnd_corr = f_find_rnd_corr(rnd_sel);
          if (rnd_corr != nullptr)
          {
            if (rnd_corr->orientation == Orientation::Vertical && rnd_corr->bb.r_len > 4)
              break;
            if (rnd_corr->orientation == Orientation::Horizontal && rnd_corr->bb.c_len > 4)
              break;
          }
        } while (++num_iters < c_max_num_iters);
        auto* rnd_corr = f_find_rnd_corr(rnd_sel);
        if (rnd_corr != nullptr)
        {
          m_player.is_spawned = true;
          m_player.curr_corridor = rnd_corr;
          const auto& bb = rnd_corr->bb;
          m_player.pos = {
            rnd_corr->orientation == Orientation::Vertical ? rnd::rand_int(bb.top() + 1, bb.bottom() - 1) : bb.r_mid(),
            rnd_corr->orientation == Orientation::Horizontal ? rnd::rand_int(bb.left() + 1, bb.right() - 1) : bb.c_mid()
          };
          m_player.last_pos = m_player.pos;
          m_screen_helper->focus_on_world_pos_mid_screen(m_player.pos);
          return true;
        }
        return false;
      }
      else // Try to find a valid corridor position near the requested position i.e. argument "world_pos".
      {
        m_player.pos = world_pos.value();
        
        m_player.last_pos = m_player.pos;
        
        const int c_max_num_iters = 1e5_i;
        int num_iters = 0;
        do
        {
          for (const auto& cp : room_corridor_map)
            if (cp.second->is_inside_corridor(m_player.pos))
            {
              m_player.is_spawned = true;
              m_player.curr_corridor = cp.second;
              m_screen_helper->focus_on_world_pos_mid_screen(m_player.pos);
              return true;
            }
          m_player.pos += { rnd::rand_int(-2, +2), rnd::rand_int(-2, +2) };
          m_player.pos = m_player.pos.clamp(0, world_size.r, 0, world_size.c);
        } while (++num_iters < c_max_num_iters);
        return false;
      }
    }
    
    void configure_save_game(std::optional<std::string> dunggine_lib_repo_path)
    {
      use_save_game_git_hash_check = dunggine_lib_repo_path.has_value();
      if (dunggine_lib_repo_path.has_value())
        path_to_dunggine_repo = dunggine_lib_repo_path.value();
    }
    
    // Randomizes the starting direction of the sun and the starting season.
    void configure_sun_rand(float minutes_per_day = 20.f, float minutes_per_year = 120.f,
                            Latitude latitude = Latitude::NorthernHemisphere,
                            Longitude longitude = Longitude::F,
                            bool use_per_room_lat_long_for_sun_dir = true)
    {
      configure_sun(rnd::rand(), minutes_per_day, 
                    static_cast<Season>(rnd::rand_int(0, 7)), minutes_per_year,
                    latitude, longitude, use_per_room_lat_long_for_sun_dir);
    }
    
    void configure_sun(float sun_day_t_offs = 0.f, float minutes_per_day = 20.f,
                       Season start_season = Season::Spring, float minutes_per_year = 120.f,
                       Latitude latitude = Latitude::NorthernHemisphere,
                       Longitude longitude = Longitude::F,
                       bool use_per_room_lat_long_for_sun_dir = true)
    {
      m_latitude = latitude;
      m_longitude = longitude;
      m_season = start_season;
      m_sun_year_t_offs = static_cast<float>(start_season)/7;
      m_sun_minutes_per_year = minutes_per_year;
      
      m_sun_minutes_per_day = minutes_per_day;
      m_sun_day_t_offs = math::clamp(sun_day_t_offs, 0.f, 1.f);
      m_sun_dir = m_solar_motion.get_solar_direction(m_latitude, m_longitude, m_season, m_sun_day_t_offs);
      m_use_per_room_lat_long_for_sun_dir = use_per_room_lat_long_for_sun_dir;
    }
    
    bool place_keys(bool only_place_on_dry_land, bool assure_contrasting_fg_colors, bool only_place_on_same_floor)
    {
      const int c_max_num_iters = 1e5_i;
      const auto* dungeon = m_environment->get_dungeon();
      for (int f_idx = 0; f_idx < m_environment->num_floors(); ++f_idx)
      {
        auto* bsp_tree_doors = dungeon->get_tree(f_idx);
        const auto& door_vec = bsp_tree_doors->fetch_doors();
        for (auto* d : door_vec)
        {
          if (d->is_locked)
          {
            auto fk_idx = only_place_on_same_floor ? f_idx : rnd::dice(m_environment->num_floors()) - 1;
            auto* bsp_tree_key = only_place_on_same_floor ? bsp_tree_doors : dungeon->get_tree(fk_idx);
            const auto world_size = bsp_tree_key->get_world_size();
          
            Key key;
            key.curr_floor = fk_idx;
            key.key_id = d->key_id;
            bool valid_pos = false;
            int num_iters = 0;
            do
            {
              key.pos =
              {
                rnd::rand_int(0, world_size.r),
                rnd::rand_int(0, world_size.c)
              };
              
              valid_pos = m_environment->is_inside_any_room(bsp_tree_key, key.pos, &key.curr_room);
              if (only_place_on_dry_land &&
                  key.curr_room != nullptr &&
                  !is_dry(m_environment->get_terrain(key.curr_floor, key.pos)))
              {
                valid_pos = false;
              }
            } while (num_iters++ < c_max_num_iters && !valid_pos);
            
            if (key.curr_room != nullptr)
            {
              auto rs = m_environment->find_room_style(key.curr_floor, key.curr_room);
              if (rs.has_value())
                assign_room_properties(key, rs, assure_contrasting_fg_colors);
              else
              {
                std::cerr << "ERROR in place_keys() : Unable to find room style for placed key!\n";
                return false;
              }
            }
            else
            {
              std::cerr << "ERROR in place_keys() : Unable to find room for key!\n";
              return false;
            }
            
            all_keys.emplace_back(key);
          }
        }
      }
      return true;
    }
    
    bool place_lamps(int num_torches_per_floor, int num_lanterns_per_floor, int num_magic_lamps_per_floor,
                     bool only_place_on_dry_land, bool assure_contrasting_fg_colors)
    {
      const int c_max_num_iters = 1e5_i;
      const int num_lamps_per_floor = num_torches_per_floor + num_lanterns_per_floor + num_magic_lamps_per_floor;
      const auto* dungeon = m_environment->get_dungeon();
      for (int f_idx = 0; f_idx < m_environment->num_floors(); ++f_idx)
      {
        auto* bsp_tree = dungeon->get_tree(f_idx);
        const auto world_size = bsp_tree->get_world_size();
        int ctr_torches = 0;
        int ctr_lanterns = 0;
        int ctr_magic_lamps = 0;
        for (int lamp_idx = 0; lamp_idx < num_lamps_per_floor; ++lamp_idx)
        {
          Lamp lamp;
          lamp.curr_floor = f_idx;
          Lamp::LampType lamp_type = Lamp::LampType::Torch;
          if (ctr_torches++ < num_torches_per_floor)
            lamp_type = Lamp::LampType::Torch;
          else if (ctr_lanterns++ < num_lanterns_per_floor)
            lamp_type = Lamp::LampType::Lantern;
          else if (ctr_magic_lamps++ < num_magic_lamps_per_floor)
            lamp_type = Lamp::LampType::MagicLamp;
          lamp.init_rand(lamp_type);
          bool valid_pos = false;
          int num_iters = 0;
          do
          {
            if (lamp_idx == 0 && num_iters < 50)
            {
              lamp.pos =
              {
                rnd::rand_int(m_player.pos.r - 20, m_player.pos.r + 20),
                rnd::rand_int(m_player.pos.c - 20, m_player.pos.c + 20)
              };
            }
            else
            {
              lamp.pos =
              {
                rnd::rand_int(0, world_size.r),
                rnd::rand_int(0, world_size.c)
              };
            }

            valid_pos = m_environment->is_inside_any_room(bsp_tree, lamp.pos, &lamp.curr_room);
            if (only_place_on_dry_land &&
                lamp.curr_room != nullptr &&
                !is_dry(m_environment->get_terrain(lamp.curr_floor, lamp.pos)))
            {
              valid_pos = false;
            }
          } while (num_iters++ < c_max_num_iters && !valid_pos);
          
          if (lamp.curr_room != nullptr)
          {
            auto rs = m_environment->find_room_style(lamp.curr_floor, lamp.curr_room);
            if (rs.has_value())
              assign_room_properties(lamp, rs, assure_contrasting_fg_colors);
            else
            {
              std::cerr << "ERROR in place_lamps() : Unable to find room style for placed lamp!\n";
              return false;
            }
          }
          else
          {
            std::cerr << "ERROR in place_lamps() : Unable to find room for lamp!\n";
            return false;
          }
          
          all_lamps.emplace_back(lamp);
        }
      }
      return true;
    }
    
    bool place_weapons(int num_daggers_per_floor,
                       int num_swords_per_floor,
                       int num_flails_per_floor,
                       int num_morningstars_per_floor,
                       int num_slings_per_floor,
                       int num_bows_per_floor,
                       int num_crossbows_per_floor,
                       bool only_place_on_dry_land, bool assure_contrasting_fg_colors)
    {
      const int c_max_num_iters = 1e5_i;
      const auto* dungeon = m_environment->get_dungeon();
      const int num_weapons_per_floor = num_daggers_per_floor
                                        + num_swords_per_floor
                                        + num_flails_per_floor
                                        + num_morningstars_per_floor
                                        + num_slings_per_floor
                                        + num_bows_per_floor
                                        + num_crossbows_per_floor;
      for (int f_idx = 0; f_idx < m_environment->num_floors(); ++f_idx)
      {
        auto* bsp_tree = dungeon->get_tree(f_idx);
        const auto world_size = bsp_tree->get_world_size();
        int ctr_daggers = 0;
        int ctr_swords = 0;
        int ctr_flails = 0;
        int ctr_morningstars = 0;
        int ctr_slings = 0;
        int ctr_bows = 0;
        int ctr_crossbows = 0;
        for (int wpn_idx = 0; wpn_idx < num_weapons_per_floor; ++wpn_idx)
        {
          std::unique_ptr<Weapon> weapon;
          if (ctr_daggers++ < num_daggers_per_floor)
            weapon = std::make_unique<Dagger>();
          else if (ctr_swords++ < num_swords_per_floor)
            weapon = std::make_unique<Sword>();
          else if (ctr_flails++ < num_flails_per_floor)
            weapon = std::make_unique<Flail>();
          else if (ctr_morningstars++ < num_morningstars_per_floor)
            weapon = std::make_unique<MorningStar>();
          else if (ctr_slings++ < num_slings_per_floor)
            weapon = std::make_unique<Sling>();
          else if (ctr_bows++ < num_bows_per_floor)
            weapon = std::make_unique<Bow>();
          else if (ctr_crossbows++ < num_crossbows_per_floor)
            weapon = std::make_unique<Crossbow>();
          weapon->curr_floor = f_idx;
          bool valid_pos = false;
          int num_iters = 0;
          do
          {
            weapon->pos =
            {
              rnd::rand_int(0, world_size.r),
              rnd::rand_int(0, world_size.c)
            };

            valid_pos = m_environment->is_inside_any_room(bsp_tree, weapon->pos, &weapon->curr_room);
            if (only_place_on_dry_land &&
                weapon->curr_room != nullptr &&
                !is_dry(m_environment->get_terrain(weapon->curr_floor, weapon->pos)))
            {
              valid_pos = false;
            }
          } while (num_iters++ < c_max_num_iters && !valid_pos);
          
          if (weapon->curr_room != nullptr)
          {
            auto rs = m_environment->find_room_style(weapon->curr_floor, weapon->curr_room);
            if (rs.has_value())
              assign_room_properties(*weapon.get(), rs, assure_contrasting_fg_colors);
            else
            {
              std::cerr << "ERROR in place_weapons() : Unable to find room style for placed weapon!\n";
              return false;
            }
          }
          else
          {
            std::cerr << "ERROR in place_weapons() : Unable to find room for weapon!\n";
            return false;
          }
          
          all_weapons.emplace_back(weapon.release());
        }
      }
      return true;
    }
    
    bool place_potions(int num_health_potions_per_floor, int num_poison_potions_per_floor,
                       bool only_place_on_dry_land, bool assure_contrasting_fg_colors)
    {
      const int c_max_num_iters = 1e5_i;
      const auto* dungeon = m_environment->get_dungeon();
      const int num_potions_per_floor = num_health_potions_per_floor + num_poison_potions_per_floor;
      for (int f_idx = 0; f_idx < m_environment->num_floors(); ++f_idx)
      {
        auto* bsp_tree = dungeon->get_tree(f_idx);
        const auto world_size = bsp_tree->get_world_size();
        //int ctr_health_potions = 0;
        int ctr_poison_potions = 0;
        for (int pot_idx = 0; pot_idx < num_potions_per_floor; ++pot_idx)
        {
          Potion potion;
          if (ctr_poison_potions++ < num_poison_potions_per_floor)
            potion.poison = true;
          potion.curr_floor = f_idx;
          bool valid_pos = false;
          int num_iters = 0;
          do
          {
            potion.pos =
            {
              rnd::rand_int(0, world_size.r),
              rnd::rand_int(0, world_size.c)
            };

            valid_pos = m_environment->is_inside_any_room(bsp_tree, potion.pos, &potion.curr_room);
            if (only_place_on_dry_land &&
                potion.curr_room != nullptr &&
                !is_dry(m_environment->get_terrain(potion.curr_floor, potion.pos)))
            {
              valid_pos = false;
            }
          } while (num_iters++ < c_max_num_iters && !valid_pos);
          
          if (potion.curr_room != nullptr)
          {
            auto rs = m_environment->find_room_style(potion.curr_floor, potion.curr_room);
            if (rs.has_value())
              assign_room_properties(potion, rs, assure_contrasting_fg_colors);
            else
            {
              std::cerr << "ERROR in place_potions() : Unable to find room style for placed potion!\n";
              return false;
            }
          }
          else
          {
            std::cerr << "ERROR in place_potions() : Unable to find room for potion!\n";
            return false;
          }
          
          all_potions.emplace_back(potion);
        }
      }
      return true;
    }
    
    bool place_armour(int num_shields_per_floor, int num_gambesons_per_floor,
                      int num_cmhs_per_floor, int num_pbas_per_floor,
                      int num_padded_coifs_per_floor, int num_cmcs_per_floor,
                      int num_helmets_per_floor,
                      bool only_place_on_dry_land, bool assure_contrasting_fg_colors)
    {
      const int c_max_num_iters = 1e5_i;
      const auto* dungeon = m_environment->get_dungeon();
      const int num_armour_per_floor = num_shields_per_floor
                                       + num_gambesons_per_floor
                                       + num_cmhs_per_floor
                                       + num_pbas_per_floor
                                       + num_padded_coifs_per_floor
                                       + num_cmcs_per_floor
                                       + num_helmets_per_floor;
      for (int f_idx = 0; f_idx < m_environment->num_floors(); ++f_idx)
      {
        auto* bsp_tree = dungeon->get_tree(f_idx);
        const auto world_size = bsp_tree->get_world_size();
        int ctr_shields = 0;
        int ctr_gambesons = 0;
        int ctr_cmhs = 0;
        int ctr_pbas = 0;
        int ctr_padded_coifs = 0;
        int ctr_cmcs = 0;
        int ctr_helmets = 0;
        for (int a_idx = 0; a_idx < num_armour_per_floor; ++a_idx)
        {
          std::unique_ptr<Armour> armour;
          if (ctr_shields++ < num_shields_per_floor)
            armour = std::make_unique<Shield>();
          else if (ctr_gambesons++ < num_gambesons_per_floor)
            armour = std::make_unique<Gambeson>();
          else if (ctr_cmhs++ < num_cmhs_per_floor)
            armour = std::make_unique<ChainMailleHauberk>();
          else if (ctr_pbas++ < num_pbas_per_floor)
            armour = std::make_unique<PlatedBodyArmour>();
          else if (ctr_padded_coifs++ < num_padded_coifs_per_floor)
            armour = std::make_unique<PaddedCoif>();
          else if (ctr_cmcs++ < num_cmcs_per_floor)
            armour = std::make_unique<ChainMailleCoif>();
          else if (ctr_helmets++ < num_helmets_per_floor)
            armour = std::make_unique<Helmet>();
          armour->curr_floor = f_idx;
          bool valid_pos = false;
          int num_iters = 0;
          do
          {
            armour->pos =
            {
              rnd::rand_int(0, world_size.r),
              rnd::rand_int(0, world_size.c)
            };

            valid_pos = m_environment->is_inside_any_room(bsp_tree, armour->pos, &armour->curr_room);
            if (only_place_on_dry_land &&
                armour->curr_room != nullptr &&
                !is_dry(m_environment->get_terrain(armour->curr_floor, armour->pos)))
            {
              valid_pos = false;
            }
          } while (num_iters++ < c_max_num_iters && !valid_pos);
          
          if (armour->curr_room != nullptr)
          {
            auto rs = m_environment->find_room_style(armour->curr_floor, armour->curr_room);
            if (rs.has_value())
              assign_room_properties(*armour.get(), rs, assure_contrasting_fg_colors);
            else
            {
              std::cerr << "ERROR in place_armour() : Unable to find room style for placed armour!\n";
              return false;
            }
          }
          else
          {
            std::cerr << "ERROR in place_armour() : Unable to find room for armour!\n";
            return false;
          }
          
          all_armour.emplace_back(armour.release());
        }
      }
      return true;
    }
    
    bool place_npcs(int num_npcs, bool only_place_on_dry_land)
    {
      const int c_max_num_iters = 1e5_i;
      const auto* dungeon = m_environment->get_dungeon();
      for (int f_idx = 0; f_idx < m_environment->num_floors(); ++f_idx)
      {
        auto* bsp_tree = dungeon->get_tree(f_idx);
        const auto world_size = bsp_tree->get_world_size();
        for (int npc_idx = 0; npc_idx < num_npcs; ++npc_idx)
        {
          NPC npc;
          npc.curr_floor = f_idx;
          npc.npc_class = rnd::rand_enum<Class>();
          npc.npc_race = rnd::rand_enum<Race>();
          bool valid_pos = false;
          int num_iters = 0;
          do
          {
            npc.pos =
            {
              rnd::rand_int(0, world_size.r),
              rnd::rand_int(0, world_size.c)
            };

            valid_pos = m_environment->is_inside_any_room(bsp_tree, npc.pos, &npc.curr_room);
            if (only_place_on_dry_land &&
                npc.curr_room != nullptr)
            {
              if (!is_dry(m_environment->get_terrain(npc.curr_floor, npc.pos)))
                valid_pos = false;
              else if (!m_environment->allow_move_to(npc.curr_floor, npc.pos.r, npc.pos.c))
                valid_pos = false;
            }
          } while (num_iters++ < c_max_num_iters && !valid_pos);
                    
          if (npc.curr_room != nullptr)
          {
            npc.is_underground = m_environment->is_underground(npc.curr_floor, npc.curr_room);
            npc.init(all_weapons, all_armour);
          }
          else
          {
            std::cerr << "ERROR in place_npcs() : Unable to find room for npc!\n";
            return false;
          }
          
          all_npcs.emplace_back(npc);
        }
      }
      return true;
    }
    
    void update(int frame_ctr, float fps,
                double real_time_s, float sim_time_s, float sim_dt_s,
                float fire_smoke_dt_factor, float projectile_speed_factor,
                int melee_attack_dice, int ranged_attack_dice,
                const keyboard::KeyPressDataPair& kpdp, bool* game_over)
    {
      utils::try_set(game_over, m_player.health <= 0);
      if (utils::try_get(game_over))
        return;
        
      stall_game = m_player.show_inventory;
      
      bool do_los_terrainos = frame_ctr % std::max(1, math::roundI(fps / 5)) == 0;
      bool do_npc_move = frame_ctr % std::max(1, math::roundI(fps / 4)) == 0;
      bool do_fight = frame_ctr % std::max(1, math::roundI(fps / 3)) == 0;
    
      update_sun(static_cast<float>(real_time_s));
      
      auto fow_radius = 5.5f;
      auto* lamp = m_player.get_selected_lamp(m_inventory.get());
      if (lamp != nullptr)
      {
        math::maximize(fow_radius, lamp->radius);
        math::minimize(fow_radius, globals::max_fow_radius);
      }
      
      set_visibilities(fow_radius, m_player.pos);
      
      auto& curr_pos = m_player.pos;
      m_keyboard->handle_keyboard(kpdp, real_time_s);
      
      update_inventory();
      
      if (stall_game)
        return;
      
      // Fog of war
      if (use_fog_of_war)
        update_field(curr_pos,
                     [](auto obj) { return &obj->fog_of_war; },
                     false, fow_radius, 0.f, Lamp::LightType::Isotropic);
                  
      // Light
      clear_field([](auto obj) { return &obj->light; }, false);
      if (lamp != nullptr)
      {
        update_field(curr_pos,
                     [](auto obj) { return &obj->light; },
                     true, lamp->radius, lamp->angle_deg,
                     lamp->light_type);
      }
      
      // Update current room and current corridor.
      if (m_player.curr_corridor != nullptr)
      {
        auto* door_0 = m_player.curr_corridor->doors[0];
        auto* door_1 = m_player.curr_corridor->doors[1];
        if (door_0 != nullptr && curr_pos == door_0->pos)
          m_player.curr_room = door_0->room;
        else if (door_1 != nullptr && curr_pos == door_1->pos)
          m_player.curr_room = door_1->room;
      }
      if (m_player.curr_room != nullptr)
      {
        for (auto* door : m_player.curr_room->doors)
          if (curr_pos == door->pos)
          {
            m_player.curr_corridor = door->corridor;
            break;
          }
      }
      
      // PC LOS etc.
      bool was_alive = m_player.health > 0;
      m_player.on_terrain = m_environment->get_terrain(m_player.curr_floor, m_player.pos);
      m_player.update(m_screen_helper.get(), m_inventory.get(),
                      do_los_terrainos,
                      sim_time_s, sim_dt_s * fire_smoke_dt_factor);
      if (was_alive && m_player.health <= 0)
      {
        message_handler->add_message(static_cast<float>(real_time_s),
                                     "You died!",
                                     MessageHandler::Level::Fatal);
        broadcast([](auto* listener) { listener->on_pc_death(); });
      }
      
      // NPCs
      {
        BSPNode* pc_room = m_player.is_inside_curr_room() ? m_player.curr_room : nullptr;
        Corridor* pc_corr = m_player.is_inside_curr_corridor() ? m_player.curr_corridor : nullptr;
        for (auto& npc : all_npcs)
        {
          npc.on_terrain = m_environment->get_terrain(npc.curr_floor, npc.pos);
          npc.update(curr_pos, pc_room, pc_corr, m_environment.get(),
                     do_los_terrainos, do_npc_move,
                     sim_time_s, sim_dt_s);
        
          if (npc.is_hostile && !npc.was_hostile)
            broadcast([&npc](auto* listener) { listener->on_fight_begin(&npc); });
          else if (!npc.is_hostile && npc.was_hostile)
            broadcast([&npc](auto* listener) { listener->on_fight_end(&npc); });
        }
      }
      
      if (do_fight)
        update_fighting(static_cast<float>(real_time_s), sim_time_s, sim_dt_s,
                        projectile_speed_factor,
                        melee_attack_dice, ranged_attack_dice);
        
      if (trigger_game_save)
      {
        std::string filepath = "savegame_0.dsg";
        unsigned int curr_rnd_seed = 0;
        // Expects just one listener.
        broadcast([&filepath, &curr_rnd_seed](auto* l)
          { l->on_save_game_request(filepath, curr_rnd_seed); });
        
        save_game_post_build(filepath, curr_rnd_seed, real_time_s);
      
        trigger_game_save = false;
      }
      else if (trigger_game_load)
      {
        std::string filepath = "savegame_0.dsg";
        unsigned int curr_rnd_seed = 0;
        // Expects just one listener.
        broadcast([&filepath](auto* l)
          { l->on_load_game_request_pre(filepath); });
          
        // Only proceed if we have the correct git hash.
        if (load_game_pre_build(filepath, &curr_rnd_seed, real_time_s))
        {
          broadcast([curr_rnd_seed](auto* listener)
                    { listener->on_load_game_request_post(curr_rnd_seed); });
          
          broadcast([](auto* l) { l->on_scene_rebuild_request(); });
          
          load_game_post_build(filepath, real_time_s);
        }
              
        trigger_game_load = false;
      }
      
      m_screen_helper->update_scrolling(curr_pos);
    }
    
    
    template<int NR, int NC>
    void draw(ScreenHandler<NR, NC>& sh, double real_time_s, float sim_time_s,
              int anim_ctr_swim, int anim_ctr_fight,
              int melee_blood_prob_visible, int melee_blood_prob_invisible,
              ui::VerticalAlignment mb_v_align = ui::VerticalAlignment::CENTER,
              ui::HorizontalAlignment mb_h_align = ui::HorizontalAlignment::CENTER,
              int mb_v_align_offs = 0, int mb_h_align_offs = 0,
              bool framed_mode = false,
              bool gore = false)
    {
      const auto& room_corridor_map = m_environment->get_room_corridor_map(m_player.curr_floor);
      const auto& door_vec = m_environment->fetch_doors(m_player.curr_floor);
      const auto& staircase_vec = m_environment->fetch_staircases(m_player.curr_floor);
      
      MessageBoxDrawingArgs mb_args;
      mb_args.v_align = mb_v_align;
      mb_args.h_align = mb_h_align;
      mb_args.v_align_offs = mb_v_align_offs;
      mb_args.h_align_offs = mb_h_align_offs;
      mb_args.framed_mode = framed_mode;
      message_handler->update(sh, static_cast<float>(real_time_s), mb_args);
      
      if (debug)
      {
        if (!tbd.empty())
        {
          ui::TextBoxDrawingArgsAlign tbd_args;
          tbd_args.v_align = ui::VerticalAlignment::TOP;
          tbd_args.base.box_style = { Color::Blue, Color::Yellow };
          tbd_args.framed_mode = framed_mode;
          tbd.calc_pre_draw(str::Adjustment::Left);
          tbd.draw(sh, tbd_args);
        }
      }
        
      if (m_player.show_inventory)
      {
        m_inventory->set_bounding_box({ 2, 2, NR - 5, NC - 5 });
        m_inventory->draw(sh);
      }
        
      draw_health_bars(sh, framed_mode);
      draw_strength_bar(sh, framed_mode);
      
      auto pc_scr_pos = m_screen_helper->get_screen_pos(m_player.pos);
      
      draw_fighting(sh, pc_scr_pos, anim_ctr_fight % 2 == 0, static_cast<float>(real_time_s), sim_time_s,
                    melee_blood_prob_visible, melee_blood_prob_invisible);
      
      for (auto& bs : m_player.blood_splats)
        bs.update(sim_time_s);
      for (auto& npc : all_npcs)
        for (auto& bs : npc.blood_splats)
          bs.update(sim_time_s);

      if (debug)
      {
        sh.write_buffer(terrain2str(m_player.on_terrain), 5, 1, Color::Black, Color::White);
        sh.write_buffer("Floor: " + std::to_string(m_player.curr_floor), 6, 1, Color::Black, Color::White);
      }
      
      auto f_draw_swim_anim = [anim_ctr_swim, &sh, this](bool is_moving, int curr_floor,
                                                         const RC& pos, const RC& scr_pos,
                                                         float los_r, float los_c)
      {
        if (anim_ctr_swim % 3 == 0 && is_moving)
        {
          RC swim_pos { math::roundI(pos.r - los_r), math::roundI(pos.c - los_c) };
          if (m_environment->is_inside_any_room(curr_floor, swim_pos))
          {
            RC swim_pos_scr { math::roundI(scr_pos.r - los_r), math::roundI(scr_pos.c - los_c) };
            sh.write_buffer("*", swim_pos_scr.r, swim_pos_scr.c, Color::White, Color::Transparent2);
          }
        }
      };
      
      // PC
      if (m_player.is_spawned)
      {
        sh.write_buffer(std::string(1, m_player.character), pc_scr_pos.r, pc_scr_pos.c, m_player.style);
        
        if (is_wet(m_player.on_terrain))
          f_draw_swim_anim(m_player.is_moving, m_player.curr_floor, m_player.pos, pc_scr_pos, m_player.los_r, m_player.los_c);
            
        m_player.draw(sh, sim_time_s);
      }
      
      // Items and NPCs
      auto f_render_item = [&](const auto& obj)
      {
        if (obj.curr_floor != m_player.curr_floor)
          return;
        if (!obj.visible)
          return;
        auto scr_pos = m_screen_helper->get_screen_pos(obj.pos);
        sh.write_buffer(std::string(1, obj.character), scr_pos.r, scr_pos.c, obj.style);
      };
      
      for (const auto& npc : all_npcs)
      {
        if (npc.curr_floor != m_player.curr_floor)
          continue;
        //bool swimming = is_wet(npc.on_terrain) && npc.can_swim && !npc.can_fly;
        bool dead_on_liquid = npc.health <= 0 && is_wet(npc.on_terrain); //&& swimming;
        if (!dead_on_liquid || sim_time_s - npc.death_time_s < 1.5f + (npc.can_fly ? 0.5f : 0.f))
          f_render_item(npc);
        
        if (npc.visible && is_wet(npc.on_terrain))
        {
          auto npc_scr_pos = m_screen_helper->get_screen_pos(npc.pos);
          if (npc.health > 0 && npc.can_swim && !npc.can_fly)
            f_draw_swim_anim(npc.is_moving, npc.curr_floor, npc.pos, npc_scr_pos, npc.los_r, npc.los_c);
          else if (npc.health <= 0)
          {
            float time_delay = 0.f;
            if (npc.can_fly)
            {
              if (math::in_range<float>(sim_time_s - npc.death_time_s, 1.f, 1.5f, Range::Closed))
              {
                //    ***
                //   *   *
                //    ***
                for (int r_offs = -1; r_offs <= +1; ++r_offs)
                  for (int c_offs = -2; c_offs <= +2; ++c_offs)
                  {
                    if (r_offs != 0 && std::abs(c_offs) == 2)
                      continue;
                    if (r_offs == 0 && std::abs(c_offs) < 2)
                      continue;
                    if (r_offs == 0 && c_offs == 0)
                      continue;
                    RC offs_pos { r_offs, c_offs };
                    RC npc_scr_offs_pos = npc_scr_pos + offs_pos;
                    RC npc_world_offs_pos = npc.pos + offs_pos;
                    if (m_environment->is_inside_any_room(npc.curr_floor, npc_world_offs_pos))
                      sh.write_buffer("*", npc_scr_offs_pos.r, npc_scr_offs_pos.c, Color::White, Color::Transparent2);
                  }
              }
            }
            if (math::in_range<float>(sim_time_s - npc.death_time_s - time_delay, 0.5f, 1.f, Range::Closed))
            {
              for (int r_offs = -1; r_offs <= +1; ++r_offs)
                for (int c_offs = -1; c_offs <= +1; ++c_offs)
                {
                  if (r_offs == 0 && c_offs == 0)
                    continue;
                  RC offs_pos { r_offs, c_offs };
                  RC npc_scr_offs_pos = npc_scr_pos + offs_pos;
                  RC npc_world_offs_pos = npc.pos + offs_pos;
                  if (m_environment->is_inside_any_room(npc.curr_floor, npc_world_offs_pos))
                    sh.write_buffer("*", npc_scr_offs_pos.r, npc_scr_offs_pos.c, Color::White, Color::Transparent2);
                }
            }
          }
        }
        
        if (npc.debug)
        {
          auto scr_pos = m_screen_helper->get_screen_pos(npc.pos);
          
          if (npc.vel_r < 0.f)
            sh.write_buffer("^", scr_pos.r - 1, scr_pos.c, Color::Black, Color::White);
          else if (npc.vel_r > 0.f)
            sh.write_buffer("v", scr_pos.r + 1, scr_pos.c, Color::Black, Color::White);
          
          if (npc.vel_c < 0.f)
            sh.write_buffer("<", scr_pos.r, scr_pos.c - 1, Color::Black, Color::White);
          else if (npc.vel_c > 0.f)
            sh.write_buffer(">", scr_pos.r, scr_pos.c + 1, Color::Black, Color::White);
          
          if (npc.curr_room != nullptr)
          {
            auto scr_pos_room = m_screen_helper->get_screen_pos(npc.curr_room->bb_leaf_room.center());
            bresenham::plot_line(sh, scr_pos, scr_pos_room,
                    ".", Color::White, Color::Transparent2);
          }
          if (npc.curr_corridor != nullptr)
          {
            auto scr_pos_corr = m_screen_helper->get_screen_pos(npc.curr_corridor->bb.center());
            bresenham::plot_line(sh, scr_pos, scr_pos_corr,
                    ".", Color::White, Color::Transparent2);
          }
        }
      }
      
      for (auto* door : door_vec)
      {
        auto door_pos = door->pos;
        auto door_scr_pos = m_screen_helper->get_screen_pos(door_pos);
        std::string door_ch = "^";
        if (door->is_door)
        {
          if (door->is_open)
            door_ch = "L";
          else if (door->is_locked)
            door_ch = "G";
          else
            door_ch = "D";
        }
        sh.write_buffer(door_ch, door_scr_pos.r, door_scr_pos.c, Color::Black, (use_fog_of_war && door->fog_of_war) ? Color::Black : (door->light ? Color::Yellow : Color::DarkYellow));
      }
      
      for (const auto* staircase : staircase_vec)
      {
        auto staircase_scr_pos = m_screen_helper->get_screen_pos(staircase->pos);
        sh.write_buffer("B", staircase_scr_pos.r, staircase_scr_pos.c, (use_fog_of_war && staircase->fog_of_war) ? Color::Black : (staircase->light ? Color::LightGray : Color::DarkGray), Color::Black);
      }
      
      for (const auto& key : all_keys)
        f_render_item(key);
      
      for (const auto& lamp : all_lamps)
        f_render_item(lamp);
      
      for (const auto& weapon : all_weapons)
        f_render_item(*weapon);
        
      for (const auto& potion : all_potions)
        f_render_item(potion);
        
      for (const auto& armour : all_armour)
        f_render_item(*armour);
        
      if (gore)
      {
        auto f_draw_blood_splat = [&sh](int curr_floor, const RC& scr_pos, const BloodSplat& bs)
        {
          if (bs.curr_floor != curr_floor)
            return;
          if (is_wet(bs.terrain) && !bs.alive)
            return;
          std::string str = "";
          switch (bs.shape)
          {
            case 1: str = " "; break;
            case 2: str = "."; break;
            case 3: str = ":"; break;
            case 4: str = "~"; break;
          }
          auto style = styles::make_shaded_style(Color::Red, bs.visible ? color::ShadeType::Bright : color::ShadeType::Dark);
          sh.write_buffer(str, scr_pos.r, scr_pos.c, style);
        };
        for (const auto& bs : m_player.blood_splats)
        {
          auto bs_scr_pos = m_screen_helper->get_screen_pos(bs.pos);
          f_draw_blood_splat(m_player.curr_floor, bs_scr_pos, bs);
        }
        for (const auto& npc : all_npcs)
        {
          for (const auto& bs : npc.blood_splats)
          {
            auto bs_scr_pos = m_screen_helper->get_screen_pos(bs.pos);
            f_draw_blood_splat(m_player.curr_floor, bs_scr_pos, bs);
          }
        }
      }
      
      m_environment->draw_environment(sh, real_time_s,
                                      m_player.curr_floor, use_fog_of_war,
                                      m_sun_dir, m_solar_motion,
                                      m_t_solar_period, m_season,
                                      m_use_per_room_lat_long_for_sun_dir,
                                      m_screen_helper.get(),
                                      debug);
                                      
      if (trigger_screenshot)
      {
        auto screenshot = sh.get_screen_buffer_chars();
        
        std::string filepath = "screenshot_0.txt";
        // Expects just one listener.
        broadcast([&filepath](auto* l)
          { l->on_screenshot_request(filepath); });
        
        if (TextIO::write_file(filepath, screenshot))
        {
          message_handler->add_message(static_cast<float>(real_time_s),
                                       "Successfully saved screenshot:\n\"" + filepath + "\"!",
                                       MessageHandler::Level::Guide,
                                       3.f);
        }
        else
        {
          message_handler->add_message(static_cast<float>(real_time_s),
                                       "ERROR : Unable to save screenshot to file:\n\"" + filepath + "\"!",
                                       MessageHandler::Level::Fatal,
                                       3.f);
        }
        
        trigger_screenshot = false;
      }
    }
    
    std::string get_latest_git_commit_hash() const
    {
      auto old_pwd = folder::get_pwd();
      auto new_pwd = path_to_dunggine_repo;
      if (!new_pwd.empty())
        folder::set_pwd(new_pwd);
      auto hash = sys::exec("git log -1 --pretty=format:\"%H\"");
      folder::set_pwd(old_pwd);
      return hash;
    }
    
    void save_game_post_build(const std::string& savegame_filename, unsigned int curr_rnd_seed, double real_time_s) const
    {
      std::vector<std::string> lines;
      
      lines.emplace_back(use_save_game_git_hash_check ? get_latest_git_commit_hash() : "0");
      
      lines.emplace_back(std::to_string(curr_rnd_seed));
      
      lines.emplace_back("m_environment");
      m_environment->serialize(lines);
      sg::write_var_enum(lines, SG_WRITE_VAR(m_sun_dir));
      sg::write_var_enum(lines, SG_WRITE_VAR(m_latitude));
      sg::write_var_enum(lines, SG_WRITE_VAR(m_longitude));
      sg::write_var_enum(lines, SG_WRITE_VAR(m_season));
      sg::write_var(lines, SG_WRITE_VAR(m_sun_minutes_per_day));
      sg::write_var(lines, SG_WRITE_VAR(m_sun_day_t_offs));
      sg::write_var(lines, SG_WRITE_VAR(m_sun_minutes_per_year));
      sg::write_var(lines, SG_WRITE_VAR(m_sun_year_t_offs));
      sg::write_var(lines, SG_WRITE_VAR(m_t_solar_period));
      sg::write_var(lines, SG_WRITE_VAR(debug));
      
      lines.emplace_back("m_player");
      m_player.serialize(lines);
      
      lines.emplace_back("all_npcs");
      for (const auto& npc : all_npcs)
        npc.serialize(lines);
      
      lines.emplace_back("m_inventory");
      m_inventory->serialize(lines);
      
      lines.emplace_back("all_keys");
      for (const auto& key : all_keys)
        key.serialize(lines);
      
      lines.emplace_back("all_lamps");
      for (const auto& lamp : all_lamps)
        lamp.serialize(lines);
        
      lines.emplace_back("all_weapons");
      for (const auto& weapon : all_weapons)
        weapon->serialize(lines);
        
      lines.emplace_back("all_potions");
      for (const auto& potion : all_potions)
        potion.serialize(lines);
        
      lines.emplace_back("all_armour");
      for (const auto& armour : all_armour)
        armour->serialize(lines);
        
      sg::write_var(lines, "use_fog_of_war", use_fog_of_war);
      
#if false
      //std::unique_ptr<ScreenHelper> m_screen_helper;
#endif
      
      if (TextIO::write_file(savegame_filename, lines))
      {
        message_handler->add_message(static_cast<float>(real_time_s),
                                     "Successfully saved save-game:\n\"" + savegame_filename + "\"!",
                                     MessageHandler::Level::Guide,
                                     3.f);
      }
      else
      {
        message_handler->add_message(static_cast<float>(real_time_s),
                                     "ERROR : Unable to save save-game file:\n\"" + savegame_filename + "\"!",
                                     MessageHandler::Level::Fatal,
                                     3.f);
      }
    }
    
    bool load_game_pre_build(const std::string& savegame_filename, unsigned int* curr_rnd_seed, double real_time_s)
    {
      std::string git_hash;
    
      std::vector<std::string> lines;
      
      if (!TextIO::read_file(savegame_filename, lines))
      {
        message_handler->add_message(static_cast<float>(real_time_s),
                                     "ERROR : Unable to load save-game file:\n\"" + savegame_filename + "\"!",
                                     MessageHandler::Level::Fatal,
                                     3.f);
        return false;
      }
      
      if (use_save_game_git_hash_check &&
          lines[0] != get_latest_git_commit_hash())
      {
        message_handler->add_message(static_cast<float>(real_time_s),
                                     "ERROR : Tried to load a saved game\nbut the git hash of the save-game\ndoesn't match the git hash of the\nlast commit of DungGine!",
                                      MessageHandler::Level::Fatal,
                                      5.f);
        return false;
      }
      std::istringstream iss(lines[1]);
      iss >> *curr_rnd_seed;
      return true;
    }
    
    void load_game_post_build(const std::string& savegame_filename, double real_time_s)
    {
      std::vector<std::string> lines;
      
      TextIO::read_file(savegame_filename, lines);
      
      for (auto it_line = lines.begin() + 2; it_line != lines.end(); ++it_line)
      {
        if (*it_line == "m_environment")
        {
          it_line = m_environment->deserialize(it_line + 1, lines.end());
        }
        else if (sg::read_var_enum<SolarDirection>(&it_line, SG_READ_VAR(m_sun_dir))) {}
        else if (sg::read_var_enum<Latitude>(&it_line, SG_READ_VAR(m_latitude))) {}
        else if (sg::read_var_enum<Longitude>(&it_line, SG_READ_VAR(m_longitude))) {}
        else if (sg::read_var_enum<Season>(&it_line, SG_READ_VAR(m_season))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(m_sun_minutes_per_day))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(m_sun_day_t_offs))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(m_sun_minutes_per_year))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(m_sun_year_t_offs))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(m_t_solar_period))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(debug))) {}
        else if (*it_line == "m_player")
        {
          it_line = m_player.deserialize(it_line + 1, lines.end(), m_environment.get());
        }
        else if (*it_line == "all_npcs")
        {
          for (auto& npc : all_npcs)
            it_line = npc.deserialize(it_line + 1, lines.end(), m_environment.get()); // code smell!!
        }
        
        else if (*it_line == "m_inventory")
          it_line = m_inventory->deserialize(it_line + 1, lines.end());
        
        else if (*it_line == "all_keys")
          for (auto& key : all_keys)
            it_line = key.deserialize(it_line + 1, lines.end(), m_environment.get());
        
        else if (*it_line == "all_lamps")
          for (auto& lamp : all_lamps)
            it_line = lamp.deserialize(it_line + 1, lines.end(), m_environment.get());
        
        else if (*it_line == "all_weapons")
          for (auto& weapon : all_weapons)
            it_line = weapon->deserialize(it_line + 1, lines.end(), m_environment.get());
        
        else if (*it_line == "all_potions")
          for (auto& potion : all_potions)
            it_line = potion.deserialize(it_line + 1, lines.end(), m_environment.get());
        
        else if (*it_line == "all_armour")
          for (auto& armour : all_armour)
            it_line = armour->deserialize(it_line + 1, lines.end(), m_environment.get());
        
        else if (sg::read_var(&it_line, SG_READ_VAR(use_fog_of_war)))
        {
          message_handler->add_message(static_cast<float>(real_time_s),
                                       "Successfully loaded save-game:\n\"" + savegame_filename + "\"!",
                                       MessageHandler::Level::Guide,
                                       3.f);
          return;
        }
        else
        {
          std::cerr << "Error in save game parsing!\n";
        }
      }
    }
    
  };
  
}
