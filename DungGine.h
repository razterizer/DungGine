//
//  DungGine.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-06-13.
//

#pragma once
#include "BSPTree.h"
#include "DungGineStyles.h"
#include "RoomStyle.h"
#include "Items.h"
#include "Player.h"
#include <Termin8or/Keyboard.h>
#include <Termin8or/MessageHandler.h>


namespace dung
{

  enum class ScreenScrollingMode { AlwaysInCentre, PageWise, WhenOutsideScreen };

  class DungGine
  {
    BSPTree* m_bsp_tree;
    std::vector<BSPNode*> m_leaves;
    
    std::map<BSPNode*, RoomStyle> m_room_styles;
    
    Direction m_sun_dir = Direction::E;
    Direction m_shadow_dir = Direction::W;
    float m_sun_minutes_per_day = 20.f;
    float m_sun_t_offs = 0.f;
    
    Player m_player;
    ttl::Rectangle m_screen_in_world;
    // Value between 0 and 1 where 1 means a full screen vertically or horizontally.
    // Fraction of screen that will be scrolled (when in PageWise scroll mode).
    float t_scroll_amount = 0.2f;
    ScreenScrollingMode scr_scrolling_mode = ScreenScrollingMode::AlwaysInCentre;
    
    // (0,0) world pos
    // +--------------------+
    // | (5,8) scr world pos|
    // |    +-------+       |
    // |    |       |       |
    // |    |    @  |       |  <---- (8, 20) player world pos
    // |    +-------+       |
    // |                    |
    // |                    |
    // +--------------------+
    
    std::vector<Key> all_keys;
    // Lamps illuminate items and NPCs. If you've already discovered an item or
    //   NPC using a lamp (and after FOW been cleared),
    //   then they will still be visible when the room is not lit.
    // Lamps will not work in surface level rooms.
    std::vector<Lamp> all_lamps;
    
    std::unique_ptr<MessageHandler> message_handler;
    bool use_fog_of_war = false;
    
    RC get_screen_pos(const RC& world_pos) const
    {
      return world_pos - m_screen_in_world.pos();
    }
    
    void update_sun(float sim_time_s)
    {
      float t_solar_period = std::fmod(m_sun_t_offs + (sim_time_s / 60.f) / m_sun_minutes_per_day, 1);
      //int idx = static_cast<int>(m_sun_dir);
      //idx += t_solar_period * (static_cast<float>(Direction::NUM_ITEMS) - 1.f);
      static constexpr auto dp = 1.f/8.f; // solar period step (delta period).
      for (int i = 0; i < 8; ++i)
      {
        // 2 means east: S(0), SE(1), E(2). The sun comes up from the east.
        int curr_dir_idx = (i+2) % 8;
        if ((static_cast<int>(m_sun_dir) - 1) != curr_dir_idx)
        {
          if (math::in_range<float>(t_solar_period, i*dp, (i+1)*dp, Range::ClosedOpen))
          {
            m_sun_dir = static_cast<Direction>(curr_dir_idx + 1);
            break;
          }
        }
      }
    }
    
    // #NOTE: Only for unwalled area!
    bool is_inside_any_room(const RC& pos, BSPNode** room_node = nullptr)
    {
      for (auto* leaf : m_leaves)
        if (leaf->bb_leaf_room.is_inside_offs(pos, -1))
        {
          if (room_node != nullptr)
            *room_node = leaf;
          return true;
        }
      return false;
    }
    
    bool is_underground(BSPNode* leaf)
    {
      auto it = m_room_styles.find(leaf);
      if (it != m_room_styles.end())
        return it->second.is_underground;
      return false;
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
      
      std::vector<std::pair<std::string, bool>> items;
      items.emplace_back(std::make_pair("Keys:", false));
      for (int inv_key_idx = 0; inv_key_idx < num_inv_keys; ++inv_key_idx)
      {
        auto key_idx = m_player.key_idcs[inv_key_idx];
        const auto& key = all_keys[key_idx];
        items.emplace_back(make_pair("  Key:" + std::to_string(key.key_id), true));
      }
      items.emplace_back(std::make_pair("", false));
      items.emplace_back(std::make_pair("Lamps:", false));
      for (int inv_lamp_idx = 0; inv_lamp_idx < num_inv_lamps; ++inv_lamp_idx)
      {
        auto lamp_idx = m_player.lamp_idcs[inv_lamp_idx];
        items.emplace_back(std::make_pair("  Lamp:" + std::to_string(lamp_idx), true));
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
                                           m_player.inv_select_idx == adj_item_idx);
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
      
      drawing::draw_box(sh, bb_inv, drawing::OutlineType::Line, { Color::White, Color::DarkGray }, { Color::White, Color::DarkGray }, ' ');
    }
    
