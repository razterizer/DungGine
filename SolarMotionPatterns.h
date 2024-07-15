//
//  SolarMotionPatterns.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-07-15.
//

#pragma once
#include <Termin8or/Drawing.h>

namespace dung
{
  
  using SolarDirection = drawing::SolarDirection;
  enum class Latitude { Polar, High, Equator };
  enum class Season { Winter, EarlySpring, Spring, EarlySummer, Summer, LateSummer, Autumn, LateAutumn };
  
  class SolarMotionPatterns
  {
    static constexpr int c_num_phases = 16;
    // Well, you can't really have north on the north pole and vice versa,
    //   but since north maps to the up direction in the world,
    //   it is a hack that works.
    const std::array<SolarDirection, c_num_phases> sun_motion_polar_winter
    {
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
    };
    
    const std::array<SolarDirection, c_num_phases> sun_motion_polar_summer
    {
      SolarDirection::E,
      SolarDirection::E,
      SolarDirection::SE,
      SolarDirection::SE,
      SolarDirection::S,
      SolarDirection::S,
      SolarDirection::SW,
      SolarDirection::SW,
      SolarDirection::W,
      SolarDirection::W,
      SolarDirection::NW,
      SolarDirection::NW,
      SolarDirection::N,
      SolarDirection::N,
      SolarDirection::NE,
      SolarDirection::NE,
    };
    
    const std::array<SolarDirection, c_num_phases> sun_motion_high_winter
    {
      SolarDirection::E_Low,
      SolarDirection::SE,
      SolarDirection::S,
      SolarDirection::SW,
      SolarDirection::W_Low,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
    };
    
    const std::array<SolarDirection, c_num_phases> sun_motion_high_spring_autumn
    {
      SolarDirection::E_Low,
      SolarDirection::E,
      SolarDirection::SE,
      SolarDirection::S,
      SolarDirection::S,
      SolarDirection::SW,
      SolarDirection::W,
      SolarDirection::W_Low,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
    };
    
    const std::array<SolarDirection, c_num_phases> sun_motion_high_summer
    {
      SolarDirection::E_Low,
      SolarDirection::E_Low,
      SolarDirection::E,
      SolarDirection::SE,
      SolarDirection::S,
      SolarDirection::S,
      SolarDirection::S,
      SolarDirection::S,
      SolarDirection::SW,
      SolarDirection::W,
      SolarDirection::W_Low,
      SolarDirection::W_Low,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
    };
    
    const std::array<SolarDirection, c_num_phases> sun_motion_equator
    {
      SolarDirection::E_Low,
      SolarDirection::E,
      SolarDirection::E,
      SolarDirection::Zenith,
      SolarDirection::Zenith,
      SolarDirection::W,
      SolarDirection::W,
      SolarDirection::W_Low,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
    };
    
  public:
  
    SolarDirection get_solar_direction(Latitude latitude, Season season, float sun_t)
    {
      int idx = math::roundI(c_num_phases*sun_t);
      switch (latitude)
      {
        case Latitude::Polar:
          switch (season)
          {
            case Season::Winter:
            case Season::EarlySpring:
            case Season::Autumn:
            case Season::LateAutumn:
              return sun_motion_polar_winter[idx];
            case Season::Spring:
            case Season::EarlySummer:
            case Season::Summer:
            case Season::LateSummer:
              return sun_motion_polar_summer[idx];
          }
        case Latitude::High:
          switch (season)
          {
            case Season::EarlySpring:
            case Season::Spring:
            case Season::EarlySummer:
            case Season::LateSummer:
            case Season::Autumn:
            case Season::LateAutumn:
              return sun_motion_high_spring_autumn[idx];
            case Season::Winter:
              return sun_motion_high_winter[idx];
            case Season::Summer:
              return sun_motion_high_summer[idx];
          }
        case Latitude::Equator:
          return sun_motion_equator[idx];
      }
    }
  };
  
};
