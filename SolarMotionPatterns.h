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
  enum class Latitude { NorthPole, NorthernHemisphere, Equator, SouthernHemisphere, SouthPole };
  enum class Longitude
  {
    Front, // -45° to 45° (45W to 45E)
    East,  // 45° to 135° (45E to 135E)
    Back,  // 135° to 225° (135E to 135W)
    West   // 225° to 315° (135W to 45W)
  };
  // These are global seasons and uses the northern hemisphere as a reference frame.
  enum class Season { Winter, EarlySpring, Spring, EarlySummer, Summer, LateSummer, Autumn, LateAutumn };
  
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
  
    SolarDirection get_solar_direction(Latitude latitude, Season season, float sun_t)
    {
      int idx = math::roundI((c_num_phases - 1)*sun_t);
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
          }
      }
    }
  };
  
};
