#ifndef ANTENNA_ARRAY_H
#define ANTENNA_ARRAY_H

#include <complex>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "constants.h"

struct antenna_array
{
    // positions of antenna elements in local frame of antenna
    std::vector<glm::vec3> element_pos_m;

    // alpha of the elements
    double element_alpha;

    // unused for now, in case plane of antenna is not tangent to boresight of each element
    // glm::vec3 ant_elem_boresight

    double frequency_hz;
};

// std::vector<std::complex<double>> 

std::complex<double> compute_array_factor(antenna_array& ant, glm::vec3 steer_uvw, glm::vec3 eval_uvw)
{
    double k = 2*glm::pi<double>() / ant.frequency_hz;

    std::complex<double> sum = 0;

    for (auto& pos : ant.element_pos_m)
    {
        
    }
}

#endif