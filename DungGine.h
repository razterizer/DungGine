//
//  DungGine.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-06-13.
//

#pragma once
#include "BSPTree.h"
#include "DungGineStyles.h"


namespace dung
{


  class DungGine
  {
    BSPTree* m_bsp_tree;
    std::vector<BSPNode*> m_leaves;
    
    using WallType = drawing::OutlineType; //{ Hash, Masonry, Masonry1, Masonry2, Masonry3, Temple };
    using Direction = drawing::Direction;
    using Style = styles::Style;
    enum class FloorType { None, Sand, Grass, Stone, Stone2, Water, Wood, NUM_ITEMS };
    
    struct RoomStyle
    {
      WallType wall_type = WallType::Hash;
      Style wall_style { Text::Color::DarkGray, Text::Color::LightGray };
      FloorType floor_type = FloorType::None;
      bool is_underground = true;
      
      void init_rand()
      {
        wall_type = rnd::rand_enum<WallType>();
        WallBasicType wall_basic_type = WallBasicType::Other;
        switch (wall_type)
        {
          case WallType::Masonry:
          case WallType::Masonry2:
          case WallType::Masonry3:
          case WallType::Masonry4:
            wall_basic_type = WallBasicType::Masonry;
          case WallType::Temple:
            wall_basic_type = WallBasicType::Temple;
          case WallType::Line:
          case WallType::Hash:
          default:
            wall_basic_type = WallBasicType::Other;
            break;
        }
        wall_style = styles::get_random_style(wall_palette[wall_basic_type]);
        floor_type = rnd::rand_enum<FloorType>();
        is_underground = rnd::rand_bool();
      }
      
      char get_fill_char() const
      {
        switch (floor_type)
        {
          case FloorType::Sand:
            return ':';
          case FloorType::Grass:
            return '|';
          case FloorType::Stone:
            return 'H';
          case FloorType::Stone2:
            return '8';
          case FloorType::Water:
            return '~';
          case FloorType::Wood:
            return 'W';
          case FloorType::None:
          default:
            return ' ';
            break;
        }
      }
      
      char get_shadow_char() const
      {
        return get_fill_char();
      }
      
      Style get_fill_style() const
      {
        Style style;
        switch (floor_type)
        {
          case FloorType::Sand:
            style = { Text::Color::DarkYellow, Text::Color::Yellow };
            break;
          case FloorType::Grass:
            style = { Text::Color::DarkGreen, Text::Color::Green };
            break;
          case FloorType::Stone:
          case FloorType::Stone2:
            style = { Text::Color::DarkGray, Text::Color::LightGray };
            break;
          case FloorType::Water:
            style = { Text::Color::DarkBlue, Text::Color::Blue };
            break;
          case FloorType::Wood:
            style = { Text::Color::DarkRed, Text::Color::Yellow };
            break;
          case FloorType::None:
          default:
            style = { Text::Color::Black, Text::Color::LightGray };
            break;
        }
        if (is_underground)
          std::swap(style.fg_color, style.bg_color);
        return style;
      }
      
      Style get_shadow_style() const
      {
        switch (floor_type)
        {
          case FloorType::Sand:
            return { Text::Color::Yellow, Text::Color::DarkYellow };
          case FloorType::Grass:
            return { Text::Color::Green, Text::Color::DarkGreen };
          case FloorType::Stone:
          case FloorType::Stone2:
            return { Text::Color::LightGray, Text::Color::DarkGray };
          case FloorType::Water:
            return { Text::Color::Blue, Text::Color::DarkBlue };
          case FloorType::Wood:
            return { Text::Color::Red, Text::Color::DarkYellow };
          case FloorType::None:
          default:
            return { Text::Color::Black, Text::Color::DarkGray };
            break;
        }
      }
    };
    
    std::map<BSPNode*, RoomStyle> m_room_styles;
    
