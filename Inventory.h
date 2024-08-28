//
//  Inventory.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-08-19.
//

#pragma once
#include "Items.h"
#include <Termin8or/SpriteHandler.h>
#include <Termin8or/Drawing.h>
#include <Termin8or/Rectangle.h>

namespace dung
{

  enum class InvItemState { NONE, SWITCH_SELECTION, SET_HILITE, RESET_HILITE };

  struct InvItem
  {
    std::string text;
    // level = 0: title, level = 1: sub-title, level = 2: item.
    int level = 2;
    bool selected = false;
    bool hilited = false;
    Item* item = nullptr;
    
    InvItem() = default;
    InvItem(const std::string& t)
      : text(t)
    {}
    InvItem(const std::string& t, Item* i)
      : text(t)
      , item(i)
    {}
    InvItem(const std::string& t, int lvl)
      : text(t)
      , level(lvl)
    {}
  };
  
  class InvSubGroup
  {
    InvItem title { "", 1 };
    bool use_title = false;
    std::vector<InvItem> m_items;
    int selection_idx = 0;
    int hilite_idx = 0;
    
  public:
    void add_item(const std::string& text, Item* item = nullptr)
    {
      m_items.emplace_back(text, item);
    }
    
    void remove_item(Item* item)
    {
      stlutils::erase_if(m_items, [item](const auto& ii) { return ii.item == item; });
    }
    
    void set_title(const std::string& text)
    {
      title.text = text;
      use_title = true;
    }
    
    InvItem get_item(int idx) const
    {
      if (use_title && idx == 0)
        return title;
      int rel_idx = idx - use_title;
      if (math::in_range<int>(rel_idx, 0, static_cast<int>(m_items.size()), Range::ClosedOpen))
        return m_items[rel_idx];
      return {};
    };
    
    InvItem* find_item(Item* i)
    {
      auto it = stlutils::find_if(m_items, [i](const auto& ii) { return ii.item == i; });
      if (it != m_items.end())
        return &(*it);
      return nullptr;
    }
    
    InvItem* get_selected_item()
    {
      auto it = stlutils::find_if(m_items, [](const auto& ii) { return ii.selected; });
      if (it != m_items.end())
        return &(*it);
      return nullptr;
    }
    
    void toggle_state(int idx, InvItemState state)
    {
      int rel_idx = idx - use_title;
      
      bool same_selection = selection_idx == rel_idx;
    
      if (state == InvItemState::SWITCH_SELECTION)
      {
        if (math::in_range<int>(selection_idx, 0, static_cast<int>(m_items.size()), Range::ClosedOpen))
        {
          auto& item = m_items[selection_idx];
          if (!same_selection)
            item.selected = false;
        }
      }
    
      if (state == InvItemState::SWITCH_SELECTION)
        selection_idx = rel_idx;
      else if (state == InvItemState::SET_HILITE)
        hilite_idx = rel_idx;
        
      if (math::in_range<int>(rel_idx, 0, static_cast<int>(m_items.size()), Range::ClosedOpen))
      {
        auto& item = m_items[rel_idx];
        switch (state)
        {
          case InvItemState::NONE: break;
          case InvItemState::SWITCH_SELECTION:
              math::toggle(item.selected);
            break;
          case InvItemState::SET_HILITE:
            item.hilited = true;
            break;
          case InvItemState::RESET_HILITE:
            item.hilited = false;
            break;
        }
      }
      
      if (state == InvItemState::RESET_HILITE)
        hilite_idx = -1;
    }
    
    int size() const { return use_title + static_cast<int>(m_items.size()); }
    
    std::vector<InvItem>::iterator begin()
    {
      return m_items.begin();
    }
    
    std::vector<InvItem>::iterator end()
    {
      return m_items.end();
    }
    
    std::vector<InvItem>::const_iterator begin() const
    {
      return m_items.cbegin();
    }
    
    std::vector<InvItem>::const_iterator end() const
    {
      return m_items.cend();
    }
  };
  
  // ////////////////
  
  class InvGroup
  {
    InvItem title { "", 0 };
    std::vector<InvSubGroup> m_subgroups;
    
  public:
    InvGroup(const std::string& title_text)
      : title(title_text, 0)
    {}
    
    const std::string& get_title() const
    {
      return title.text;
    }
  
    void add_subgroup(const InvSubGroup& subgroup)
    {
      m_subgroups.emplace_back(subgroup);
    }
    
    InvSubGroup* fetch_subgroup(int subgroup_idx)
    {
      if (math::in_range<int>(subgroup_idx, 0, m_subgroups.size(), Range::ClosedOpen))
        return &m_subgroups[subgroup_idx];
      else
      {
        add_subgroup({});
        return &m_subgroups.back();
      }
    }
    
    void remove_item(Item* item)
    {
      for (auto& sg : m_subgroups)
        sg.remove_item(item);
    }
    
    void set_title(const std::string& text)
    {
      title.text = text;
    }
    
    int size() const
    {
      int num_items = 1;
      for (auto& sg : m_subgroups)
        num_items += sg.size();
      return num_items;
    }
    
    InvItem get_item(int idx) const
    {
      if (idx == 0)
        return title;
    
      int cum_idx = 1; // Cumulative index.
      for (const auto& sg : m_subgroups)
      {
        int rel_idx = idx - cum_idx;
        if (math::in_range<int>(rel_idx, 0, sg.size(), Range::ClosedOpen))
          return sg.get_item(rel_idx);
        cum_idx += sg.size();
      }
      return {};
    };
    
    void toggle_state(int idx, InvItemState state)
    {
      if (idx == 0)
        return; // Can't select the title for now.
        
      int cum_idx = 1; // Cumulative index.
      for (auto& sg : m_subgroups)
      {
        int rel_idx = idx - cum_idx;
        if (math::in_range<int>(rel_idx, 0, sg.size(), Range::ClosedOpen))
          sg.toggle_state(rel_idx, state);
        cum_idx += sg.size();
      }
    }
    
