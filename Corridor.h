//
//  Corridor.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-06-25.
//

#pragma once
#include "Orientation.h"
#include <Core/Utils.h>
#include <Termin8or/Rectangle.h>
#include <Core/bool_vector.h>


namespace dung
{

  struct Door;

  struct Corridor
  {
    ttl::Rectangle bb;
    Orientation orientation = Orientation::Vertical;
    std::array<Door*, 2> doors;
    
    bool_vector fog_of_war;
    bool_vector light;
    
    bool is_inside_corridor(const RC& pos, ttl::BBLocation* location = nullptr) const
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
          if (bb.c_len < 3)
          {
            utils::try_set(location, ttl::BBLocation::Inside);
            return bb.is_inside_offs(pos, top_offs, bottom_offs, 0, 0);
          }
          utils::try_set(location, bb.find_location_offs(pos, top_offs, bottom_offs, -1, -1));
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
          if (bb.r_len < 3)
          {
            utils::try_set(location, ttl::BBLocation::Inside);
            return bb.is_inside_offs(pos, 0, 0, left_offs, right_offs);
          }
          utils::try_set(location, bb.find_location_offs(pos, -1, -1, left_offs, right_offs));
          return bb.is_inside_offs(pos, -1, -1, left_offs, right_offs);
        }
        default: // Impossible to reach, but alas necessary with some compilers.
          return false;
      }
    }
    
    bool is_in_fog_of_war(const RC& world_pos)
    {
      auto local_pos = world_pos - bb.pos();
      auto idx = local_pos.r * bb.c_len + local_pos.c;
      if (math::in_range<int>(idx, 0, static_cast<int>(fog_of_war.size()), Range::ClosedOpen))
        return fog_of_war[idx];
      return true;
    }
    
    bool is_in_light(const RC& world_pos)
    {
      auto local_pos = world_pos - bb.pos();
      auto idx = local_pos.r * bb.c_len + local_pos.c;
      if (math::in_range<int>(idx, 0, static_cast<int>(light.size()), Range::ClosedOpen))
        return light[idx];
      return false;
    }
  };

}
