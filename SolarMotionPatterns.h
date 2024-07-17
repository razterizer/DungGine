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
  enum class Latitude { NorthPole, NorthernHemisphere, Equator, SouthernHemisphere, SouthPole, NUM_ITEMS };
  enum class Longitude
  {
    F,     // 0° (Front)
    FFE,   // 22.5° (Front Front East)
    FE,    // 45° (Front East)
    EFE,   // 67.5° (East Front East)
    E,     // 90° (East)
    EBE,   // 112.5° (East Back East)
    BE,    // 135° (Back East)
    BBE,   // 157.5° (Back Back East)
    B,     // 180° (Back)
    BBW,   // -157.5° (Back Back West)
    BW,    // -135° (Back West)
    WBW,   // -112.5° (West Back West)
    W,     // -90° (West)
    WFW,   // -67.5° (West Front West)
    FW,    // -45° (Front West)
    FFW,   // -22.5° (Front Front West)
    NUM_ITEMS
  };
  // These are global seasons and uses the northern hemisphere as a reference frame.
  enum class Season { Winter, EarlySpring, Spring, EarlySummer, Summer, LateSummer, Autumn, LateAutumn, NUM_ITEMS };
  
  class SolarMotionPatterns
  {
    static constexpr int c_num_phases = 16;
    
    // Well, you can't really have north on the north pole and vice versa,
    //   but since north maps to the up direction in the world,
    //   it is a hack that works.
    
    // North Pole : Winter, Spring/Autumn, Summer.
    const std::array<SolarDirection, c_num_phases> np_winter
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
    
    const std::array<SolarDirection, c_num_phases> np_spring_autumn
    {
      SolarDirection::E_Low,
      SolarDirection::E_Low,
      SolarDirection::SE_Low,
      SolarDirection::SE_Low,
      SolarDirection::S_Low,
      SolarDirection::S_Low,
      SolarDirection::SW_Low,
      SolarDirection::SW_Low,
      SolarDirection::W_Low,
      SolarDirection::W_Low,
      SolarDirection::NW_Low,
      SolarDirection::NW_Low,
      SolarDirection::N_Low,
      SolarDirection::N_Low,
      SolarDirection::NE_Low,
      SolarDirection::NE_Low,
    };
    
    const std::array<SolarDirection, c_num_phases> np_summer
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
    
    // Northern Hemisphere : Winter, Spring/Autumn, Summer.
    const std::array<SolarDirection, c_num_phases> nh_winter
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
    
    const std::array<SolarDirection, c_num_phases> nh_spring_autumn
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
    
    const std::array<SolarDirection, c_num_phases> nh_summer
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
    
    // Equator
    const std::array<SolarDirection, c_num_phases> equator
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
    
    // Southern Hemisphere : Winter, Spring/Autumn, Summer.
    const std::array<SolarDirection, c_num_phases> sh_winter
    {
      SolarDirection::E_Low,
      SolarDirection::E_Low,
      SolarDirection::E,
      SolarDirection::NE,
      SolarDirection::N,
      SolarDirection::N,
      SolarDirection::N,
      SolarDirection::N,
      SolarDirection::NW,
      SolarDirection::W,
      SolarDirection::W_Low,
      SolarDirection::W_Low,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
      SolarDirection::Nadir,
    };
    
    const std::array<SolarDirection, c_num_phases> sh_spring_autumn
    {
      SolarDirection::E_Low,
      SolarDirection::E,
      SolarDirection::NE,
      SolarDirection::N,
      SolarDirection::N,
      SolarDirection::NW,
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
    
    const std::array<SolarDirection, c_num_phases> sh_summer
    {
      SolarDirection::E_Low,
      SolarDirection::NE,
      SolarDirection::N,
      SolarDirection::NW,
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
    
    // South Pole : Winter, Spring/Autumn, Summer.
    const std::array<SolarDirection, c_num_phases> sp_winter
    {
      SolarDirection::E,
      SolarDirection::E,
      SolarDirection::NE,
      SolarDirection::NE,
      SolarDirection::N,
      SolarDirection::N,
      SolarDirection::NW,
      SolarDirection::NW,
      SolarDirection::W,
      SolarDirection::W,
      SolarDirection::SW,
      SolarDirection::SW,
      SolarDirection::S,
      SolarDirection::S,
      SolarDirection::SE,
      SolarDirection::SE,
    };
    
    const std::array<SolarDirection, c_num_phases> sp_spring_autumn
    {
      SolarDirection::E_Low,
      SolarDirection::E_Low,
      SolarDirection::NE_Low,
      SolarDirection::NE_Low,
      SolarDirection::N_Low,
      SolarDirection::N_Low,
      SolarDirection::NW_Low,
      SolarDirection::NW_Low,
      SolarDirection::W_Low,
      SolarDirection::W_Low,
      SolarDirection::SW_Low,
      SolarDirection::SW_Low,
      SolarDirection::S_Low,
      SolarDirection::S_Low,
      SolarDirection::SE_Low,
      SolarDirection::SE_Low,
    };
    
    const std::array<SolarDirection, c_num_phases> sp_summer
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
    
    
    
  public:
  
    SolarDirection get_solar_direction(Latitude latitude, Longitude longitude, Season season, float sun_t)
    {
      int long_offs = static_cast<int>(longitude);
      int idx = static_cast<int>(std::floor(c_num_phases*sun_t) + long_offs) % c_num_phases;
      switch (latitude)
      {
        case Latitude::NorthPole:
          switch (season)
          {
            case Season::Spring:
            case Season::Autumn:
              return np_spring_autumn[idx];
            case Season::LateAutumn:
            case Season::Winter:
            case Season::EarlySpring:
              return np_winter[idx];
            case Season::EarlySummer:
            case Season::Summer:
            case Season::LateSummer:
              return np_summer[idx];
            case Season::NUM_ITEMS: // Error state
              return SolarDirection::Nadir;
          }
        case Latitude::NorthernHemisphere:
          switch (season)
          {
            case Season::EarlySpring:
            case Season::Spring:
            case Season::EarlySummer:
            case Season::LateSummer:
            case Season::Autumn:
            case Season::LateAutumn:
              return nh_spring_autumn[idx];
            case Season::Winter:
              return nh_winter[idx];
            case Season::Summer:
              return nh_summer[idx];
            case Season::NUM_ITEMS: // Error state
              return SolarDirection::Nadir;
          }
        case Latitude::Equator:
          return equator[idx];
        case Latitude::SouthernHemisphere:
          switch (season)
          {
            case Season::EarlySpring:
            case Season::Spring:
            case Season::EarlySummer:
            case Season::LateSummer:
            case Season::Autumn:
            case Season::LateAutumn:
              return sh_spring_autumn[idx];
            case Season::Winter:
              return sh_winter[idx];
            case Season::Summer:
              return sh_summer[idx];
            case Season::NUM_ITEMS: // Error state
              return SolarDirection::Nadir;
          }
        case Latitude::SouthPole:
          switch (season)
          {
            case Season::Spring:
            case Season::Autumn:
              return sp_spring_autumn[idx];
            case Season::LateAutumn:
            case Season::Winter:
            case Season::EarlySpring:
              return sp_winter[idx];
            case Season::EarlySummer:
            case Season::Summer:
            case Season::LateSummer:
              return sp_summer[idx];
            case Season::NUM_ITEMS: // Error state
              return SolarDirection::Nadir;
          }
        case Latitude::NUM_ITEMS: // Error state
          return SolarDirection::Nadir;
      }
    }
  };
  
};
