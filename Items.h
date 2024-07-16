//
//  Items.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-06-21.
//

#pragma once

namespace dung
{
  
  struct Item
  {
    RC pos; // world pos
    bool picked_up = false;
    Style style;
    char character = '?';
    bool fog_of_war = true;
    bool light = false;
    float weight = 0.f; // kg-ish.
    float price = 0.f;  // SEK-ish.
    bool is_underground = false;
    BSPNode* curr_room = nullptr;
    Corridor* curr_corridor = nullptr;
  };
  
  struct Key : Item
  {
    Key()
    {
      character = 'F';
      style.fg_color = Color::Green;
      style.bg_color = Color::Transparent2;
      weight = 0.1f;
      price = math::roundI(20*rnd::randn_clamp(20.f, 30.f, 0.f, 1e4f))/20.f;
    }
    
    int key_id = 0;
    
    void randomize_fg_color()
    {
      style.fg_color = color::get_random_color(key_fg_palette);
    }
  };
  
  struct Lamp : Item
  {
    Lamp()
    {
      character = 'Y';
      style.fg_color = Color::Yellow;
      style.bg_color = Color::Transparent2;
      weight = 0.4f;
      price = math::roundI(20*rnd::randn_clamp(200.f, 100.f, 0.f, 1e4f))/20.f;
    }
    enum class LampType { Isotropic, Directional, NUM_ITEMS };
    LampType type = LampType::Isotropic;
  };
  
}
