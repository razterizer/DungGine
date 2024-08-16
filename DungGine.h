//
//  DungGine.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-06-13.
//

#pragma once
#include "BSPTree.h"
#include "Environment.h"
#include "ScreenHelper.h"
#include "DungGineStyles.h"
#include "RoomStyle.h"
#include "Items.h"
#include "PC.h"
#include "NPC.h"
#include "SolarMotionPatterns.h"
#include "Globals.h"
#include <Termin8or/Keyboard.h>
#include <Termin8or/MessageHandler.h>
#include <Core/FolderHelper.h>


namespace dung
{

  using namespace std::string_literals;

  class DungGine final
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
    std::vector<int> fight_r_offs = { 1, 0, -1, -1, -1, 0, 1, 1 };
    std::vector<int> fight_c_offs = { 1, 1, 1, 0, -1, -1, -1, 0 };
    
    ui::TextBox tb_health, tb_strength;
    
    // /////////////////////
    
    void update_sun(float real_time_s)
    {
      m_t_solar_period = std::fmod(m_sun_day_t_offs + (real_time_s / 60.f) / m_sun_minutes_per_day, 1);
      m_sun_dir = m_solar_motion.get_solar_direction(m_latitude, m_longitude, m_season, m_t_solar_period);
      
      float t_season_period = std::fmod(m_sun_year_t_offs + (real_time_s / 60.f) / m_sun_minutes_per_year, 1);
      m_season = static_cast<Season>(math::roundI(7*t_season_period));
    }
        
