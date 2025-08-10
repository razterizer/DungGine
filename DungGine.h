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
    
    ui::TextBox tb_health, tb_strength;
    ui::TextBoxDebug tbd;
    
    bool stall_game = false;
    
    bool trigger_game_save = false;
    bool trigger_game_load = false;
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
          keys_subgroup->add_item(item_str, &key);
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
          lamps_subgroup->add_item(item_str, &lamp);
        m_player.curr_tot_inv_weight += lamp.weight;
      }
      
      auto* weapons_group = m_inventory->fetch_group("Weapons:");
      auto* weapons_subgroup_melee = weapons_group->fetch_subgroup(0);
      weapons_subgroup_melee->set_title("Melee:");
      for (int inv_wpn_idx = 0; inv_wpn_idx < num_inv_wpns; ++inv_wpn_idx)
      {
        auto wpn_idx = m_player.weapon_idcs[inv_wpn_idx];
        auto& weapon = all_weapons[wpn_idx];
        std::string item_str = "  ";
        if (dynamic_cast<Sword*>(weapon.get()) != nullptr)
          item_str += "Sword";
        else if (dynamic_cast<Dagger*>(weapon.get()) != nullptr)
          item_str += "Dagger";
        else if (dynamic_cast<Flail*>(weapon.get()) != nullptr)
          item_str += "Flail";
        else
          item_str += "<Weapon>";
        item_str += ":";
        item_str += std::to_string(wpn_idx);
        f_format_item_str(item_str, weapon->weight, weapon->price, weapon->damage);
        if (weapons_subgroup_melee->find_item(weapon.get()) == nullptr)
          weapons_subgroup_melee->add_item(item_str, weapon.get());
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
          potions_subgroup->add_item(item_str, &potion);
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
            armour_subgroup->add_item(item_str, armour.get());
        m_player.curr_tot_inv_weight += armour->weight;
      }
      
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
        stlutils::memset(*field, clear_val);
        
        *get_field_ptr(door_0) = clear_val;
        *get_field_ptr(door_1) = clear_val;
      }
      if (m_player.curr_room != nullptr)
      {
        //bb = m_player.curr_room->bb_leaf_room;
        field = get_field_ptr(m_player.curr_room);
        stlutils::memset(*field, clear_val);
        
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
      
      auto f_normalize_angle = [](float& ang)
      {
        while (ang < 0.f)
          ang += math::c_2pi;
        while (ang >= math::c_2pi)
          ang -= math::c_2pi;
      };
      
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
    
              float lo_angle_rad = std::atan2(-dir_lo_r, dir_lo_c);
              float hi_angle_rad = std::atan2(-dir_hi_r, dir_hi_c);
              f_normalize_angle(lo_angle_rad);
              f_normalize_angle(hi_angle_rad);
              if (lo_angle_rad > hi_angle_rad)
                hi_angle_rad += math::c_2pi;
                
              float curr_angle_rad = static_cast<float>(std::atan2(-(obj.pos.r - curr_pos.r), obj.pos.c - curr_pos.c));
              f_normalize_angle(curr_angle_rad);
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
    
    void set_visibilities(float fow_radius, const RC& pc_pos)
    {
      const auto c_fow_radius_sq = math::sq(fow_radius);
    
      auto f_calc_night = [&](const auto& obj) -> bool
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
      
      auto f_fow_near = [&pc_pos, c_fow_radius_sq](const auto& obj) -> bool
      {
        return distance_squared(obj.pos, pc_pos) <= c_fow_radius_sq;
      };
            
      for (auto& key : all_keys)
        key.set_visibility(use_fog_of_war, f_fow_near(key), f_calc_night(key));
      
      for (auto& lamp : all_lamps)
        lamp.set_visibility(use_fog_of_war, f_fow_near(lamp), f_calc_night(lamp));
      
      for (auto& weapon : all_weapons)
        weapon->set_visibility(use_fog_of_war, f_fow_near(*weapon), f_calc_night(*weapon));
      
      for (auto& potion : all_potions)
        potion.set_visibility(use_fog_of_war, f_fow_near(potion), f_calc_night(potion));
        
      for (auto& armour : all_armour)
        armour->set_visibility(use_fog_of_war, f_fow_near(*armour), f_calc_night(*armour));
        
      for (auto& npc : all_npcs)
        npc.set_visibility(use_fog_of_war, f_fow_near(npc), f_calc_night(npc));
        
      for (auto& bs : m_player.blood_splats)
        bs.set_visibility(use_fog_of_war, f_calc_night(bs));
      
      for (auto& npc : all_npcs)
        for (auto& bs : npc.blood_splats)
          bs.set_visibility(use_fog_of_war, f_calc_night(bs));
    }
    
    template<int NR, int NC>
    void draw_health_bars(ScreenHandler<NR, NC>& sh, bool framed_mode)
    {
      std::vector<std::string> health_bars;
      std::vector<Style> styles;
      std::string pc_hb = str::rep_char(' ', 10);
      float pc_ratio = globals::max_health / 10;
      for (int i = 0; i < 10; ++i)
        pc_hb[i] = m_player.health > static_cast<int>(i*pc_ratio) ? '#' : ' ';
      health_bars.emplace_back(pc_hb);
      styles.emplace_back(Style { Color::Magenta, Color::Transparent2 });
      
      for (const auto& npc : all_npcs)
      {
        if (npc.health > 0 && npc.state == State::Fight)
        {
          std::string npc_hb = str::rep_char(' ', 10);
          float npc_ratio = globals::max_health / 10;
          for (int i = 0; i < 10; ++i)
            npc_hb[i] = npc.health > static_cast<int>(i*npc_ratio) ? 'O' : ' ';
          health_bars.emplace_back(npc_hb);
          styles.emplace_back(Style { Color::Red, Color::Transparent2 });
        }
      }
      
      ui::TextBoxDrawingArgsAlign tb_args;
      tb_args.v_align = ui::VerticalAlignment::TOP;
      tb_args.h_align = ui::HorizontalAlignment::LEFT;
      tb_args.base.box_style = { Color::White, Color::DarkBlue };
      tb_args.framed_mode = framed_mode;
      tb_health.set_text(health_bars, styles);
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
    
    void update_fighting(float real_time_s)
    {
      if (m_player.health > 0)
      {
        for (auto& npc : all_npcs)
        {
          if (npc.health > 0 && npc.state == State::Fight)
          {
            auto f_calc_damage = [](const Weapon* weapon, int bonus)
            {
              if (weapon == nullptr)
                return 1 + bonus; // Fists with strength bonus
              return weapon->damage + bonus;
            };
        
            int blind_attack_penalty = (npc.visible ? 0 : 12) + rnd::rand_int(0, 8);
        
            // NPC attack roll.
            int npc_attack_roll = rnd::dice(20) + npc.thac0 + npc.get_melee_attack_bonus() - blind_attack_penalty;
        
            // Calculate the player's total armor class.
            int player_ac = m_player.calc_armour_class(m_inventory.get());
        
            // Determine if NPC hits the player.
            // e.g. d12 + 1 + (2 + 10/2) >= (10 + 10/2).
            // d12 + 8 >= 15.
            if (npc_attack_roll >= player_ac)
            {
              // NPC hits the player
              int damage = 1; // Default damage for fists
              if (npc.weapon_idx != -1)
                damage = f_calc_damage(all_weapons[npc.weapon_idx].get(), npc.get_melee_damage_bonus());
        
              // Apply damage to the player
              bool was_alive = m_player.health > 0;
              m_player.health -= damage;
              if (was_alive && m_player.health <= 0)
              {
                message_handler->add_message(real_time_s,
                                             "You were killed!",
                                             MessageHandler::Level::Fatal);
                broadcast([](auto* listener) { listener->on_pc_death(); });
              }
            }
            
            // Roll a d20 for the player's attack roll (if the NPC is visible).
            // If invisible, then roll a d32 instead.
            const auto* weapon = m_player.get_selected_melee_weapon(m_inventory.get());
            int player_attack_roll = rnd::dice(20) + m_player.thac0 + m_player.get_melee_attack_bonus() - blind_attack_penalty;
            int npc_ac = npc.calc_armour_class();
            
            // Determine if player hits the NPC.
            if (player_attack_roll >= npc_ac)
            {
              // PC hits the NPC.
              int damage = f_calc_damage(weapon, m_player.get_melee_damage_bonus());
              
              // Apply damage to the NPC.
              bool was_alive = npc.health > 0;
              npc.health -= damage;
              if (was_alive && npc.health <= 0)
              {
                message_handler->add_message(real_time_s,
                                             "You killed the " + race2str(npc.npc_race) + "!",
                                             MessageHandler::Level::Guide);
                broadcast([](auto* listener) { listener->on_npc_death(); });
              }
            }
          }
        }
      }
    }
    
    template<int NR, int NC>
    void draw_fighting(ScreenHandler<NR, NC>& sh, const RC& pc_scr_pos, bool do_update_fight, float real_time_s, float sim_time_s)
    {
      if (m_player.health > 0)
      {
        for (auto& npc : all_npcs)
        {
          if (npc.health <= 0)
            continue;
          
          if (npc.state == State::Fight)
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
          
          if (npc.state == State::Fight)
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
              if (do_update_fight && rnd::one_in(npc.visible ? 20 : 28))
              {
                auto& bs = m_player.blood_splats.emplace_back(m_environment.get(), m_player.curr_floor, m_player.pos + offs, rnd::dice(4), sim_time_s, offs);
                bs.curr_room = m_player.curr_room;
                bs.curr_corridor = m_player.curr_corridor;
                if (m_player.is_inside_curr_room())
                  bs.is_underground = m_environment->is_underground(m_player.curr_floor, m_player.curr_room);
                else if (m_player.is_inside_curr_corridor())
                  bs.is_underground = m_environment->is_underground(m_player.curr_floor, m_player.curr_corridor);
              }
            }
            if (npc.visible)
            {
              if (do_update_fight)
                npc.cached_fight_offs = f_calc_fight_offs(-dp);
              auto offs = npc.cached_fight_offs;
              if (m_environment->is_inside_any_room(npc.curr_floor, npc.pos + offs))
              {
                f_render_fight(&npc, pc_scr_pos, offs);
                if (do_update_fight && rnd::one_in(npc.visible ? 20 : 28))
                {
                  auto& bs = npc.blood_splats.emplace_back(m_environment.get(), npc.curr_floor, npc.pos + offs, rnd::dice(4), sim_time_s, offs);
                  bs.curr_room = npc.curr_room;
                  bs.curr_corridor = npc.curr_corridor;
                  bs.is_underground = npc.is_underground;
                }
              }
            }
          }
        }
      }
    }
    
  public:
    DungGine(const std::string& exe_folder, bool use_fow, DungGineTextureParams texture_params = {})
      : message_handler(std::make_unique<MessageHandler>())
      , use_fog_of_war(use_fow)
    {
      m_screen_helper = std::make_unique<ScreenHelper>();
      m_environment = std::make_unique<Environment>();
      m_environment->load_textures(exe_folder, texture_params);
      m_inventory = std::make_unique<Inventory>();
      m_keyboard = std::make_unique<Keyboard>(m_environment.get(), m_inventory.get(), message_handler.get(),
                                              m_player,
                                              all_keys, all_lamps, all_weapons, all_potions, all_armour,
                                              all_npcs,
                                              trigger_game_save, trigger_game_load,
                                              tbd, debug);
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
    
    void style_dungeon()
    {
      m_environment->style_dungeon(m_latitude, m_longitude);
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
            if (rnd_corr->orientation == Orientation::Vertical && rnd_corr->bb.r_len > 2)
              break;
            if (rnd_corr->orientation == Orientation::Horizontal && rnd_corr->bb.c_len > 2)
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
    
    bool place_keys(bool only_place_on_dry_land)
    {
      const int c_max_num_iters = 1e5_i;
      const auto* dungeon = m_environment->get_dungeon();
      for (int f_idx = 0; f_idx < m_environment->num_floors(); ++f_idx)
      {
        auto* bsp_tree = dungeon->get_tree(f_idx);
        const auto world_size = bsp_tree->get_world_size();
        const auto& door_vec = bsp_tree->fetch_doors();
        for (auto* d : door_vec)
        {
          if (d->is_locked)
          {
            Key key;
            key.curr_floor = f_idx;
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
              
              BSPNode* room = nullptr;
              valid_pos = m_environment->is_inside_any_room(bsp_tree, key.pos, &room);
              if (only_place_on_dry_land &&
                  room != nullptr &&
                  !is_dry(m_environment->get_terrain(key.curr_floor, key.pos)))
              {
                valid_pos = false;
              }
            } while (num_iters++ < c_max_num_iters && !valid_pos);
            
            BSPNode* leaf = nullptr;
            if (!m_environment->is_inside_any_room(bsp_tree, key.pos, &leaf))
              return false;
            
            if (leaf != nullptr)
            {
              key.curr_room = leaf;
              key.is_underground = m_environment->is_underground(key.curr_floor, leaf);
            }
            
            all_keys.emplace_back(key);
          }
        }
      }
      return true;
    }
    
    bool place_lamps(int num_torches_per_floor, int num_lanterns_per_floor, int num_magic_lamps_per_floor, bool only_place_on_dry_land)
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
            BSPNode* room = nullptr;
            valid_pos = m_environment->is_inside_any_room(bsp_tree, lamp.pos, &room);
            if (only_place_on_dry_land &&
                room != nullptr &&
                !is_dry(m_environment->get_terrain(lamp.curr_floor, lamp.pos)))
            {
              valid_pos = false;
            }
          } while (num_iters++ < c_max_num_iters && !valid_pos);
          
          BSPNode* leaf = nullptr;
          if (!m_environment->is_inside_any_room(bsp_tree, lamp.pos, &leaf))
            return false;
          
          if (leaf != nullptr)
          {
            lamp.curr_room = leaf;
            lamp.is_underground = m_environment->is_underground(lamp.curr_floor, leaf);
          }
          
          all_lamps.emplace_back(lamp);
        }
      }
      return true;
    }
    
    bool place_weapons(int num_weapons_per_floor, bool only_place_on_dry_land)
    {
      const int c_max_num_iters = 1e5_i;
      const auto* dungeon = m_environment->get_dungeon();
      for (int f_idx = 0; f_idx < m_environment->num_floors(); ++f_idx)
      {
        auto* bsp_tree = dungeon->get_tree(f_idx);
        const auto world_size = bsp_tree->get_world_size();
        for (int wpn_idx = 0; wpn_idx < num_weapons_per_floor; ++wpn_idx)
        {
          std::unique_ptr<Weapon> weapon;
          switch (rnd::rand_int(0, 2))
          {
            case 0: weapon = std::make_unique<Sword>(); break;
            case 1: weapon = std::make_unique<Dagger>(); break;
            case 2: weapon = std::make_unique<Flail>(); break;
              // Error:
            default: return false;
          }
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
            BSPNode* room = nullptr;
            valid_pos = m_environment->is_inside_any_room(bsp_tree, weapon->pos, &room);
            if (only_place_on_dry_land &&
                room != nullptr &&
                !is_dry(m_environment->get_terrain(weapon->curr_floor, weapon->pos)))
            {
              valid_pos = false;
            }
          } while (num_iters++ < c_max_num_iters && !valid_pos);
          
          BSPNode* leaf = nullptr;
          if (!m_environment->is_inside_any_room(bsp_tree, weapon->pos, &leaf))
            return false;
          
          if (leaf != nullptr)
          {
            weapon->curr_room = leaf;
            weapon->is_underground = m_environment->is_underground(weapon->curr_floor, leaf);
          }
          
          all_weapons.emplace_back(weapon.release());
        }
      }
      return true;
    }
    
    bool place_potions(int num_potions_per_floor, bool only_place_on_dry_land)
    {
      const int c_max_num_iters = 1e5_i;
      const auto* dungeon = m_environment->get_dungeon();
      for (int f_idx = 0; f_idx < m_environment->num_floors(); ++f_idx)
      {
        auto* bsp_tree = dungeon->get_tree(f_idx);
        const auto world_size = bsp_tree->get_world_size();
        for (int pot_idx = 0; pot_idx < num_potions_per_floor; ++pot_idx)
        {
          Potion potion;
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
            BSPNode* room = nullptr;
            valid_pos = m_environment->is_inside_any_room(bsp_tree, potion.pos, &room);
            if (only_place_on_dry_land &&
                room != nullptr &&
                !is_dry(m_environment->get_terrain(potion.curr_floor, potion.pos)))
            {
              valid_pos = false;
            }
          } while (num_iters++ < c_max_num_iters && !valid_pos);
          
          BSPNode* leaf = nullptr;
          if (!m_environment->is_inside_any_room(bsp_tree, potion.pos, &leaf))
            return false;
          
          if (leaf != nullptr)
          {
            potion.curr_room = leaf;
            potion.is_underground = m_environment->is_underground(potion.curr_floor, leaf);
          }
          
          all_potions.emplace_back(potion);
        }
      }
      return true;
    }
    
    bool place_armour(int num_armour, bool only_place_on_dry_land)
    {
      const int c_max_num_iters = 1e5_i;
      const auto* dungeon = m_environment->get_dungeon();
      for (int f_idx = 0; f_idx < m_environment->num_floors(); ++f_idx)
      {
        auto* bsp_tree = dungeon->get_tree(f_idx);
        const auto world_size = bsp_tree->get_world_size();
        for (int a_idx = 0; a_idx < num_armour; ++a_idx)
        {
          std::unique_ptr<Armour> armour;
          switch (rnd::rand_int(0, 6))
          {
            case 0: armour = std::make_unique<Shield>(); break;
            case 1: armour = std::make_unique<Gambeson>(); break;
            case 2: armour = std::make_unique<ChainMailleHauberk>(); break;
            case 3: armour = std::make_unique<PlatedBodyArmour>(); break;
            case 4: armour = std::make_unique<PaddedCoif>(); break;
            case 5: armour = std::make_unique<ChainMailleCoif>(); break;
            case 6: armour = std::make_unique<Helmet>(); break;
              // Error:
            default: return false;
          }
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
            BSPNode* room = nullptr;
            valid_pos = m_environment->is_inside_any_room(bsp_tree, armour->pos, &room);
            if (only_place_on_dry_land &&
                room != nullptr &&
                !is_dry(m_environment->get_terrain(armour->curr_floor, armour->pos)))
            {
              valid_pos = false;
            }
          } while (num_iters++ < c_max_num_iters && !valid_pos);
          
          BSPNode* leaf = nullptr;
          if (!m_environment->is_inside_any_room(bsp_tree, armour->pos, &leaf))
            return false;
          
          if (leaf != nullptr)
          {
            armour->curr_room = leaf;
            armour->is_underground = m_environment->is_underground(armour->curr_floor, leaf);
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
            BSPNode* room = nullptr;
            valid_pos = m_environment->is_inside_any_room(bsp_tree, npc.pos, &room);
            if (only_place_on_dry_land &&
                room != nullptr)
            {
              if (!is_dry(m_environment->get_terrain(npc.curr_floor, npc.pos)))
                valid_pos = false;
              else if (!m_environment->allow_move_to(npc.curr_floor, npc.pos.r, npc.pos.c))
                valid_pos = false;
            }
          } while (num_iters++ < c_max_num_iters && !valid_pos);
          
          BSPNode* leaf = nullptr;
          if (!m_environment->is_inside_any_room(bsp_tree, npc.pos, &leaf))
            return false;
          
          if (leaf != nullptr)
          {
            npc.curr_room = leaf;
            npc.is_underground = m_environment->is_underground(npc.curr_floor, leaf);
            npc.init(all_weapons);
          }
          
          all_npcs.emplace_back(npc);
        }
      }
      return true;
    }
    
    void update(int frame_ctr, float fps,
                double real_time_s, float sim_time_s, float sim_dt_s,
                float fire_smoke_dt_factor, 
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
        update_fighting(static_cast<float>(real_time_s));
        
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
        
      if (m_player.show_inventory)
      {
        m_inventory->set_bounding_box({ 2, 2, NR - 5, NC - 5 });
        m_inventory->draw(sh);
      }
        
      draw_health_bars(sh, framed_mode);
      draw_strength_bar(sh, framed_mode);
      
      auto pc_scr_pos = m_screen_helper->get_screen_pos(m_player.pos);
      
      draw_fighting(sh, pc_scr_pos, anim_ctr_fight % 2 == 0, static_cast<float>(real_time_s), sim_time_s);
      
      for (auto& bs : m_player.blood_splats)
        bs.update(sim_time_s);
      for (auto& npc : all_npcs)
        for (auto& bs : npc.blood_splats)
          bs.update(sim_time_s);

      if (debug)
      {
        sh.write_buffer(terrain2str(m_player.on_terrain), 5, 1, Color::Black, Color::White);
        sh.write_buffer("Floor: " + std::to_string(m_player.curr_floor), 6, 1, Color::Black, Color::White);
        
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
