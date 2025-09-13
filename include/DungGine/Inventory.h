//
//  Inventory.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-08-19.
//

#pragma once
#include "Items.h"
#include "SaveGame.h"
#include <Termin8or/screen/ScreenHandler.h>
#include <Termin8or/drawing/Drawing.h>
#include <Termin8or/geom/Rectangle.h>

namespace dung
{

  // Hilite is global and selection is local per sub-group.
  // Thus there are only one hilite index but several selection indices,
  //   max one per sub-group.

  struct InvItem
  {
    std::string text;
    // level = 0: title, level = 1: sub-title, level = 2: item.
    int level = 2;
    bool selected = false;
    bool hilited = false;
    Item* item = nullptr;
    int item_index = -1;
    
    InvItem() = default;
    InvItem(const std::string& t)
      : text(t)
    {}
    InvItem(const std::string& t, Item* i, int i_idx)
      : text(t)
      , item(i)
      , item_index(i_idx)
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
    bool sort_items = false;
    bool invalidated = false;
    
  public:
    void set_sort_mode(bool enable_sort_items)
    {
      sort_items = enable_sort_items;
    }
    
    bool is_invalidated() const { return invalidated; }
    void reset_invalidation() { invalidated = false; }
  
    void add_item(const std::string& text, Item* item = nullptr, int idx = -1)
    {
      m_items.emplace_back(text, item, idx);
      
      if (sort_items)
        stlutils::sort(m_items, [](const auto& iA, const auto& iB)
          {
            return iA.item_index < iB.item_index;
          });
    }
    
    int find_hilited_index() const
    {
      int num_items = stlutils::sizeI(m_items);
      for (int i_idx = 0; i_idx < num_items; ++i_idx)
      {
        if (m_items[i_idx].hilited)
          return use_title + i_idx;
      }
      return -1;
    }
    
    int find_selected_index() const
    {
      int num_items = stlutils::sizeI(m_items);
      for (int i_idx = 0; i_idx < num_items; ++i_idx)
      {
        if (m_items[i_idx].selected)
          return use_title + i_idx;
      }
      return -1;
    }
    
    bool set_hilited_state(int idx, bool enable_hilite)
    {
      if (idx == -1)
        return false;
      int rel_idx = idx - use_title;
      if (math::in_range<int>(rel_idx, 0, stlutils::sizeI(m_items), Range::ClosedOpen))
      {
        m_items[rel_idx].hilited = enable_hilite;
        return true;
      }
      return false;
    }
    
    bool set_selected_state(int idx, bool enable_select)
    {
      if (idx == -1)
        return false;
      int rel_idx = idx - use_title;
      if (math::in_range<int>(rel_idx, 0, stlutils::sizeI(m_items), Range::ClosedOpen))
      {
        m_items[rel_idx].selected = enable_select;
        return true;
      }
      return false;
    }
    
    bool get_hilited_state(int idx) const
    {
      if (idx == -1)
        return false;
      int rel_idx = idx - use_title;
      if (math::in_range<int>(rel_idx, 0, stlutils::sizeI(m_items), Range::ClosedOpen))
        return m_items[rel_idx].hilited;
      return false;
    }
    
    bool get_selected_state(int idx) const
    {
      if (idx == -1)
        return false;
      int rel_idx = idx - use_title;
      if (math::in_range<int>(rel_idx, 0, stlutils::sizeI(m_items), Range::ClosedOpen))
        return m_items[rel_idx].selected;
      return false;
    }
    