    template<int NR, int NC>
    void draw_inventory(SpriteHandler<NR, NC>& sh) const
    {
      sh.write_buffer(str::adjust_str("Inventory", str::Adjustment::Center, NC - 1), 4, 0, Color::White, Color::Transparent2);
    
      const int x = -5;
      const int y = -2;
      const int z = 4;
      ttl::Rectangle bb_inv { 2, 2, NR + x, NC - 5 };
      static int r_offs = 0;
      const int c_item = 6;
      const int r_min = bb_inv.top() + z;
      const int r_max = bb_inv.bottom() + y;
      int r = r_min + r_offs;
      Style style_category { Color::White, Color::Transparent2 };
      HiliteSelectFGStyle style_item { Color::DarkGreen, Color::Transparent2, Color::Green, Color::DarkBlue, Color::Blue };
      
      auto num_inv_keys = static_cast<int>(m_player.key_idcs.size());
      auto num_inv_lamps = static_cast<int>(m_player.lamp_idcs.size());
      auto num_inv_wpns = static_cast<int>(m_player.weapon_idcs.size());
      auto num_inv_potions = static_cast<int>(m_player.potion_idcs.size());
      auto num_inv_armour = static_cast<int>(m_player.armour_idcs.size());
      
      auto f_format_item_str = [](std::string& item_str, float weight, float price, int hp)
      {
        std::ostringstream oss;
        oss << std::setprecision(1) << std::fixed << weight;
        std::string weight_str = oss.str() + " kg";
        item_str += str::rep_char(' ', 25 - static_cast<int>(item_str.size()) - static_cast<int>(weight_str.size())) + weight_str;
        
        oss.str("");
        oss.clear();
        oss << std::setprecision(2) << std::fixed << price;
        std::string price_str = oss.str() + " FK"; // Fantasy-Kronor.
        item_str += str::rep_char(' ', 42 - static_cast<int>(item_str.size()) - static_cast<int>(price_str.size())) + price_str;
        
        if (hp > 0)
        {
          oss.str("");
          oss.clear();
          oss << std::setprecision(0) << std::fixed << hp;
          std::string hp_str = oss.str() + " hp";
          item_str += str::rep_char(' ', 52 - static_cast<int>(item_str.size()) - static_cast<int>(hp_str.size())) + hp_str;
        }
      };
      
      std::vector<std::pair<std::string, bool>> items;
      items.emplace_back(std::make_pair("Keys:", false));
      for (int inv_key_idx = 0; inv_key_idx < num_inv_keys; ++inv_key_idx)
      {
        auto key_idx = m_player.key_idcs[inv_key_idx];
        const auto& key = all_keys[key_idx];
        std::string item_str = "  Key:" + std::to_string(key.key_id);
        f_format_item_str(item_str, key.weight, key.price, 0);
        items.emplace_back(make_pair(item_str, true));
      }
      items.emplace_back(std::make_pair("", false));
      items.emplace_back(std::make_pair("Lamps:", false));
      for (int inv_lamp_idx = 0; inv_lamp_idx < num_inv_lamps; ++inv_lamp_idx)
      {
        auto lamp_idx = m_player.lamp_idcs[inv_lamp_idx];
        const auto& lamp = all_lamps[lamp_idx];
        auto lamp_type = lamp.get_type_str();
        str::to_upper(lamp_type[0]);
        std::string item_str = "  "s + lamp_type + ":" + std::to_string(lamp_idx);
        f_format_item_str(item_str, lamp.weight, lamp.price, 0);
        items.emplace_back(std::make_pair(item_str, true));
      }
      items.emplace_back(std::make_pair("", false));
      items.emplace_back(std::make_pair("Weapons:", false));
      for (int inv_wpn_idx = 0; inv_wpn_idx < num_inv_wpns; ++inv_wpn_idx)
      {
        auto wpn_idx = m_player.weapon_idcs[inv_wpn_idx];
        const auto& weapon = all_weapons[wpn_idx];
        std::string item_str = "  ";
        if (dynamic_cast<Sword*>(weapon.get()) != nullptr)
          item_str += "Sword:";
        else if (dynamic_cast<Dagger*>(weapon.get()) != nullptr)
          item_str += "Dagger:";
        else if (dynamic_cast<Flail*>(weapon.get()) != nullptr)
          item_str += "Flail:";
        else
          item_str += "<Weapon>";
        item_str += ":";
        item_str += std::to_string(wpn_idx);
        f_format_item_str(item_str, weapon->weight, weapon->price, weapon->damage);
        items.emplace_back(std::make_pair(item_str, true));
      }
      items.emplace_back(std::make_pair("", false));
      items.emplace_back(std::make_pair("Potions:", false));
      for (int inv_pot_idx = 0; inv_pot_idx < num_inv_potions; ++inv_pot_idx)
      {
        auto pot_idx = m_player.potion_idcs[inv_pot_idx];
        const auto& potion = all_potions[pot_idx];
        std::string item_str = "  Potion:" + std::to_string(pot_idx);
        f_format_item_str(item_str, potion.weight, potion.price, 0);
        items.emplace_back(std::make_pair(item_str, true));
      }
      items.emplace_back(std::make_pair("", false));
      items.emplace_back(std::make_pair("Armour:", false));
      for (int inv_a_idx = 0; inv_a_idx < num_inv_armour; ++inv_a_idx)
      {
        auto a_idx = m_player.armour_idcs[inv_a_idx];
        const auto& armour = all_armour[a_idx];
        std::string item_str = "  ";
        if (dynamic_cast<Shield*>(armour.get()) != nullptr)
          item_str += "Shield:";
        else if (dynamic_cast<Gambeson*>(armour.get()) != nullptr)
          item_str += "Gambeson:";
        else if (dynamic_cast<ChainMailleHauberk*>(armour.get()) != nullptr)
          item_str += "C.M. Hauberk:";
        else if (dynamic_cast<PlatedBodyArmour*>(armour.get()) != nullptr)
          item_str += "P. Body Armour:";
        else if (dynamic_cast<PaddedCoif*>(armour.get()) != nullptr)
          item_str += "Padded Coif:";
        else if (dynamic_cast<ChainMailleCoif*>(armour.get()) != nullptr)
          item_str += "C.M. Coif:";
        else if (dynamic_cast<Helmet*>(armour.get()) != nullptr)
          item_str += "Helmet:";
        else
          item_str += "<Armour>";
        item_str += ":";
        item_str += std::to_string(a_idx);
        f_format_item_str(item_str, armour->weight, armour->price, armour->protection);
        items.emplace_back(std::make_pair(item_str, true));
      }
      
      auto num_items = static_cast<int>(items.size());
      int num_non_items = 0;
      for (int item_idx = 0; item_idx < num_items; ++item_idx)
      {
        const auto& i = items[item_idx];
        if (!i.second)
          num_non_items++;
        int adj_item_idx = item_idx - num_non_items;
        if (r_min <= r && r <= r_max)
        {
            Style style = style_category;
            if (i.second)
              style = style_item.get_style(m_player.inv_hilite_idx == adj_item_idx,
                                           stlutils::contains(m_player.inv_select_idcs, adj_item_idx));
            sh.write_buffer(i.first, r, c_item, style);
            if (m_player.inv_hilite_idx == adj_item_idx && m_player.inv_hilite_idx < m_player.last_item_idx())
            {
              if (r == r_min)
                r_offs++;
              else if (r == r_max)
                r_offs--;
            }
        }
        else
        {
          if (m_player.inv_hilite_idx == 0)
            r_offs = 0;
          else if (m_player.inv_hilite_idx == m_player.last_item_idx())
            r_offs = x - m_player.num_items() + NR + y - (z - 1) - num_non_items;
        }
        r++;
      }
      
      drawing::draw_box_outline(sh, bb_inv, drawing::OutlineType::Line, { Color::White, Color::DarkGray });
      drawing::draw_box(sh, bb_inv, { Color::White, Color::DarkGray }, ' ');
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
        
      //for (auto& npc : all_npcs)
      //  *get_field_ptr(&npc) = clear_val;
      
      ttl::Rectangle bb;
      bool_vector* field = nullptr;
      
      if (m_player.curr_corridor != nullptr)
      {
        bb = m_player.curr_corridor->bb;
        field = get_field_ptr(m_player.curr_corridor);
        
        auto* door_0 = m_player.curr_corridor->doors[0];
        auto* door_1 = m_player.curr_corridor->doors[1];
        stlutils::memset(*field, clear_val);
        
        *get_field_ptr(door_0) = clear_val;
        *get_field_ptr(door_1) = clear_val;
      }
      if (m_player.curr_room != nullptr)
      {
        bb = m_player.curr_room->bb_leaf_room;
        field = get_field_ptr(m_player.curr_room);
        stlutils::memset(*field, clear_val);
        
        for (auto* door : m_player.curr_room->doors)
          *get_field_ptr(door) = clear_val;
      }

    }
    
    template<typename Lambda>
    void update_field(const RC& curr_pos, Lambda get_field_ptr, bool set_val, float radius, float angle_deg,
                      Lamp::LampType src_type)
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
            if (src_type == Lamp::LampType::Directional)
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
                