    struct Player
    {
      char character = '@';
      Style style = { Text::Color::Magenta, Text::Color::White };
      RC world_pos;
      bool is_spawned = false;
    };
    
    Player m_player;
    RC m_screen_world_pos;
    // Value between 0 and 1 where 1 means a full screen vertically or horizontally.
    float t_scroll_amount = 0.5f; // Half the screen will be scrolled.
    
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
    
    RC get_screen_pos(const RC& world_pos) const
    {
      return world_pos - m_screen_world_pos;
    }
    
  public:
    DungGine() = default;
    
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
    void place_player(std::optional<RC> world_pos = std::nullopt)
    {
      const auto world_size = m_bsp_tree->get_world_size();
    
      if (world_pos.has_value())
        m_player.world_pos = world_pos.value();
      else
        m_player.world_pos = world_size / 2;
      
      const auto& room_corridor_map = m_bsp_tree->get_room_corridor_map();
      
      do
      {
        for (const auto& cp : room_corridor_map)
          if (cp.second->is_inside_corridor(m_player.world_pos))
          {
            m_player.is_spawned = true;
            return;
          }
        m_player.world_pos += { rnd::rand_int(-1, +1), rnd::rand_int(-1, +1) };
        m_player.world_pos = m_player.world_pos.clamp(0, world_size.r, 0, world_size.c);
      } while (true);
    }
    
    template<int NR, int NC>
    void draw(SpriteHandler<NR, NC>& sh) const
    {
      const auto& room_corridor_map = m_bsp_tree->get_room_corridor_map();
      
      if (m_player.is_spawned)
      {
        auto player_scr_pos = get_screen_pos(m_player.world_pos);
        sh.write_buffer(std::string(1, m_player.character), player_scr_pos.r, player_scr_pos.c, m_player.style);
      }
      
      for (const auto& cp : room_corridor_map)
      {
        auto* corr = cp.second;
        auto door_pos_0 = corr->doors[0]->pos;
        auto door_pos_1 = corr->doors[1]->pos;
        auto door_scr_pos_0 = get_screen_pos(door_pos_0);
        auto door_scr_pos_1 = get_screen_pos(door_pos_1);
        sh.write_buffer("D", door_scr_pos_0.r, door_scr_pos_0.c, Text::Color::Black, Text::Color::Yellow);
        sh.write_buffer("D", door_scr_pos_1.r, door_scr_pos_1.c, Text::Color::Black, Text::Color::Yellow);
      }
      
      auto shadow_type = rnd::rand_enum<Direction>(); // Random for now.
      for (const auto& room_pair : m_room_styles)
      {
        const auto& bb = room_pair.first->bb_leaf_room;
        const auto& room_style = room_pair.second;
        auto bb_scr_pos = get_screen_pos(bb.pos());
        drawing::draw_box(sh,
                          bb_scr_pos.r, bb_scr_pos.c, bb.r_len, bb.c_len,
                          room_style.wall_type,
                          room_style.wall_style,
                          room_style.get_fill_style(),
                          room_style.get_fill_char(),
                          room_style.is_underground ? Direction::None : shadow_type,
                          room_style.get_shadow_style(),
                          room_style.get_fill_char());
      }
      
      for (const auto& cp : room_corridor_map)
      {
        auto* corr = cp.second;
        const auto& bb = corr->bb;
        
        auto bb_scr_pos = get_screen_pos(bb.pos());
        drawing::draw_box(sh,
                          bb_scr_pos.r, bb_scr_pos.c, bb.r_len, bb.c_len,
                          WallType::Masonry4,
                          { Text::Color::LightGray, Text::Color::Black }, //wall_palette[WallBasicType::Masonry],
                          { Text::Color::DarkGray, Text::Color::LightGray },
                          '8',
                          shadow_type,
                          { Text::Color::LightGray, Text::Color::DarkGray },
                          '8');
      }
    }
    
  };
  
}
