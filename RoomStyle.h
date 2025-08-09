//
//  RoomStyle.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-06-21.
//

#pragma once
#include "SolarMotionPatterns.h"
#include "DungGineStyles.h"

namespace dung
{
  
  using WallType = drawing::OutlineType; //{ Hash, Masonry, Masonry1, Masonry2, Masonry3, Temple };
  using SolarDirection = drawing::SolarDirection;
  using Style = styles::Style;
  using HiliteSelectFGStyle = styles::HiliteSelectFGStyle;
  enum class FloorType { None, Sand, Grass, Stone, Stone2, Water, Wood, NUM_ITEMS };
  
  struct RoomStyle
  {
    WallType wall_type = WallType::Hash;
    Style wall_style { Color::DarkGray, Color::LightGray };
    FloorType floor_type = FloorType::None;
    bool is_underground = true;
    RC tex_pos;
    Latitude latitude = Latitude::NorthernHemisphere;
    Longitude longitude = Longitude::F;
    
    void init_rand(bool first_floor, bool first_floor_is_surface_level)
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
          break;
        case WallType::Temple:
          wall_basic_type = WallBasicType::Temple;
          break;
        case WallType::Line:
        case WallType::Hash:
        default:
          wall_basic_type = WallBasicType::Other;
          break;
      }
      wall_style = styles::get_random_style(wall_palette[wall_basic_type]);
      do
      {
        floor_type = rnd::rand_enum<FloorType>();
      } while (floor_type == FloorType::None);
      if (first_floor_is_surface_level)
        is_underground = !first_floor;
      else
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
          style = styles::make_shaded_style(Color::Yellow, color::ShadeType::Bright);
          break;
        case FloorType::Grass:
          style = styles::make_shaded_style(Color::Green, color::ShadeType::Bright);
          break;
        case FloorType::Stone:
        case FloorType::Stone2:
          style = styles::make_shaded_style(Color::LightGray, color::ShadeType::Bright);
          break;
        case FloorType::Water:
          style = styles::make_shaded_style(Color::Blue, color::ShadeType::Bright);
          break;
        case FloorType::Wood:
          style = { Color::DarkRed, Color::Yellow };
          break;
        case FloorType::None:
        default:
          style = { Color::DarkGray, Color::LightGray };
          break;
      }
      if (is_underground)
        style.swap();
      return style;
    }
    
  };
  
}
