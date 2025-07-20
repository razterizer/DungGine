//
//  Comparison.h
//  DungGine
//
//  Created by Rasmus Anthin on 2025-07-20.
//

#pragma once

template<typename T>
struct PtrLess
{
  bool operator()(const T* lhs, const T* rhs) const
  {
    return lhs->id < rhs->id; // or compare by position, bounding box, etc.
  }
};

template<typename T>
struct PtrPairLess
{
  bool operator()(const std::pair<T*, T*>& a,
                  const std::pair<T*, T*>& b) const
  {
    if (a.first->id == b.first->id)
        return a.second->id < b.second->id;
    return a.first->id < b.first->id;
  }
};