    template<typename Lambda>
    void clear_field(Lambda get_field_ptr, bool clear_val)
    {
      for (auto& key : all_keys)
        *get_field_ptr(&key) = clear_val;
        
      for (auto& lamp : all_lamps)
        *get_field_ptr(&lamp) = clear_val;
      
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
    void update_field(const RC& curr_pos, Lambda get_field_ptr, bool set_val)
    {
      const auto c_fow_dist = 2.3f;
      
      for (auto& key : all_keys)
        if (distance(key.pos, curr_pos) <= c_fow_dist)
          *get_field_ptr(&key) = set_val;
          
      for (auto& lamp : all_lamps)
        if (distance(lamp.pos, curr_pos) <= c_fow_dist)
          *get_field_ptr(&lamp) = set_val;
      
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
        
        //    ###
        //   #####
        //    ###
        set_field(local_pos);
        for (int c = -1; c <= +1; ++c)
        {
          set_field(local_pos + RC { -1, c });
          set_field(local_pos + RC { +1, c });
        }
        for (int c = -2; c <= +2; ++c)
          set_field(local_pos + RC { 0, c });
        
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
    
  public:
    DungGine(bool use_fow)
      : message_handler(std::make_unique<MessageHandler>())
      , use_fog_of_war(use_fow)
    {}
    
    void load_dungeon(BSPTree* bsp_tree)
    {
      m_bsp_tree = bsp_tree;
      m_leaves = m_bsp_tree->fetch_leaves();
    }
    
    void style_dungeon()
    {
      for (auto* leaf : m_leaves)
      {
        RoomStyle room_style;
        room_style.init_rand();
        m_room_styles[leaf] = room_style;
      }
    }
    
    void set_player_character(char ch) { m_player.character = ch; }
    bool place_player(const RC& screen_size, std::optional<RC> world_pos = std::nullopt)
    {
      const auto world_size = m_bsp_tree->get_world_size();
      m_screen_in_world.set_size(screen_size);
    
      if (world_pos.has_value())
        m_player.pos = world_pos.value();
      else
        m_player.pos = world_size / 2;
      
      const auto& room_corridor_map = m_bsp_tree->get_room_corridor_map();
      
      const int c_max_num_iters = 1e5;
      int num_iters = 0;
      do
      {
        for (const auto& cp : room_corridor_map)
          if (cp.second->is_inside_corridor(m_player.pos))
          {
            m_player.is_spawned = true;
            m_player.curr_corridor = cp.second;
            m_screen_in_world.set_pos(m_player.pos - m_screen_in_world.size()/2);
            return true;
          }
        m_player.pos += { rnd::rand_int(-2, +2), rnd::rand_int(-2, +2) };
        m_player.pos = m_player.pos.clamp(0, world_size.r, 0, world_size.c);
      } while (++num_iters < c_max_num_iters);
      return false;
    }
    
    // Randomizes the starting direction of the sun.
    void configure_sun(float minutes_per_day)
    {
      Direction sun_dir = static_cast<Direction>(rnd::rand_int(0, 7) + 1);
      configure_sun(sun_dir, minutes_per_day);
    }
    
    void configure_sun(Direction sun_dir, float minutes_per_day)
    {
      if (sun_dir == Direction::None || sun_dir == Direction::NUM_ITEMS)
      {
        std::cerr << "ERROR: invalid value of sun direction supplied to function DungGine::configure_sun()!" << std::endl;
        return;
      }
      m_sun_dir = sun_dir;
      m_sun_minutes_per_day = minutes_per_day;
      // None, S, SE, E, NE, N, NW, W, SW, NUM_ITEMS
      // 0     1  2   3  4   5  6   7  8
      m_sun_t_offs = (static_cast<int>(sun_dir) - 1) / 8.f;
    }
    
    bool place_keys()
    {
      const auto world_size = m_bsp_tree->get_world_size();
      const auto& door_vec = m_bsp_tree->fetch_doors();
      const int c_max_num_iters = 1e5;
      int num_iters = 0;
      for (auto* d : door_vec)
      {
        if (d->is_locked)
        {
          Key key;
          key.key_id = d->key_id;
          key.randomize_fg_color();
          do
          {
            key.pos =
            {
              rnd::rand_int(0, world_size.r),
              rnd::rand_int(0, world_size.c)
            };
          } while (num_iters++ < c_max_num_iters && !is_inside_any_room(key.pos));
          
          BSPNode* leaf = nullptr;
          if (!is_inside_any_room(key.pos, &leaf))
            return false;
            
          if (leaf != nullptr)
            key.is_underground = is_underground(leaf);
            
          all_keys.emplace_back(key);
        }
      }
      return true;
    }
    
    bool place_lamps(int num_lamps)
    {
      const auto world_size = m_bsp_tree->get_world_size();
      const int c_max_num_iters = 1e5;
      int num_iters = 0;
      for (int lamp_idx = 0; lamp_idx < num_lamps; ++lamp_idx)
      {
        Lamp lamp;
        lamp.type = rnd::rand_enum<Lamp::LampType>();
        do
        {
          lamp.pos =
          {
            rnd::rand_int(0, world_size.r),
            rnd::rand_int(0, world_size.c)
          };
        } while (num_iters++ < c_max_num_iters && !is_inside_any_room(lamp.pos));
        
        BSPNode* leaf = nullptr;
        if (!is_inside_any_room(lamp.pos, &leaf))
          return false;
          
        if (leaf != nullptr)
          lamp.is_underground = is_underground(leaf);
        
        all_lamps.emplace_back(lamp);
      }
      return true;
    }
    
    void set_screen_scrolling_mode(ScreenScrollingMode mode, float t_page = 0.2f)
    {
      scr_scrolling_mode = mode;
      if (mode == ScreenScrollingMode::PageWise)
        t_scroll_amount = t_page;
    }
    
    void update(double sim_time_s, const keyboard::KeyPressData& kpd)
    {
      update_sun(static_cast<float>(sim_time_s));
      int sun_dir_idx = static_cast<int>(m_sun_dir) - 1;
      m_shadow_dir = static_cast<Direction>(((sun_dir_idx + 4) % 8) + 1);
      
      auto is_inside_curr_bb = [&](int r, int c) -> bool
      {
        if (m_player.curr_corridor != nullptr && m_player.curr_corridor->is_inside_corridor({r, c}))
          return true;
        if (m_player.curr_room != nullptr && m_player.curr_room->is_inside_room({r, c}))
          return true;
        return false;
      };
      
      auto& curr_pos = m_player.pos;
      if (str::to_lower(kpd.curr_key) == 'a' || kpd.curr_special_key == keyboard::SpecialKey::Left)
      {
        if (m_player.show_inventory)
        {
        }
        else if (is_inside_curr_bb(curr_pos.r, curr_pos.c - 1))
          curr_pos.c--;
      }
      else if (str::to_lower(kpd.curr_key) == 'd' || kpd.curr_special_key == keyboard::SpecialKey::Right)
      {
        if (m_player.show_inventory)
        {
          if (m_player.inv_select_idx >= 0)
          {
            std::string msg = "You dropped an item: ";
            if (m_player.inv_select_idx < m_player.key_idcs.size())
            {
              auto key_idx = m_player.key_idcs[m_player.inv_select_idx];
              auto& key = all_keys[key_idx];
              key.picked_up = false;
              key.pos = curr_pos;
              stlutils::erase(m_player.key_idcs, key_idx);
              msg += "key:" + std::to_string(key.key_id) + "!";
            }
            else if (m_player.inv_select_idx < m_player.key_idcs.size() + m_player.lamp_idcs.size())
            {
              auto lamp_idx = m_player.lamp_idcs[m_player.inv_select_idx - m_player.key_idcs.size()];
              auto& lamp = all_lamps[lamp_idx];
              lamp.picked_up = false;
              lamp.pos = curr_pos;
              stlutils::erase(m_player.lamp_idcs, lamp_idx);
              msg += "lamp:" + std::to_string(lamp_idx) + "!";
            }
            else
            {
              msg += "Invalid Item!";
              std::cerr << "ERROR: Attempted to drop invalid item!" << std::endl;
            }
            m_player.inv_select_idx = -1;
            message_handler->add_message(static_cast<float>(sim_time_s),
                                         msg,
                                         MessageHandler::Level::Guide);
          }
        }
        else if (is_inside_curr_bb(curr_pos.r, curr_pos.c + 1))
          curr_pos.c++;
      }
      else if (str::to_lower(kpd.curr_key) == 's' || kpd.curr_special_key == keyboard::SpecialKey::Down)
      {
        if (m_player.show_inventory)
        {
          m_player.inv_hilite_idx++;
          m_player.inv_hilite_idx = m_player.inv_hilite_idx % (m_player.key_idcs.size() + m_player.lamp_idcs.size());
        }
        else if (is_inside_curr_bb(curr_pos.r + 1, curr_pos.c))
          curr_pos.r++;
      }
      else if (str::to_lower(kpd.curr_key) == 'w' || kpd.curr_special_key == keyboard::SpecialKey::Up)
      {
        if (m_player.show_inventory)
        {
          m_player.inv_hilite_idx--;
          if (m_player.inv_hilite_idx < 0)
            m_player.inv_hilite_idx = static_cast<int>(m_player.key_idcs.size() + m_player.lamp_idcs.size()) - 1;
        }
        else if (is_inside_curr_bb(curr_pos.r - 1, curr_pos.c))
          curr_pos.r--;
      }
      else if (kpd.curr_key == ' ')
      {
        if (m_player.show_inventory)
        {
          int& select_idx = m_player.inv_select_idx;
          const int hilite_idx = m_player.inv_hilite_idx;
          select_idx = (select_idx == -1) ? hilite_idx : -1;
        }
        else
        {
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
                  
                  message_handler->add_message(static_cast<float>(sim_time_s),
                                               "The door is unlocked!",
                                               MessageHandler::Level::Guide);
                }
                else
                {
                  message_handler->add_message(static_cast<float>(sim_time_s),
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
              if (m_player.key_idcs.size() <= m_player.inv_select_idx)
                m_player.inv_select_idx++;
              if (m_player.key_idcs.size() <= m_player.inv_hilite_idx)
                m_player.inv_hilite_idx++;
              m_player.key_idcs.emplace_back(key_idx);
              key.picked_up = true;
              message_handler->add_message(static_cast<float>(sim_time_s),
                                           "You picked up a key!", MessageHandler::Level::Guide);
            }
          }
          for (size_t lamp_idx = 0; lamp_idx < all_lamps.size(); ++lamp_idx)
          {
            auto& lamp = all_lamps[lamp_idx];
            if (lamp.pos == curr_pos && !lamp.picked_up)
            {
              m_player.lamp_idcs.emplace_back(lamp_idx);
              lamp.picked_up = true;
              message_handler->add_message(static_cast<float>(sim_time_s),
                                           "You picked up a lamp!", MessageHandler::Level::Guide);
            }
          }
        }
      }
      else if (str::to_lower(kpd.curr_key) == 'i')
      {
        math::toggle(m_player.show_inventory);
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
      
      // Fog of war
      if (use_fog_of_war)
        update_field(curr_pos,
                     [](auto obj) { return &obj->fog_of_war; },
                     false);
                  
      // Light
      auto* lamp = m_player.get_selected_lamp(all_lamps);
      clear_field([](auto obj) { return &obj->light; }, false);
      if (lamp != nullptr)
      {
        update_field(curr_pos,
                     [](auto obj) { return &obj->light; },
                     true);
      }
      
      // Scrolling mode.
      switch (scr_scrolling_mode)
      {
        case ScreenScrollingMode::AlwaysInCentre:
          m_screen_in_world.set_pos(curr_pos - m_screen_in_world.size()/2);
          break;
        case ScreenScrollingMode::PageWise:
        {
          int offs_v = -static_cast<int>(std::round(m_screen_in_world.r_len*t_scroll_amount));
          int offs_h = -static_cast<int>(std::round(m_screen_in_world.c_len*t_scroll_amount));
          if (!m_screen_in_world.is_inside_offs(curr_pos, offs_v, offs_h))
            m_screen_in_world.set_pos(curr_pos - m_screen_in_world.size()/2);
          break;
        }
        case ScreenScrollingMode::WhenOutsideScreen:
          if (!m_screen_in_world.is_inside(curr_pos))
          {
            if (curr_pos.r < m_screen_in_world.top())
              m_screen_in_world.r -= m_screen_in_world.r_len;
            else if (curr_pos.r > m_screen_in_world.bottom())
              m_screen_in_world.r += m_screen_in_world.r_len;
            else if (curr_pos.c < m_screen_in_world.left())
              m_screen_in_world.c -= m_screen_in_world.c_len;
            else if (curr_pos.c > m_screen_in_world.right())
              m_screen_in_world.c += m_screen_in_world.c_len;
          }
          break;
        default:
          break;
      }
    }
    
    
    template<int NR, int NC>
    void draw(SpriteHandler<NR, NC>& sh, double sim_time_s) const
    {
      const auto& room_corridor_map = m_bsp_tree->get_room_corridor_map();
      const auto& door_vec = m_bsp_tree->fetch_doors();
      
      message_handler->update(sh, static_cast<float>(sim_time_s), true);
      
      if (m_player.show_inventory)
        draw_inventory(sh);
      
      if (m_player.is_spawned)
      {
        auto player_scr_pos = get_screen_pos(m_player.pos);
        sh.write_buffer(std::string(1, m_player.character), player_scr_pos.r, player_scr_pos.c, m_player.style);
      }
      
      for (auto* door : door_vec)
      {
        auto door_pos = door->pos;
        auto door_scr_pos = get_screen_pos(door_pos);
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
      {
        if (key.picked_up || (use_fog_of_war && key.fog_of_war) || !key.light)
          continue;
        auto key_scr_pos = get_screen_pos(key.pos);
        sh.write_buffer(std::string(1, key.character), key_scr_pos.r, key_scr_pos.c, key.style);
      }
      
      for (const auto& lamp : all_lamps)
      {
        if (lamp.picked_up)
          continue;
        auto lamp_scr_pos = get_screen_pos(lamp.pos);
        sh.write_buffer(std::string(1, lamp.character), lamp_scr_pos.r, lamp_scr_pos.c, lamp.style);
      }
      
      auto shadow_type = m_shadow_dir;
      for (const auto& room_pair : m_room_styles)
      {
        auto* room = room_pair.first;
        const auto& bb = room->bb_leaf_room;
        const auto& room_style = room_pair.second;
        auto bb_scr_pos = get_screen_pos(bb.pos());
        
        // Fog of war
        if (use_fog_of_war)
        {
          for (int r = 0; r <= bb.r_len; ++r)
          {
            for (int c = 0; c <= bb.c_len; ++c)
            {
              if (room->fog_of_war[r * (bb.c_len + 1) + c])
                sh.write_buffer(".", bb_scr_pos.r + r, bb_scr_pos.c + c, Color::Black, Color::Black);
            }
          }
        }
        
        drawing::draw_box(sh,
                          bb_scr_pos.r, bb_scr_pos.c, bb.r_len, bb.c_len,
                          room_style.wall_type,
                          room_style.wall_style,
                          room_style.get_fill_style(),
                          room_style.get_fill_char(),
                          room_style.is_underground ? Direction::None : shadow_type,
                          styles::shade_style(room_style.get_fill_style(), color::ShadeType::Dark),
                          room_style.get_fill_char(),
                          room->light);
      }
      
      for (const auto& cp : room_corridor_map)
      {
        auto* corr = cp.second;
        const auto& bb = corr->bb;
        auto bb_scr_pos = get_screen_pos(bb.pos());
        
        // Fog of war
        if (use_fog_of_war)
        {
          for (int r = 0; r <= bb.r_len; ++r)
          {
            for (int c = 0; c <= bb.c_len; ++c)
            {
              if (corr->fog_of_war[r * (bb.c_len + 1) + c])
                sh.write_buffer(".", bb_scr_pos.r + r, bb_scr_pos.c + c, Color::Black, Color::Black);
            }
          }
        }
        
        drawing::draw_box(sh,
                          bb_scr_pos.r, bb_scr_pos.c, bb.r_len, bb.c_len,
                          WallType::Masonry4,
                          { Color::LightGray, Color::Black }, //wall_palette[WallBasicType::Masonry],
                          { Color::DarkGray, Color::LightGray },
                          '8',
                          shadow_type,
                          { Color::LightGray, Color::DarkGray },
                          '8',
                          corr->light);
      }
    }
    
  };
  
}