              float curr_angle_rad = std::atan2(-(obj.pos.r - curr_pos.r), obj.pos.c - curr_pos.c);
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
        if (p.r < 0 || p.c < 0 || p.r > bb.r_len || p.c > bb.c_len)
          return;
        // ex:
        // +---+
        // |   |
        // +---+
        // r_len = 2, c_len = 4
        // FOW size = 3*5
        // idx = 0 .. 14
        // r = 2, c = 4 => idx = r * (c_len + 1) + c = 2*5 + 4 = 14.
        int idx = p.r * (size.c + 1) + p.c;
        if (0 <= idx && idx < field->size())
          (*field)[idx] = set_val;
      };
      
      auto update_rect_field = [&]() // #FIXME: FHXFTW
      {
        local_pos = curr_pos - bb.pos();
        size = bb.size();
        
        std::vector<RC> positions;
        switch (src_type)
        {
          case Lamp::LampType::Isotropic:
            positions = drawing::filled_circle_positions(local_pos, radius, globals::px_aspect);
            break;
          case Lamp::LampType::Directional:
            positions = drawing::filled_arc_positions(local_pos, radius, math::deg2rad(angle_deg), m_player.los_r, m_player.los_c, globals::px_aspect);
            break;
          case Lamp::LampType::NUM_ITEMS:
            break;
        }
        for (const auto& pos : positions)
          set_field(pos);
        
        int r_room = -1;
        int c_room = -1;
        if (curr_pos.r - bb.top() <= 1)
          r_room = 0;
        else if (bb.bottom() - curr_pos.r <= 1)
          r_room = bb.r_len;
        if (curr_pos.c - bb.left() <= 1)
          c_room = 0;
        else if (bb.right() - curr_pos.c <= 1)
          c_room = bb.c_len;
        
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
      }
    }
    
    void handle_keys(const keyboard::KeyPressData& kpd, double real_time_s)
    {
      auto curr_key = keyboard::get_char_key(kpd);
      auto curr_special_key = keyboard::get_special_key(kpd);
    
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
                 m_environment->allow_move_to(curr_pos.r, curr_pos.c - 1))
          curr_pos.c--;
      }
      else if (str::to_lower(curr_key) == 'd' || curr_special_key == keyboard::SpecialKey::Right)
      {
        if (m_player.show_inventory)
        {
          auto f_drop_item = [&](auto& obj, int obj_idx, auto& player_obj_idcs)
          {
            obj.picked_up = false;
            obj.pos = curr_pos;
            if (m_player.is_inside_curr_room())
            {
              obj.is_underground = m_environment->is_underground(m_player.curr_room);
              obj.curr_room = m_player.curr_room;
              obj.curr_corridor = nullptr;
            }
            else if (m_player.is_inside_curr_corridor())
            {
              obj.is_underground = m_environment->is_underground(m_player.curr_corridor);
              obj.curr_room = nullptr;
              obj.curr_corridor = m_player.curr_corridor;
            }
            stlutils::erase(player_obj_idcs, obj_idx);
          };
          
          std::string msg = "You dropped an item: ";
          if (m_player.in_keys_range(m_player.inv_select_idx_key))
          {
            auto key_idx = m_player.key_idcs[m_player.inv_select_idx_key - m_player.start_inv_idx_keys()];
            auto& key = all_keys[key_idx];
            f_drop_item(key, key_idx, m_player.key_idcs);
            msg += "key:" + std::to_string(key.key_id) + "!";
            stlutils::erase(m_player.inv_select_idcs, m_player.inv_select_idx_key);
            m_player.inv_select_idx_key = -1;
          }
          else if (m_player.in_lamps_range(m_player.inv_select_idx_lamp))
          {
            auto lamp_idx = m_player.lamp_idcs[m_player.inv_select_idx_lamp - m_player.start_inv_idx_lamps()];
            auto& lamp = all_lamps[lamp_idx];
            f_drop_item(lamp, lamp_idx, m_player.lamp_idcs);
            msg += "lamp:" + std::to_string(lamp_idx) + "!";
            stlutils::erase(m_player.inv_select_idcs, m_player.inv_select_idx_lamp);
            m_player.inv_select_idx_lamp = -1;
          }
          else if (m_player.in_weapons_range(m_player.inv_select_idx_weapon))
          {
            auto wpn_idx = m_player.weapon_idcs[m_player.inv_select_idx_weapon - m_player.start_inv_idx_weapons()];
            auto& weapon = *all_weapons[wpn_idx];
            f_drop_item(weapon, wpn_idx, m_player.weapon_idcs);
            msg += weapon.type +":" + std::to_string(wpn_idx) + "!";
            stlutils::erase(m_player.inv_select_idcs, m_player.inv_select_idx_weapon);
            m_player.inv_select_idx_weapon = -1;
          }
          else if (m_player.in_potions_range(m_player.inv_select_idx_potion))
          {
            auto pot_idx = m_player.potion_idcs[m_player.inv_select_idx_potion - m_player.start_inv_idx_potions()];
            auto& potion = all_potions[pot_idx];
            f_drop_item(potion, pot_idx, m_player.potion_idcs);
            msg += "potion:" + std::to_string(pot_idx) + "!";
            stlutils::erase(m_player.inv_select_idcs, m_player.inv_select_idx_potion);
            m_player.inv_select_idx_potion = -1;
          }
          else if (m_player.in_armour_range(m_player.inv_select_idx_armour))
          {
            auto a_idx = m_player.armour_idcs[m_player.inv_select_idx_armour - m_player.start_inv_idx_armour()];
            auto& armour = *all_armour[a_idx];
            f_drop_item(armour, a_idx, m_player.armour_idcs);
            msg += armour.type +":" + std::to_string(a_idx) + "!";
            stlutils::erase(m_player.inv_select_idcs, m_player.inv_select_idx_armour);
            m_player.inv_select_idx_armour = -1;
          }
          else
          {
            msg += "Invalid Item!";
            std::cerr << "ERROR: Attempted to drop invalid item!" << std::endl;
          }
          message_handler->add_message(static_cast<float>(real_time_s),
                                       msg,
                                       MessageHandler::Level::Guide);
        }
        else if (is_inside_curr_bb(curr_pos.r, curr_pos.c + 1) &&
                 m_player.allow_move() &&
                 m_environment->allow_move_to(curr_pos.r, curr_pos.c + 1))
          curr_pos.c++;
      }
      else if (str::to_lower(curr_key) == 's' || curr_special_key == keyboard::SpecialKey::Down)
      {
        if (m_player.show_inventory)
        {
          m_player.inv_hilite_idx++;
          m_player.inv_hilite_idx = m_player.inv_hilite_idx % m_player.num_items();
        }
        else if (is_inside_curr_bb(curr_pos.r + 1, curr_pos.c) &&
                 m_player.allow_move() &&
                 m_environment->allow_move_to(curr_pos.r + 1, curr_pos.c))
          curr_pos.r++;
      }
      else if (str::to_lower(curr_key) == 'w' || curr_special_key == keyboard::SpecialKey::Up)
      {
        if (m_player.show_inventory)
        {
          m_player.inv_hilite_idx--;
          if (m_player.inv_hilite_idx < 0)
            m_player.inv_hilite_idx = m_player.num_items() - 1;
        }
        else if (is_inside_curr_bb(curr_pos.r - 1, curr_pos.c) &&
                 m_player.allow_move() &&
                 m_environment->allow_move_to(curr_pos.r - 1, curr_pos.c))
          curr_pos.r--;
      }
      else if (curr_key == ' ')
      {
        if (m_player.show_inventory)
        {
          const int hilite_idx = m_player.inv_hilite_idx;
          if (stlutils::contains(m_player.inv_select_idcs, hilite_idx))
          {
            stlutils::erase(m_player.inv_select_idcs, hilite_idx);
            if (m_player.in_keys_range(hilite_idx))
              m_player.inv_select_idx_key = -1;
            else if (m_player.in_lamps_range(hilite_idx))
              m_player.inv_select_idx_lamp = -1;
            else if (m_player.in_weapons_range(hilite_idx))
              m_player.inv_select_idx_weapon = -1;
            else if (m_player.in_potions_range(hilite_idx))
              m_player.inv_select_idx_potion = -1;
          }
          else
          {
            m_player.inv_select_idcs.emplace_back(hilite_idx);
            if (m_player.in_keys_range(hilite_idx))
              m_player.inv_select_idx_key = hilite_idx;
            else if (m_player.in_lamps_range(hilite_idx))
              m_player.inv_select_idx_lamp = hilite_idx;
            else if (m_player.in_weapons_range(hilite_idx))
              m_player.inv_select_idx_weapon = hilite_idx;
            else if (m_player.in_potions_range(hilite_idx))
              m_player.inv_select_idx_potion = hilite_idx;
          }
        }
        else
        {
          message_handler->clear_curr_message();
        
          auto f_alter_door_states = [&](Door* door)
          {
            if (door != nullptr && door->is_door && distance(curr_pos, door->pos) == 1.f)
            {
              if (door->is_locked)
              {
                if (m_player.using_key_id(all_keys, door->key_id))
                {
                  // Currently doesn't support locking the door again.
                  // Not sure if we need that. Maybe do it in the far future...
                  door->is_locked = false;
                  
                  message_handler->add_message(static_cast<float>(real_time_s),
                                               "The door is unlocked!",
                                               MessageHandler::Level::Guide);
                  
                  m_player.remove_key_by_key_id(all_keys, door->key_id);
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
          }
          
          for (size_t key_idx = 0; key_idx < all_keys.size(); ++key_idx)
          {
            auto& key = all_keys[key_idx];
            if (key.pos == curr_pos && !key.picked_up)
            {
              for (int& sel_idx : m_player.inv_select_idcs)
                if (m_player.start_inv_idx_lamps() <= sel_idx)
                  sel_idx++;
              if (m_player.inv_select_idx_lamp != -1)
                m_player.inv_select_idx_lamp++;
              if (m_player.inv_select_idx_weapon != -1)
                m_player.inv_select_idx_weapon++;
              if (m_player.inv_select_idx_potion != -1)
                m_player.inv_select_idx_potion++;
              if (m_player.inv_select_idx_armour != -1)
                m_player.inv_select_idx_armour++;
              if (m_player.start_inv_idx_lamps() <= m_player.inv_hilite_idx)
                m_player.inv_hilite_idx++;
                
              m_player.key_idcs.emplace_back(key_idx);
              key.picked_up = true;
              message_handler->add_message(static_cast<float>(real_time_s),
                                           "You picked up a key!", MessageHandler::Level::Guide);
            }
          }
          for (size_t lamp_idx = 0; lamp_idx < all_lamps.size(); ++lamp_idx)
          {
            auto& lamp = all_lamps[lamp_idx];
            if (lamp.pos == curr_pos && !lamp.picked_up)
            {
              for (int& sel_idx : m_player.inv_select_idcs)
                if (m_player.start_inv_idx_weapons() <= sel_idx)
                  sel_idx++;
              if (m_player.inv_select_idx_weapon != -1)
                m_player.inv_select_idx_weapon++;
              if (m_player.inv_select_idx_potion != -1)
                m_player.inv_select_idx_potion++;
              if (m_player.inv_select_idx_armour != -1)
                m_player.inv_select_idx_armour++;
              if (m_player.start_inv_idx_weapons() <= m_player.inv_hilite_idx)
                m_player.inv_hilite_idx++;
                
              m_player.lamp_idcs.emplace_back(lamp_idx);
              lamp.picked_up = true;
              auto lamp_type = lamp.get_type_str();
              message_handler->add_message(static_cast<float>(real_time_s),
                                           "You picked up a"s + (str::is_an(lamp_type) ? "n ":" ") + lamp_type + "!", MessageHandler::Level::Guide);
            }
          }
          for (size_t wpn_idx = 0; wpn_idx < all_weapons.size(); ++wpn_idx)
          {
            auto& weapon = all_weapons[wpn_idx];
            if (weapon->pos == curr_pos && !weapon->picked_up)
            {
              for (int& sel_idx : m_player.inv_select_idcs)
                if (m_player.start_inv_idx_potions() <= sel_idx)
                  sel_idx++;
              if (m_player.inv_select_idx_potion != -1)
                m_player.inv_select_idx_potion++;
              if (m_player.inv_select_idx_armour != -1)
                m_player.inv_select_idx_armour++;
              if (m_player.start_inv_idx_potions() <= m_player.inv_hilite_idx)
                m_player.inv_hilite_idx++;
                
              m_player.weapon_idcs.emplace_back(wpn_idx);
              weapon->picked_up = true;
              message_handler->add_message(static_cast<float>(real_time_s),
                                           "You picked up a " + weapon->type + "!", MessageHandler::Level::Guide);
            }
          }
          for (size_t pot_idx = 0; pot_idx < all_potions.size(); ++pot_idx)
          {
            auto& potion = all_potions[pot_idx];
            if (potion.pos == curr_pos && !potion.picked_up)
            {
              for (int& sel_idx : m_player.inv_select_idcs)
                if (m_player.start_inv_idx_armour() <= sel_idx)
                  sel_idx++;
              if (m_player.inv_select_idx_armour != -1)
                m_player.inv_select_idx_armour++;
              if (m_player.start_inv_idx_armour() <= m_player.inv_hilite_idx)
                m_player.inv_hilite_idx++;
              m_player.potion_idcs.emplace_back(pot_idx);
              potion.picked_up = true;
              message_handler->add_message(static_cast<float>(real_time_s),
                                           "You picked up a potion!", MessageHandler::Level::Guide);
            }
          }
          for (size_t a_idx = 0; a_idx < all_armour.size(); ++a_idx)
          {
            auto& armour = all_armour[a_idx];
            if (armour->pos == curr_pos && !armour->picked_up)
            {
              m_player.armour_idcs.emplace_back(a_idx);
              armour->picked_up = true;
              message_handler->add_message(static_cast<float>(real_time_s),
                                           "You picked up a " + armour->type + "!",
                                           MessageHandler::Level::Guide);
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
        for (auto& npc : all_npcs)
          math::toggle(npc.debug);
      }
      else if (curr_key == '?')
        math::toggle(debug);
      else if (str::to_lower(curr_key) == 'i')
      {
        for (const auto& key : all_keys)
        {
          if (key.visible_near)
            message_handler->add_message(static_cast<float>(real_time_s),
                                         "You see a key nearby!", MessageHandler::Level::Guide);
        }
        for (const auto& lamp : all_lamps)
        {
          if (lamp.visible_near)
          {
            auto lamp_type = lamp.get_type_str();
            message_handler->add_message(static_cast<float>(real_time_s),
                                         "You see a"s + (str::is_an(lamp_type) ? "n ":" ") + lamp_type + " nearby!", MessageHandler::Level::Guide);
          }
        }
        for (const auto& weapon : all_weapons)
        {
          if (weapon->visible_near)
            message_handler->add_message(static_cast<float>(real_time_s),
                                         "You can see a " + weapon->type + " nearby!", MessageHandler::Level::Guide);
        }
        for (const auto& potion : all_potions)
        {
          if (potion.visible_near)
            message_handler->add_message(static_cast<float>(real_time_s),
                                         "You can see a potion nearby!", MessageHandler::Level::Guide);
        }
        for (const auto& armour : all_armour)
        {
          if (armour->visible_near)
            message_handler->add_message(static_cast<float>(real_time_s),
                                         "You can see a " + armour->type + " nearby!", MessageHandler::Level::Guide);
        }
        for (const auto& npc : all_npcs)
        {
          if (npc.visible_near)
            message_handler->add_message(static_cast<float>(real_time_s),
                                         "You can see a " + race2str(npc.npc_race) + " nearby!", MessageHandler::Level::Guide);
        }
      }
      else if (str::to_lower(curr_key) == 'c')
      {
        auto* potion = m_player.get_selected_potion(all_potions);
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
          m_player.remove_selected_potion(all_potions);
          message_handler->add_message(static_cast<float>(real_time_s),
                                       "You throw away the empty vial.",
                                       MessageHandler::Level::Guide);
        }
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
          
          auto room_style = m_environment->find_room_style(obj.curr_room);
          if (room_style.has_value())
            f_set_night(room_style.value());
          else
          {
            auto corr_style = m_environment->find_corridor_style(obj.curr_corridor);
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
    }
    
    template<int NR, int NC>
    void draw_health_bars(SpriteHandler<NR, NC>& sh)
    {
      std::vector<std::string> health_bars;
      std::vector<Style> styles;
      std::string pc_hb = str::rep_char(' ', 10);
      float pc_ratio = globals::max_health / 10;
      for (int i = 0; i < 10; ++i)
        pc_hb[i] = m_player.health > static_cast<int>(i*pc_ratio) ? '#' : ' ';
      health_bars.emplace_back(pc_hb);
      styles.emplace_back(Style { Color::Cyan, Color::Transparent2 });
      
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
      
      tb_health.set_text(health_bars, styles);
      tb_health.calc_pre_draw(str::Adjustment::Left);
      tb_health.draw(sh, ui::VerticalAlignment::TOP, ui::HorizontalAlignment::LEFT, styles::Style { Color::White, Color::DarkBlue }, true, true, 0, 0, std::nullopt, drawing::OutlineType::Line, false);
    }
    
    template<int NR, int NC>
    void draw_strength_bar(SpriteHandler<NR, NC>& sh)
    {
      std::string strength_bar = str::rep_char(' ', 10);
      float pc_ratio = m_player.strength / 10;
      for (int i = 0; i < 10; ++i)
        strength_bar[i] = (m_player.strength - m_player.weakness) > static_cast<int>(i*pc_ratio)
        ? '=' : ' ';
      Style style { Color::Green, Color::Transparent2 };
      tb_strength.set_text(strength_bar, style);
      tb_strength.calc_pre_draw(str::Adjustment::Left);
      tb_strength.draw(sh, { 1, 12 }, { Color::White, Color::DarkBlue }, true, true, 0, 0, std::nullopt, drawing::OutlineType::Line);
    }
    
  public:
    DungGine(const std::string& exe_folder, bool use_fow, DungGineTextureParams texture_params = {})
      : message_handler(std::make_unique<MessageHandler>())
      , use_fog_of_war(use_fow)
    {
      m_screen_helper = std::make_unique<ScreenHelper>();
      m_environment = std::make_unique<Environment>();
      m_environment->load_textures(exe_folder, texture_params);
    }
    
    void load_dungeon(BSPTree* bsp_tree)
    {
      m_environment->load_dungeon(bsp_tree);
    }
    
    void style_dungeon()
    {
      m_environment->style_dungeon(m_latitude, m_longitude);
    }
    
    void set_player_character(char ch) { m_player.character = ch; }
    void set_player_style(const Style& style) { m_player.style = style; }
    bool place_player(const RC& screen_size, std::optional<RC> world_pos = std::nullopt)
    {
      const auto world_size = m_environment->get_world_size();
      m_screen_helper->set_screen_size(screen_size);
    
      if (world_pos.has_value())
        m_player.pos = world_pos.value();
      else
        m_player.pos = world_size / 2;
        
      m_player.last_pos = m_player.pos;
      
      const auto& room_corridor_map = m_environment->get_room_corridor_map();
      
      const int c_max_num_iters = 1e5;
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
      const auto world_size = m_environment->get_world_size();
      const auto& door_vec = m_environment->fetch_doors();
      const int c_max_num_iters = 1e5;
      int num_iters = 0;
      bool valid_pos = false;
      for (auto* d : door_vec)
      {
        if (d->is_locked)
        {
          Key key;
          key.key_id = d->key_id;
          do
          {
            key.pos =
            {
              rnd::rand_int(0, world_size.r),
              rnd::rand_int(0, world_size.c)
            };
            
            BSPNode* room = nullptr;
            valid_pos = m_environment->is_inside_any_room(key.pos, &room);
            if (only_place_on_dry_land &&
                room != nullptr &&
                !is_dry(m_environment->get_terrain(key.pos)))
            {
              valid_pos = false;
            }
          } while (num_iters++ < c_max_num_iters && !valid_pos);
          
          BSPNode* leaf = nullptr;
          if (!m_environment->is_inside_any_room(key.pos, &leaf))
            return false;
            
          if (leaf != nullptr)
          {
            key.curr_room = leaf;
            key.is_underground = m_environment->is_underground(leaf);
          }
            
          all_keys.emplace_back(key);
        }
      }
      return true;
    }
    
    bool place_lamps(int num_lamps, bool only_place_on_dry_land)
    {
      const auto world_size = m_environment->get_world_size();
      const int c_max_num_iters = 1e5;
      int num_iters = 0;
      bool valid_pos = false;
      for (int lamp_idx = 0; lamp_idx < num_lamps; ++lamp_idx)
      {
        Lamp lamp;
        lamp.type = rnd::rand_enum<Lamp::LampType>();
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
          valid_pos = m_environment->is_inside_any_room(lamp.pos, &room);
          if (only_place_on_dry_land &&
              room != nullptr &&
              !is_dry(m_environment->get_terrain(lamp.pos)))
          {
            valid_pos = false;
          }
        } while (num_iters++ < c_max_num_iters && !valid_pos);
        
        BSPNode* leaf = nullptr;
        if (!m_environment->is_inside_any_room(lamp.pos, &leaf))
          return false;
          
        if (leaf != nullptr)
        {
          lamp.curr_room = leaf;
          lamp.is_underground = m_environment->is_underground(leaf);
        }
        
        all_lamps.emplace_back(lamp);
      }
      return true;
    }
    
    bool place_weapons(int num_weapons, bool only_place_on_dry_land)
    {
      const auto world_size = m_environment->get_world_size();
      const int c_max_num_iters = 1e5;
      int num_iters = 0;
      bool valid_pos = false;
      for (int wpn_idx = 0; wpn_idx < num_weapons; ++wpn_idx)
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
        do
        {
          weapon->pos =
          {
            rnd::rand_int(0, world_size.r),
            rnd::rand_int(0, world_size.c)
          };
          BSPNode* room = nullptr;
          valid_pos = m_environment->is_inside_any_room(weapon->pos, &room);
          if (only_place_on_dry_land &&
              room != nullptr &&
              !is_dry(m_environment->get_terrain(weapon->pos)))
          {
            valid_pos = false;
          }
        } while (num_iters++ < c_max_num_iters && !valid_pos);
        
        BSPNode* leaf = nullptr;
        if (!m_environment->is_inside_any_room(weapon->pos, &leaf))
          return false;
          
        if (leaf != nullptr)
        {
          weapon->curr_room = leaf;
          weapon->is_underground = m_environment->is_underground(leaf);
        }
        
        all_weapons.emplace_back(weapon.release());
      }
      return true;
    }
    
    bool place_potions(int num_potions, bool only_place_on_dry_land)
    {
      const auto world_size = m_environment->get_world_size();
      const int c_max_num_iters = 1e5;
      int num_iters = 0;
      bool valid_pos = false;
      for (int pot_idx = 0; pot_idx < num_potions; ++pot_idx)
      {
        Potion potion;
        do
        {
          if (num_iters < 50)
          {
            potion.pos =
            {
              rnd::rand_int(0, world_size.r),
              rnd::rand_int(0, world_size.c)
            };
          }
          BSPNode* room = nullptr;
          valid_pos = m_environment->is_inside_any_room(potion.pos, &room);
          if (only_place_on_dry_land &&
              room != nullptr &&
              !is_dry(m_environment->get_terrain(potion.pos)))
          {
            valid_pos = false;
          }
        } while (num_iters++ < c_max_num_iters && !valid_pos);
        
        BSPNode* leaf = nullptr;
        if (!m_environment->is_inside_any_room(potion.pos, &leaf))
          return false;
          
        if (leaf != nullptr)
        {
          potion.curr_room = leaf;
          potion.is_underground = m_environment->is_underground(leaf);
        }
        
        all_potions.emplace_back(potion);
      }
      return true;
    }
    
    bool place_armour(int num_armour, bool only_place_on_dry_land)
    {
      const auto world_size = m_environment->get_world_size();
      const int c_max_num_iters = 1e5;
      int num_iters = 0;
      bool valid_pos = false;
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
        do
        {
          armour->pos =
          {
            rnd::rand_int(0, world_size.r),
            rnd::rand_int(0, world_size.c)
          };
          BSPNode* room = nullptr;
          valid_pos = m_environment->is_inside_any_room(armour->pos, &room);
          if (only_place_on_dry_land &&
              room != nullptr &&
              !is_dry(m_environment->get_terrain(armour->pos)))
          {
            valid_pos = false;
          }
        } while (num_iters++ < c_max_num_iters && !valid_pos);
        
        BSPNode* leaf = nullptr;
        if (!m_environment->is_inside_any_room(armour->pos, &leaf))
          return false;
          
        if (leaf != nullptr)
        {
          armour->curr_room = leaf;
          armour->is_underground = m_environment->is_underground(leaf);
        }
        
        all_armour.emplace_back(armour.release());
      }
      return true;
    }
    
    bool place_npcs(int num_npcs, bool only_place_on_dry_land)
    {
      const auto world_size = m_environment->get_world_size();
      const int c_max_num_iters = 1e5;
      int num_iters = 0;
      bool valid_pos = false;
      for (int npc_idx = 0; npc_idx < num_npcs; ++npc_idx)
      {
        NPC npc;
        npc.npc_class = rnd::rand_enum<Class>();
        npc.npc_race = rnd::rand_enum<Race>();
        do
        {
          npc.pos =
          {
            rnd::rand_int(0, world_size.r),
            rnd::rand_int(0, world_size.c)
          };
          BSPNode* room = nullptr;
          valid_pos = m_environment->is_inside_any_room(npc.pos, &room);
          if (only_place_on_dry_land &&
              room != nullptr)
          {
            if (!is_dry(m_environment->get_terrain(npc.pos)))
              valid_pos = false;
            else if (!m_environment->allow_move_to(npc.pos.r, npc.pos.c))
              valid_pos = false;
          }
        } while (num_iters++ < c_max_num_iters && !valid_pos);
        
        BSPNode* leaf = nullptr;
        if (!m_environment->is_inside_any_room(npc.pos, &leaf))
          return false;
          
        if (leaf != nullptr)
        {
          npc.curr_room = leaf;
          npc.is_underground = m_environment->is_underground(leaf);
          npc.init(all_weapons);
        }
        
        all_npcs.emplace_back(npc);
      }
      return true;
    }
    
    void update(double real_time_s, float sim_dt_s, const keyboard::KeyPressData& kpd, bool* game_over)
    {
      utils::try_set(game_over, m_player.health <= 0);
      if (utils::try_get(game_over))
        return;
    
      update_sun(static_cast<float>(real_time_s));
      
      auto fow_radius = 5.5f;
      auto* lamp = m_player.get_selected_lamp(all_lamps);
      if (lamp != nullptr)
      {
        math::maximize(fow_radius, lamp->radius);
        math::minimize(fow_radius, globals::max_fow_radius);
      }
      
      set_visibilities(fow_radius, m_player.pos);
      
      auto& curr_pos = m_player.pos;
      handle_keys(kpd, real_time_s);
      
      // Fog of war
      if (use_fog_of_war)
        update_field(curr_pos,
                     [](auto obj) { return &obj->fog_of_war; },
                     false, fow_radius, 0.f, Lamp::LampType::Isotropic);
                  
      // Light
      clear_field([](auto obj) { return &obj->light; }, false);
      if (lamp != nullptr)
      {
        update_field(curr_pos,
                     [](auto obj) { return &obj->light; },
                     true, lamp->radius, lamp->angle_deg,
                     lamp->type);
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
      m_player.on_terrain = m_environment->get_terrain(m_player.pos);
      m_player.update();
      if (was_alive && m_player.health <= 0)
      {
        message_handler->add_message(static_cast<float>(real_time_s),
                                     "You died!",
                                     MessageHandler::Level::Fatal);
      }
      
      // NPCs
      for (auto& npc : all_npcs)
      {
        npc.on_terrain = m_environment->get_terrain(npc.pos);
        npc.update(curr_pos, m_environment.get(), sim_dt_s);
      }
      
      // Fighting
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
        
            // NPC attack roll
            int npc_attack_roll = rnd::dice(20) + npc.thac0 + npc.get_melee_attack_bonus();
        
            // Calculate the player's total armor class
            int player_ac = m_player.calc_armour_class(all_armour);
        
            // Determine if NPC hits the player
            // e.g. d12 + 1 + (2 + 10/2) >= (10 + 10/2)
            // d12 + 8 >= 15
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
                message_handler->add_message(static_cast<float>(real_time_s),
                                             "You were killed!",
                                             MessageHandler::Level::Fatal);
              }
            }
            
            // Roll a d20 for the player's attack roll (if the NPC is visible)
            if (npc.visible)
            {
              const auto* weapon = m_player.get_selected_weapon(all_weapons);
              int player_attack_roll = rnd::dice(20) + m_player.thac0 + m_player.get_melee_attack_bonus();
              int npc_ac = npc.calc_armour_class();
        
              // Determine if player hits the NPC
              if (player_attack_roll >= npc_ac)
              {
                // PC hits the NPC
                int damage = f_calc_damage(weapon, m_player.get_melee_damage_bonus());
        
                // Apply damage to the NPC
                bool was_alive = npc.health > 0;
                npc.health -= damage;
                if (was_alive && npc.health <= 0)
                {
                  message_handler->add_message(static_cast<float>(real_time_s),
                                               "You killed the " + race2str(npc.npc_race) + "!",
                                               MessageHandler::Level::Guide);
                }
              }
            }
          }
        }
      }
      
      m_screen_helper->update_scrolling(curr_pos);
    }
    
    
    template<int NR, int NC>
    void draw(SpriteHandler<NR, NC>& sh, double real_time_s, int anim_ctr)
    {
      const auto& room_corridor_map = m_environment->get_room_corridor_map();
      const auto& door_vec = m_environment->fetch_doors();
      
      message_handler->update(sh, static_cast<float>(real_time_s));
      
      if (m_player.show_inventory)
        draw_inventory(sh);
        
      draw_health_bars(sh);
      draw_strength_bar(sh);
      
      auto pc_scr_pos = m_screen_helper->get_screen_pos(m_player.pos);
      
      // Fighting
      if (m_player.health > 0)
      {
        for (auto& npc : all_npcs)
        {
          if (npc.health <= 0)
            continue;
          
          if (npc.is_hostile)
          {
            if (npc.trg_info_hostile_npc.once())
            {
              std::string message = "You are being attacked";
              std::string race = race2str(npc.npc_race);
              if (npc.visible && !race.empty())
                message += " by a"s + (str::is_an(race) ? "n " : " ") + race;
              message += "!";
              message_handler->add_message(static_cast<float>(real_time_s),
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
            
            auto f_render_fight = [&](const RC& scr_pos, const RC& dp)
            {
              styles::Style fight_style
              {
                color::get_random_color(c_fight_colors),
                Color::Transparent2
              };
              std::string fight_str = rnd::rand_select(c_fight_strings);
              auto dir = static_cast<int>(f_dp_to_dir(dp));
              
              auto r_offs = rnd::randn_select(0.f, 1.f, std::vector {
                fight_r_offs[(dir - 1)%num_dir],
                fight_r_offs[dir],
                fight_r_offs[(dir + 1)%num_dir] });
              auto c_offs = rnd::randn_select(0.f, 1.f, std::vector {
                fight_c_offs[(dir - 1)%num_dir],
                fight_c_offs[dir],
                fight_c_offs[(dir + 1)%num_dir] });
              sh.write_buffer(fight_str,
                              scr_pos.r + r_offs,
                              scr_pos.c + c_offs,
                              fight_style);
            };
            f_render_fight(npc_scr_pos, dp);
            if (npc.visible)
              f_render_fight(pc_scr_pos, -dp);
          }
        }
      }

      if (debug)
        sh.write_buffer(terrain2str(m_player.on_terrain), 5, 1, Color::Black, Color::White);
      
      // PC
      if (m_player.is_spawned)
      {
        sh.write_buffer(std::string(1, m_player.character), pc_scr_pos.r, pc_scr_pos.c, m_player.style);
        
        if (is_wet(m_player.on_terrain))
          if (anim_ctr % 3 == 0)
            sh.write_buffer("*", math::roundI(pc_scr_pos.r - m_player.los_r), math::roundI(pc_scr_pos.c - m_player.los_c), Color::White, Color::Transparent2);
      }
      
      // Items and NPCs
      auto f_render_item = [&](const auto& obj)
      {
        if (!obj.visible)
          return;
        auto scr_pos = m_screen_helper->get_screen_pos(obj.pos);
        sh.write_buffer(std::string(1, obj.character), scr_pos.r, scr_pos.c, obj.style);
      };
      
      for (const auto& npc : all_npcs)
      {
        f_render_item(npc);
        
        if (npc.visible && is_wet(npc.on_terrain) && npc.can_swim && !npc.can_fly)
        {
          auto npc_scr_pos = m_screen_helper->get_screen_pos(npc.pos);
          if (anim_ctr % 3 == 0)
            sh.write_buffer("*", math::roundI(npc_scr_pos.r - npc.los_r), math::roundI(npc_scr_pos.c - npc.los_c), Color::White, Color::Transparent2);
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
            bresenham::plot_line(sh, scr_pos.c, scr_pos.r, scr_pos_room.c, scr_pos_room.r,
                    ".", Color::White, Color::Transparent2);
          }
          if (npc.curr_corridor != nullptr)
          {
            auto scr_pos_corr = m_screen_helper->get_screen_pos(npc.curr_corridor->bb.center());
            bresenham::plot_line(sh, scr_pos.c, scr_pos.r, scr_pos_corr.c, scr_pos_corr.r,
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
      
      m_environment->draw_environment(sh, real_time_s,
                                      use_fog_of_war,
                                      m_sun_dir, m_solar_motion,
                                      m_t_solar_period, m_season,
                                      m_use_per_room_lat_long_for_sun_dir,
                                      m_screen_helper.get());
    }
    
  };
  
}
