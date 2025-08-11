//
//  Inventory.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-08-19.
//

#pragma once
#include "Items.h"
#include "SaveGame.h"
#include <Termin8or/ScreenHandler.h>
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
    int selection_idx = 0;
    int hilite_idx = 0;
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
      {
        int item_idx_at_selection = m_items[selection_idx].item_index;
        bool was_selected = m_items[selection_idx].selected;
        m_items[selection_idx].selected = false;
        int item_idx_at_hilite = m_items[hilite_idx].item_index;
        bool was_hilited = m_items[hilite_idx].hilited;
        m_items[hilite_idx].hilited = false;
        
        stlutils::sort(m_items, [](const auto& iA, const auto& iB)
          {
            return iA.item_index < iB.item_index;
          });
          
        selection_idx = stlutils::find_if_idx(m_items,
          [item_idx_at_selection](const auto& i) { return i.item_index == item_idx_at_selection; });
        m_items[selection_idx].selected = was_selected;
        hilite_idx = stlutils::find_if_idx(m_items,
          [item_idx_at_hilite](const auto& i) { return i.item_index == item_idx_at_hilite; });
        m_items[hilite_idx].hilited = was_hilited;
        invalidated = true;
      }
    }
    
    int get_hilite_idx() const
    {
      return use_title + hilite_idx;
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
    
    void toggle_state(int idx, InvItemState state)
    {
      int rel_idx = idx - use_title;
      
      bool same_selection = selection_idx == rel_idx;
    
      if (state == InvItemState::SWITCH_SELECTION)
      {
        if (math::in_range<int>(selection_idx, 0, stlutils::sizeI(m_items), Range::ClosedOpen))
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
        
      if (math::in_range<int>(rel_idx, 0, stlutils::sizeI(m_items), Range::ClosedOpen))
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
    
    int calc_new_hilite_idx()
    {
      int cum_idx = 1; // Cumulative index. Excludes the title.
      for (auto& sg : m_subgroups)
      {
        if (sg.is_invalidated())
        {
          int hilite_idx = cum_idx + sg.get_hilite_idx();
          sg.reset_invalidation();
          return hilite_idx;
        }
        cum_idx += sg.size();
      }
      return -1;
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
    
    // Save-game temporaries for delayed application.
    std::vector<int> sg_hilited_idcs, sg_selected_idcs;
    
  public:
    void set_bounding_box(const ttl::Rectangle& bb) { m_bb = bb; }
    
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
    
    void apply_invalidated()
    {
      int cum_idx = 0; // Cumulative index.
      for (auto& g : m_groups)
      {
        auto rel_idx = g.calc_new_hilite_idx();
        if (rel_idx != -1)
        {
          toggle_state(hilite_idx, InvItemState::RESET_HILITE);
          hilite_idx = cum_idx + rel_idx;
          toggle_state(hilite_idx, InvItemState::SET_HILITE);
          break;
        }
        cum_idx += g.size();
      }
      
      // #NOTE: Can occur after deserialization. So commenting out for now.
      //for (auto& g : m_groups)
      //  if (g.is_invalidated())
      //    std::cerr << "ERROR in Inventory::apply_invalidated() : Not suppose to have multiply invalidated groups!\n";
    }
    
    void apply_deserialization_changes()
    {
      for (auto idx : sg_hilited_idcs) // #NOTE: Should be just one index.
      {
        toggle_state(hilite_idx, InvItemState::RESET_HILITE);
        hilite_idx = idx;
        toggle_state(hilite_idx, InvItemState::SET_HILITE);
      }
        
      for (auto idx : sg_selected_idcs) // #NOTE: Should be just one index.
        toggle_state(idx, InvItemState::SWITCH_SELECTION);
        
      sg_hilited_idcs.clear();
      sg_selected_idcs.clear();
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
    
    void reset_hilite()
    {
      toggle_state(hilite_idx, InvItemState::RESET_HILITE);
    }
      
    template<int NR, int NC>
    void draw(ScreenHandler<NR, NC>& sh) const
    {
      sh.write_buffer(str::adjust_str("Inventory", str::Adjustment::Center, m_bb.c_len), m_bb.top() + rb0_title, m_bb.left(), Color::White, Color::Transparent2);
      
      int num_lines = size();
      
      int r_offs = std::max(0, hilite_idx - (m_bb.r_len - rb0_items - 2));
        
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
        if (math::in_range<int>(rb_items, rb0_items, m_bb.r_len - 2, Range::Closed))
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
    
    void serialize(std::vector<std::string>& lines) const
    {
      sg::write_var(lines, SG_WRITE_VAR(hilite_idx));
      
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
        if (sg::read_var(&it_line, SG_READ_VAR(hilite_idx))) {}
        else if (*it_line == "items")
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
