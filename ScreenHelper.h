//
//  ScreenHelper.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-08-13.
//

#pragma once
#include <Termin8or/Rectangle.h>

namespace dung
{

  enum class ScreenScrollingMode { AlwaysInCentre, PageWise, WhenOutsideScreen };

  class ScreenHelper final
  {
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
    
  public:
    RC get_screen_pos(const RC& world_pos) const
    {
      return world_pos - m_screen_in_world.pos();
    }
    
    void set_screen_size(const RC& screen_size)
    {
      m_screen_in_world.set_size(screen_size);
    }
    
    RC get_screen_size() const
    {
      return m_screen_in_world.size();
    }
    
    void focus_on_world_pos_mid_screen(const RC& world_pos)
    {
      m_screen_in_world.set_pos(world_pos - m_screen_in_world.size()/2);
    }
    
    void set_screen_scrolling_mode(ScreenScrollingMode mode, float t_page = 0.2f)
    {
      scr_scrolling_mode = mode;
      if (mode == ScreenScrollingMode::PageWise)
        t_scroll_amount = t_page;
    }
    
    void update_scrolling(const RC& world_pos)
    {
      // Scrolling mode.
      switch (scr_scrolling_mode)
      {
        case ScreenScrollingMode::AlwaysInCentre:
          m_screen_in_world.set_pos(world_pos - m_screen_in_world.size()/2);
          break;
        case ScreenScrollingMode::PageWise:
        {
          int offs_v = -math::roundI(m_screen_in_world.r_len*t_scroll_amount);
          int offs_h = -math::roundI(m_screen_in_world.c_len*t_scroll_amount);
          if (!m_screen_in_world.is_inside_offs(world_pos, offs_v, offs_h))
            m_screen_in_world.set_pos(world_pos - m_screen_in_world.size()/2);
          break;
        }
        case ScreenScrollingMode::WhenOutsideScreen:
          if (!m_screen_in_world.is_inside(world_pos))
          {
            if (world_pos.r < m_screen_in_world.top())
              m_screen_in_world.r -= m_screen_in_world.r_len;
            else if (world_pos.r > m_screen_in_world.bottom())
              m_screen_in_world.r += m_screen_in_world.r_len;
            else if (world_pos.c < m_screen_in_world.left())
              m_screen_in_world.c -= m_screen_in_world.c_len;
            else if (world_pos.c > m_screen_in_world.right())
              m_screen_in_world.c += m_screen_in_world.c_len;
          }
          break;
        default:
          break;
      }
    }
  };

}
