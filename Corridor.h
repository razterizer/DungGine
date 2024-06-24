//
//  Corridor.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-06-25.
//

#pragma once
#include "Orientation.h"


namespace dung
{

  struct Corridor
  {
    ttl::Rectangle bb;
    Orientation orientation = Orientation::Vertical;
    std::array<Door*, 2> doors;
    
    bool_vector fog_of_war;
    bool_vector light;
    
    bool is_inside_corridor(const RC& pos) const
    {
      switch (orientation)
      {
        case Orientation::Vertical:
        {
          bool top_door_open = doors[0]->pos.r < doors[1]->pos.r ? 
            doors[0]->open_or_no_door() : doors[1]->open_or_no_door();
          bool bottom_door_open = doors[0]->pos.r < doors[1]->pos.r ? 
            doors[1]->open_or_no_door() : doors[0]->open_or_no_door();
          int top_offs = top_door_open ? 0 : -1;
          int bottom_offs = bottom_door_open ? 0 : -1;
          if (bb.c_len < 2)
            return bb.is_inside_offs(pos, top_offs, bottom_offs, 0, 0);
          return bb.is_inside_offs(pos, top_offs, bottom_offs, -1, -1);
        }
        case Orientation::Horizontal:
        {
          bool left_door_open = doors[0]->pos.c < doors[1]->pos.c ? 
            doors[0]->open_or_no_door() : doors[1]->open_or_no_door();
          bool right_door_open = doors[0]->pos.c < doors[1]->pos.c ? 
            doors[1]->open_or_no_door() : doors[0]->open_or_no_door();
          int left_offs = left_door_open ? 0 : -1;
          int right_offs = right_door_open ? 0 : -1;
          if (bb.r_len < 2)
            return bb.is_inside_offs(pos, 0, 0, left_offs, right_offs);
          return bb.is_inside_offs(pos, -1, -1, left_offs, right_offs);
        }
        default: // Impossible to reach, but alas necessary with some compilers.
          return false;
      }
    }
  };

}
