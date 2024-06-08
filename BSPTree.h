//
//  BSPTree.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-06-08.
//

#pragma once
#include <Termin8or/RC.h>
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
  };
  
}
