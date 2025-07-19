//
//  DungObject.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-09-15.
//

#pragma once
struct BSPNode;
struct Corridor;


namespace dung
{

  struct DungObject
  {
    RC pos; // world pos
    bool fog_of_war = true;
    bool light = false;
    bool visible = false;
    bool is_underground = false;
    BSPNode* curr_room = nullptr;
    Corridor* curr_corridor = nullptr;
  };

}
