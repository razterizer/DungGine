//
//  Door.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-06-25.
//

#pragma once


namespace dung
{

  struct Corridor;
  struct BSPNode;
  

  struct Door
  {
    RC pos; // world pos
    
    bool is_door = false;
    bool is_open = false;
    bool is_locked = false;
    int key_id = 0;
    
    BSPNode* room = nullptr;
    Corridor* corridor = nullptr;
    
    bool fog_of_war = true;
    bool light = false;
    
    bool open_or_no_door() const
    {
      return !is_door || is_open;
    };
  };

}
