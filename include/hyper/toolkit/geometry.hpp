/** ////////////////////////////////////////////////////////////////

    *** Hyper C++ - A simplified C++ experience ***

        Yet (another) open source library for C++

        Original Copyright (C) Damian Tran 2019

        By aiFive Technologies, Inc. for developers

    Copying and redistribution of this code is freely permissible.
    Inclusion of the above notice is preferred but not required.

    This software is provided AS IS without any expressed or implied
    warranties.  By using this code, and any modifications and
    variants arising thereof, you are assuming all liabilities and
    risks that may be thus associated.

////////////////////////////////////////////////////////////////  **/

#pragma once

#ifndef TOOLKIT_GEOMETRY
#define TOOLKIT_GEOMETRY

#include <vector>
#include <cmath>

namespace hyperC
{

/** @brief Obtain a list of coordinates in n-dimensional space
  * corresponding to the 0-point origins of evenly-spaced volumes.
  *
  * The algorithm attempts to create [num_volumes] square-shaped
  * regions that maximally and uniformly fill an n-dimensional
  * space bounded by dimensions [n_volume].
  *
  * NOTE: The position and the volume must have the same dimensionality.
  *
  * @param n_position   The position of the bounding box [x,y,z,...].
  * @param n_volume The dimensions of the bounding box [x,y,z,...].
  * @param num_volumes  The number of vertex points to search for.
  * @param vertices Storage matrix for n-dimensional vertex coordinates.
  * @param volume   Optional storage vector for the uniform n-dimensional
  * volume.
  */
template<typename numeric_t>
void get_uniform_volumes(const std::vector<numeric_t>& n_position,
                         const std::vector<numeric_t>& n_volume,
                         const size_t& num_volumes,
                         std::vector<std::vector<numeric_t>>* vertices,
                         std::vector<numeric_t>* volume = nullptr)
{

    if(n_position.empty() ||
       n_volume.empty())
    {
        throw std::invalid_argument("hyperC::get_uniform_volumes: position or volume vectors were empty");
    }
    else if(n_position.size() != n_volume.size())
    {
        throw std::invalid_argument("hyperC::get_uniform_volumes: position and volume dimensions do not match");
    }

    if(num_volumes < 1)
    {
        return;
    }
    else if(num_volumes == 1)
    {
        vertices->emplace_back(n_position);
        if(volume)
        {
            *volume = n_volume;
        }
        return;
    }

    size_t N = n_volume.size();

    // Identify relative size of n-dimensional bounding space
    std::vector<numeric_t> fieldRatios = n_volume;
    numeric_t totalRatio = 0.0;
    for(auto& depth : n_volume)
    {
        totalRatio += depth;
    }

    for(auto& ratio : fieldRatios)
    {
        ratio /= totalRatio;
    }

    // Calculate total volume
    numeric_t t_volume = 1;
    for(auto& dimension : n_volume)
    {
        t_volume *= dimension;
    }

    // Determine the maximum volume per neuron
    numeric_t volume_unit = t_volume/num_volumes;

    // Determine n-dimensional integer lengths with volume minimally filling volume

    numeric_t cs = 0.0;
    std::vector<numeric_t> unit_square(n_volume.size(), 1.0);

    while(cs < volume_unit)
    {

        for(size_t d = 0; (d < unit_square.size()) && (cs < volume_unit); ++d)
        {
            cs = 1;

            if(unit_square[d] + 1 < n_volume[d])
            {

                ++unit_square[d];
                for(auto& length : unit_square)
                {
                    cs *= length;
                }

                if(cs > volume_unit)
                {
                    if(!unit_square[d])
                    {
                        break;
                    }
                    --unit_square[d];
                }
            }
        }

    }

    // Create a coordinate map containing allowed indices for each dimension

    std::vector<std::vector<numeric_t>> coord_map(n_volume.size());
    size_t nCoords = 1;

    for(size_t d = 0; d < n_volume.size(); ++d)
    {

        size_t i = 0;
        size_t L = n_volume[d]/unit_square[d];

        coord_map[d].resize(L, 1.0);

        for(numeric_t n = n_position[d]; (n < n_volume[d]) && (i < L); n += unit_square[d], ++i)
        {
            coord_map[d][i] = n;
        }

        nCoords *= L;
    }

    // Melt the coordinate map

    std::vector<std::vector<numeric_t>> coord_transpose(nCoords, std::vector<numeric_t>(n_volume.size(), 1.0));

    for(int d = 0; d < (int)n_volume.size(); ++d)
    {
        size_t period = 1;
        size_t i = 0;
        size_t j;

        for(int d0 = d - 1; d0 >= 0; --d0)
        {
            period *= coord_map[d0].size();
        }

        while(i < nCoords)
        {
            for(auto& val : coord_map[d])
            {
                for(j = 0; (j < period) && (i < nCoords); ++i, ++j)
                {
                    coord_transpose[i][d] = val;
                }
            }
        }
    }

    // Store the vertices
    vertices->resize(num_volumes);
    for(size_t i = 0; (i < nCoords) && (i < num_volumes); ++i)
    {
        (*vertices)[i] = coord_transpose[i];
    }

    if(volume)
    {
        *volume = unit_square;
    }

}

}

#endif // TOOLKIT_GEOMETRY
