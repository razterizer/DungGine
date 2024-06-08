//
//  BSPTree.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-06-08.
//

#pragma once
#include <Termin8or/RC.h>
#include <Termin8or/SpriteHandler.h>
#include <Core/Rand.h>
#include <Core/Math.h>
#include <array>
#include <memory>


namespace bsp
{

  enum class Orientation { Vertical, Horizontal };
  
  // Example:
  // ####################################################
  // #                                                  #
  // #                                                  #
  // #                                                  #
  // #                                                  #
  // #                                                  #
  // #                     Root                         #
  // #                                                  #
  // #                                                  #
  // #                                                  #
  // #                                                  #
  // #                                                  #
  // #                                                  #
  // ####################################################
  //
  // ####################################################
  // #                   |                              #
  // #                   |                              #
  // #                   | Vertical                     #
  // #                   | Wall / Split                 #
  // #                   |                              #
  // #                   |                              #
  // #     Child L       |           Child R            #
  // #                   |                              #
  // #                   |                              #
  // #                   |                              #
  // #                   |                              #
  // #                   |                              #
  // ####################################################
  // |-------------------*----------------------------->|
  //              fraction = 20 / 50 = 0.4
  //
  // ####################################################--
  // #                                                  # |
  // #                                                  # |
  // #                                                  # |
  // #                   Child U                        # |
  // #                                                  # |
  // #                                                  # |
  // #                                                  # |
  // #  Horizontal Wall / Split                         # |
  // #--------------------------------------------------# * fraction = 9 / 12
  // #                                                  # |          = 0.75
  // #                   Child D                        # |
  // #                                                  # v
  // ####################################################--
  //
  // So fractional lengths are calculated from left to right (column-wise)
  //   or up to down (row-wise).
  
  template<int NR, int NC>
  void draw_room(SpriteHandler<NR, NC>& sh,
                 int r, int c, int len_r, int len_c,
                 Text::Color fg_color, Text::Color bg_color)
  {
    // len_r = 3, len_c = 2
    // ###
    // # #
    // # #
    // ###
    //
    // len_r = 4, len_c = 4 (smallest room size by default)
    // #####
    // #   #
    // #   #
    // #   #
    // #####
    
    auto str_horiz = str::rep_char('#', len_c);
    sh.write_buffer(str_horiz, r, c, fg_color, bg_color);
    for (int i = r; i <= r + len_r; ++i)
    {
      sh.write_buffer("#", i, c, fg_color, bg_color);
      sh.write_buffer("#", i, c + len_c, fg_color, bg_color);
    }
    sh.write_buffer(str_horiz, r + len_r, c, fg_color, bg_color);
  }

  struct BSPNode final
  {
    // The current room will be split into two rooms along this direction.
    Orientation orientation = Orientation::Vertical;
    // Length fraction of current room along the direction of the other orientation.
    // If orientation = Vertical then the fraction will be along the horizontal
    //   axis.
    float split_fraction = 0.5f;
    int size_rows = 0;
    int size_cols = 0;
    
    // Padding across wall. wall_padding[0] is the padding on the side of
    //   child 0.
    std::array<float, 2> wall_padding { 0, 0 };
    
    std::array<std::unique_ptr<BSPNode>, 2> children;
    
    void generate(int min_room_length)
    {
      split_fraction = rnd::rand();
      int split_length_0 = 0;
      int split_length_1 = 0;
      switch (orientation)
      {
        case Orientation::Vertical:
          split_length_0 = math::roundI(size_cols * split_fraction);
          split_length_1 = size_cols - split_length_0;
          break;
        case Orientation::Horizontal:
          split_length_0 = math::roundI(size_rows * split_fraction);
          split_length_1 = size_rows - split_length_0;
          break;
      };
      
      auto f_set_ch_size = [&](BSPNode* ch, int split_len)
      {
        switch (orientation)
        {
          case Orientation::Vertical:
            ch->size_cols = split_len;
            ch->size_rows = size_rows;
            break;
          case Orientation::Horizontal:
            ch->size_rows = split_len;
            ch->size_cols = size_cols;
            break;
        }
      };
    
      auto child_orientation = static_cast<Orientation>(1 - static_cast<int>(orientation));
      if (split_length_0 >= min_room_length)
      {
        auto& ch_0 = children[0] = std::make_unique<BSPNode>();
        ch_0->orientation = child_orientation;
        f_set_ch_size(ch_0.get(), split_length_0);
        ch_0->generate(min_room_length);
      }
      if (split_length_1 >= min_room_length)
      {
        auto& ch_1 = children[1] = std::make_unique<BSPNode>();
        ch_1->orientation = child_orientation;
        f_set_ch_size(ch_1.get(), split_length_1);
        ch_1->generate(min_room_length);
      }
    }
    
    template<int NR, int NC>
    void draw_blueprint(SpriteHandler<NR, NC>& sh,
                        int r = 0, int c = 0,
                        Text::Color fg_color = Text::Color::Black,
                        Text::Color bg_color = Text::Color::Yellow) const
    {
      draw_room(sh, r, c, size_rows, size_cols, fg_color, bg_color);
      int ch_r = 0;
      int ch_c = 0;
      int ch0_r_len = 0;
      int ch0_c_len = 0;
      if (children[0])
      {
        ch_r = r;
        ch_c = c;
        ch0_r_len = children[0]->size_rows;
        ch0_c_len = children[0]->size_cols;
        children[0]->draw_blueprint(sh, ch_r, ch_c, fg_color, bg_color);
      }
      if (children[1])
      {
        switch (orientation)
        {
          case Orientation::Vertical:
            ch_r = r;
            ch_c = c + ch0_c_len;
            break;
          case Orientation::Horizontal:
            ch_r = r + ch0_r_len;
            ch_c = c;
            break;
        }
        children[1]->draw_blueprint(sh, ch_r, ch_c, fg_color, bg_color);
      }
    }
  };

  class BSPTree final
  {
    BSPNode root;
    
  public:
    BSPTree() = default;
    
    void generate(int world_size_rows, int world_size_cols,
                  Orientation first_split_orientation,
                  int min_room_length = 4)
    {
      root.orientation = first_split_orientation;
      root.size_rows = world_size_rows;
      root.size_cols = world_size_cols;
      root.generate(min_room_length);
    }
    
    template<int NR, int NC>
    void draw_blueprint(SpriteHandler<NR, NC>& sh,
                        int r0 = 0, int c0 = 0,
                        Text::Color fg_color = Text::Color::Black,
                        Text::Color bg_color = Text::Color::Yellow) const
    {
      root.draw_blueprint(sh, r0, c0, fg_color, bg_color);
    }
  };
  
}
