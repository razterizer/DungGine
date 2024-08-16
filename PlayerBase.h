//
//  AgentBase.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-08-15.
//

#pragma once

namespace dung
{

  struct PlayerBase
  {
    char character;
    Style style;
  
    RC pos, last_pos;
    float los_r = 0.f;
    float los_c = 0.f;
    float last_los_r = 0.f;
    float last_los_c = 0.f;
    
    BSPNode* curr_room = nullptr;
    Corridor* curr_corridor = nullptr;
    
    int health = globals::max_health;
    int strength = 10;
    int dexterity = 10;
    int endurance = 10;
    int weakness = 0;
    int thac0 = 1;
    
    Terrain on_terrain = Terrain::Default;
  };

}