    std::vector<InvSubGroup>::iterator begin()
    {
      return m_subgroups.begin();
    }
    
    std::vector<InvSubGroup>::iterator end()
    {
      return m_subgroups.end();
    }
    
    std::vector<InvSubGroup>::const_iterator begin() const
    {
      return m_subgroups.cbegin();
    }
    
    std::vector<InvSubGroup>::const_iterator end() const
    {
      return m_subgroups.cend();
    }
  };

  // //////////////////////

  class Inventory
  {
    std::vector<InvGroup> m_groups;
    ttl::Rectangle m_bb;
    
    int rb0_title = 2;
    int rb0_items = 4; // row box-relative-pos start items.
    int cb0_items = 2; // col box-relative-pos start items.
    int hilite_idx = 0;
    
  public:
    void set_bounding_box(const ttl::Rectangle& bb) { m_bb = bb; }
    
    void add_group(const InvGroup& group)
    {
      m_groups.emplace_back(group);
    }
    
    void remove_item(Item* item)
    {
      for (auto& g : m_groups)
        g.remove_item(item);
    }
    
    InvGroup* fetch_group(const std::string& group_title)
    {
      auto it = stlutils::find_if(m_groups,
        [&group_title](const auto& g) { return g.get_title() == group_title; });
      if (it != m_groups.end())
        return &(*it);
      else
      {
        add_group({ group_title });
        return &m_groups.back();
      }
    }
    
    int size() const
    {
      int num_lines = 0;
      for (const auto& g : m_groups)
        num_lines += g.size();
      return num_lines;
    }
    
    void clear()
    {
      m_groups.clear();
    }
    
    InvItem get_item(int idx) const
    {
      int cum_idx = 0; // Cumulative index.
      for (const auto& g : m_groups)
      {
        int rel_idx = idx - cum_idx;
        if (math::in_range<int>(rel_idx, 0, g.size(), Range::ClosedOpen))
          return g.get_item(rel_idx);
        cum_idx += g.size();
      }
      return {};
    };
    
    void toggle_state(int idx, InvItemState state)
    {
      int cum_idx = 0; // Cumulative index.
      for (auto& g : m_groups)
      {
        int rel_idx = idx - cum_idx;
        if (math::in_range<int>(rel_idx, 0, g.size(), Range::ClosedOpen))
          g.toggle_state(rel_idx, state);
        cum_idx += g.size();
      }
    }
    
    void inc_hilite()
    {
      toggle_state(hilite_idx, InvItemState::RESET_HILITE);
      int num_wraps = 0;
      do
      {
        hilite_idx++;
        if (hilite_idx >= size())
        {
          hilite_idx = 0;
          num_wraps++;
        }
      } while (get_item(hilite_idx).level < 2 && num_wraps < 3);
      toggle_state(hilite_idx, InvItemState::SET_HILITE);
    }
    
    void dec_hilite()
    {
      toggle_state(hilite_idx, InvItemState::RESET_HILITE);
      int num_wraps = 0;
      do
      {
        hilite_idx--;
        if (hilite_idx < 0)
        {
          hilite_idx = size() - 1;
          num_wraps++;
        }
      } while (get_item(hilite_idx).level < 2 && num_wraps < 3);
      toggle_state(hilite_idx, InvItemState::SET_HILITE);
    }
    
    void toggle_hilited_selection()
    {
      toggle_state(hilite_idx, InvItemState::SWITCH_SELECTION);
    }
      
    template<int NR, int NC>
    void draw(SpriteHandler<NR, NC>& sh) const
    {
      sh.write_buffer(str::adjust_str("Inventory", str::Adjustment::Center, m_bb.c_len), m_bb.top() + rb0_title, m_bb.left(), Color::White, Color::Transparent2);
      
      int num_lines = size();
      
      int r_offs = std::max(0, hilite_idx - (m_bb.r_len - rb0_items - 1));
        
      for (int r = 0; r < num_lines; ++r)
      {
        auto item = get_item(r);
        auto text = item.text;
        styles::Style style { Color::Default, Color::Transparent2 };
        int c_offs = item.level*2;
        switch (item.level)
        {
          case 0: style.fg_color = Color::White; break;
          case 1: style.fg_color = Color::Yellow; break;
          case 2:
            if (item.selected)
            {
              if (item.hilited)
                style.fg_color = Color::Blue;
              else
                style.fg_color = Color::DarkBlue;
            }
            else
            {
              if (item.hilited)
                style.fg_color = Color::Green;
              else
                style.fg_color = Color::DarkGreen;
            }
            break;
        }
        int rb_items = rb0_items + r - r_offs;
        int cb_items = cb0_items + c_offs;
        if (math::in_range<int>(rb_items, rb0_items, m_bb.r_len, Range::ClosedOpen))
          sh.write_buffer(text, m_bb.top() + rb_items, m_bb.left() + cb_items, style);
      }
      
      drawing::draw_box_outline(sh, m_bb, drawing::OutlineType::Line, { Color::White, Color::DarkGray });
      drawing::draw_box(sh, m_bb, { Color::White, Color::DarkGray }, ' ');
    }
    
    std::vector<InvGroup>::iterator begin()
    {
      return m_groups.begin();
    }
    
    std::vector<InvGroup>::iterator end()
    {
      return m_groups.end();
    }
    
    std::vector<InvGroup>::const_iterator cbegin() const
    {
      return m_groups.cbegin();
    }
    
    std::vector<InvGroup>::const_iterator cend() const
    {
      return m_groups.cend();
    }
    
  };

}
