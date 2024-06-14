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
    using ShadowType = drawing::ShadowType;
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
    
    template<int NR, int NC>
    void draw(SpriteHandler<NR, NC>& sh, int r0 = 0, int c0 = 0) const
    {
      auto shadow_type = rnd::rand_enum<ShadowType>(); // Random for now.
      for (const auto& room_pair : m_room_styles)
      {
        const auto& bb = room_pair.first->bb_leaf_room;
        const auto& room_style = room_pair.second;
        drawing::draw_box(sh,
                          r0 + bb.r, c0 + bb.c, bb.r_len, bb.c_len,
                          room_style.wall_type,
                          room_style.wall_style,
                          room_style.get_fill_style(),
                          room_style.get_fill_char(),
                          room_style.is_underground ? ShadowType::None : shadow_type,
                          room_style.get_shadow_style(),
                          room_style.get_fill_char());
      }
      
      const auto& corridors = m_bsp_tree->get_flat_corridors();
      for (const auto& cp : corridors)
      {
        const auto& bb = cp.second;
        
        drawing::draw_box(sh,
                          r0 + bb.r, c0 + bb.c, bb.r_len, bb.c_len,
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