    bool toggle_hilited_selection()
    {
      int hilite_idx = find_hilited_index();
      if (hilite_idx != -1)
      {
        int selection_idx = find_selected_index();
        // if same index then toggle selection state, otherwise transfer.
        if (selection_idx != hilite_idx)
          set_selected_state(selection_idx, false);

        bool selected = get_selected_state(hilite_idx);
        set_selected_state(hilite_idx, !selected);
        return true;
      }
      return false;
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
      if (math::in_range<int>(rel_idx, 0, stlutils::sizeI(m_items), Range::ClosedOpen))
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
    InvItem* get_hilited_item()
    {
      auto it = stlutils::find_if(m_items, [](const auto& ii) { return ii.hilited; });
      if (it != m_items.end())
        return &(*it);
      return nullptr;
    }
    
    int size() const { return use_title + stlutils::sizeI(m_items); }
    
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
    
    void set_sort_mode(bool enable_sort_items)
    {
      for (auto& sg : m_subgroups)
        sg.set_sort_mode(enable_sort_items);
    }
    
    bool is_invalidated() const
    {
      for (const auto& sg : m_subgroups)
        if (sg.is_invalidated())
          return true;
      return false;
    }
    
    int find_hilited_index() const
    {
      int cum_idx = 1;
      for (const auto& sg : m_subgroups)
      {
        auto rel_idx = sg.find_hilited_index();
        if (rel_idx != -1)
          return cum_idx + rel_idx;
        cum_idx += sg.size();
      }
      return -1;
    }
    
    bool set_hilited_state(int idx, bool enable_hilite)
    {
      if (idx == -1)
        return false;
      int cum_idx = 1; // Cumulative index.
      for (auto& sg : m_subgroups)
      {
        int rel_idx = idx - cum_idx;
        if (math::in_range<int>(rel_idx, 0, sg.size(), Range::ClosedOpen))
          if (sg.set_hilited_state(rel_idx, enable_hilite))
            return true;
        cum_idx += sg.size();
      }
      return false;
    }
    
    bool set_selected_state(int idx, bool enable_select)
    {
      if (idx == -1)
        return false;
      int cum_idx = 1; // Cumulative index.
      for (auto& sg : m_subgroups)
      {
        int rel_idx = idx - cum_idx;
        if (math::in_range<int>(rel_idx, 0, sg.size(), Range::ClosedOpen))
          if (sg.set_selected_state(rel_idx, enable_select))
            return true;
        cum_idx += sg.size();
      }
      return false;
    }
    
    bool get_hilited_state(int idx) const
    {
      if (idx == -1)
        return false;
      int cum_idx = 1; // Cumulative index.
      for (auto& sg : m_subgroups)
      {
        int rel_idx = idx - cum_idx;
        if (math::in_range<int>(rel_idx, 0, sg.size(), Range::ClosedOpen))
          return sg.get_hilited_state(rel_idx);
        cum_idx += sg.size();
      }
      return false;
    }
    
    bool get_selected_state(int idx) const
    {
      if (idx == -1)
        return false;
      int cum_idx = 1; // Cumulative index.
      for (auto& sg : m_subgroups)
      {
        int rel_idx = idx - cum_idx;
        if (math::in_range<int>(rel_idx, 0, sg.size(), Range::ClosedOpen))
          return sg.get_selected_state(rel_idx);
        cum_idx += sg.size();
      }
      return false;
    }
    
    bool toggle_hilited_selection()
    {
      for (auto& sg : m_subgroups)
        if (sg.toggle_hilited_selection())
          return true;
      return false;
    }
    
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
      if (math::in_range<int>(subgroup_idx, 0, stlutils::sizeI(m_subgroups), Range::ClosedOpen))
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
    t8::Rectangle m_bb;
    
    int rb0_title = 2;
    int rb0_items = 4; // row box-relative-pos start items.
    int cb0_items = 2; // col box-relative-pos start items.
    
    // Save-game temporaries for delayed application.
    std::vector<int> sg_hilited_idcs, sg_selected_idcs;
    
    int cached_hilited_index = 0; // Not source of truth.
    
  public:
    void set_bounding_box(const t8::Rectangle& bb) { m_bb = bb; }
    
    void set_sort_mode(bool enable_sort_items)
    {
      for (auto& g : m_groups)
        g.set_sort_mode(enable_sort_items);
    }
    
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
    
    int find_hilited_index() const
    {
      int cum_idx = 0;
      for (const auto& g : m_groups)
      {
        auto rel_idx = g.find_hilited_index();
        if (rel_idx != -1)
          return cum_idx + rel_idx;
        cum_idx += g.size();
      }
      return -1;
    }
    
    bool set_hilited_state(int idx, bool enable_hilite)
    {
      if (idx == -1)
        return false;
      int cum_idx = 0; // Cumulative index.
      for (auto& g : m_groups)
      {
        int rel_idx = idx - cum_idx;
        if (math::in_range<int>(rel_idx, 0, g.size(), Range::ClosedOpen))
          if (g.set_hilited_state(rel_idx, enable_hilite))
            return true;
        cum_idx += g.size();
      }
      return false;
    }
    
    bool set_selected_state(int idx, bool enable_select)
    {
      if (idx == -1)
        return false;
      int cum_idx = 0; // Cumulative index.
      for (auto& g : m_groups)
      {
        int rel_idx = idx - cum_idx;
        if (math::in_range<int>(rel_idx, 0, g.size(), Range::ClosedOpen))
          if (g.set_selected_state(rel_idx, enable_select))
            return true;
        cum_idx += g.size();
      }
      return false;
    }
    
    bool get_hilited_state(int idx) const
    {
      if (idx == -1)
        return false;
      int cum_idx = 0; // Cumulative index.
      for (auto& g : m_groups)
      {
        int rel_idx = idx - cum_idx;
        if (math::in_range<int>(rel_idx, 0, g.size(), Range::ClosedOpen))
          return g.get_hilited_state(rel_idx);
        cum_idx += g.size();
      }
      return false;
    }
    
    bool get_selected_state(int idx) const
    {
      if (idx == -1)
        return false;
      int cum_idx = 0; // Cumulative index.
      for (auto& g : m_groups)
      {
        int rel_idx = idx - cum_idx;
        if (math::in_range<int>(rel_idx, 0, g.size(), Range::ClosedOpen))
          return g.get_selected_state(rel_idx);
        cum_idx += g.size();
      }
      return false;
    }
    
    void apply_deserialization_changes()
    {
      for (auto idx : sg_hilited_idcs) // #NOTE: Should be just one index.
      {
        set_hilited_state(find_hilited_index(), false);
        set_hilited_state(idx, true);
      }
      
      if (!sg_selected_idcs.empty())
      {
        for (auto& g : *this)
          for (auto& sg : g)
            sg.set_selected_state(sg.find_selected_index(), false);
        
        for (auto idx : sg_selected_idcs)
          set_selected_state(idx, true);
      }
        
      sg_hilited_idcs.clear();
      sg_selected_idcs.clear();
    }
    
    void inc_hilite()
    {
      int hilite_idx = find_hilited_index();
      set_hilited_state(hilite_idx, false);
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
      set_hilited_state(hilite_idx, true);
    }
    
    void dec_hilite()
    {
      int hilite_idx = find_hilited_index();
      set_hilited_state(hilite_idx, false);
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
      set_hilited_state(hilite_idx, true);
    }
    
    bool toggle_hilited_selection()
    {
      for (auto& g : m_groups)
        if (g.toggle_hilited_selection())
          return true;
      return false;
    }
    
    void cache_hilited_index()
    {
      cached_hilited_index = find_hilited_index();
    }
    
    void reset_hilite()
    {
      int num_wraps = 0;
      int hilite_idx = cached_hilited_index;
      while (!set_hilited_state(hilite_idx, true) && num_wraps < 3)
      {
        hilite_idx++;
        if (hilite_idx >= size())
        {
          hilite_idx = 0;
          num_wraps++;
        }
      };
    }
      
    template<int NR, int NC>
    void draw(ScreenHandler<NR, NC>& sh) const
    {
      sh.write_buffer(str::adjust_str("Inventory", str::Adjustment::Center, m_bb.c_len), m_bb.top() + rb0_title, m_bb.left(), Color::White, Color::Transparent2);
      
      int num_lines = size();
      
      int hilite_idx = std::max(0, find_hilited_index());
      
      int r_offs = std::max(0, hilite_idx - (m_bb.r_len - rb0_items - 2));
        
      for (int r = 0; r < num_lines; ++r)
      {
        auto item = get_item(r);
        auto text = item.text;
        t8::Style style { Color::Default, Color::Transparent2 };
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
        if (math::in_range<int>(rb_items, rb0_items, m_bb.r_len - 2, Range::Closed))
          sh.write_buffer(text, m_bb.top() + rb_items, m_bb.left() + cb_items, style);
      }
      
      t8x::draw_box_outline(sh, m_bb, t8x::OutlineType::Line, { Color::White, Color::DarkGray });
      t8x::draw_box(sh, m_bb, { Color::White, Color::DarkGray }, ' ');
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
    
    void serialize(std::vector<std::string>& lines) const
    {
      lines.emplace_back("items");
      int num_lines = size();
      for (int r = 0; r < num_lines; ++r)
      {
        auto item = get_item(r);
        if (item.hilited || item.selected)
          lines.emplace_back(std::to_string(r) + " : " + std::to_string(item.hilited) + ", " + std::to_string(item.selected));
      }
      lines.emplace_back("-");
    }
    
    std::vector<std::string>::iterator deserialize(std::vector<std::string>::iterator it_line_begin,
                                                   std::vector<std::string>::iterator it_line_end)
    {
      for (auto it_line = it_line_begin; it_line != it_line_end; ++it_line)
      {
        if (*it_line == "items")
        {
          for (auto it_line2 = it_line + 1; it_line2 != it_line_end; ++it_line2)
          {
            if (*it_line2 == "-")
              return it_line2;
            else
            {
              auto tokens = str::tokenize(*it_line2, { ' ', ',', ':' });
              if (tokens.size() == 3)
              {
                int idx = std::atoi(tokens[0].c_str());
                bool hilited = static_cast<bool>(std::atoi(tokens[1].c_str()));
                bool selected = static_cast<bool>(std::atoi(tokens[2].c_str()));
                if (hilited)
                  sg_hilited_idcs.emplace_back(idx);
                if (selected)
                  sg_selected_idcs.emplace_back(idx);
              }
              else
              {
                std::cerr << "Error in deserialization of inventory item states!\n";
                return it_line2;
              }
            }
          }
        }
      }
      return it_line_end;
    }
    
  };

}
