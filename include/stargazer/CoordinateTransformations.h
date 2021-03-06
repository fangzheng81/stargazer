//
// This file is part of the stargazer library.
//
// Copyright 2016 Claudio Bandera <claudio.bandera@kit.edu (Karlsruhe Institute of Technology)
//
// The stargazer library is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// The stargazer library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <iostream>
#include "StargazerTypes.h"
#include "ceres/rotation.h"

namespace stargazer {

template <typename T>
/**
 * @brief This function will transform a point, given in landmark coordinates into world coordinates
 *
 * @param x_in  x value of input point in landmark coordinates
 * @param y_in  y value of input point in landmark coordinates
 * @param lm_pose the six dimensional pose of the landmark (rotation in rodriguez angles)
 * @param x_out x value of ouput point in world coordinates
 * @param y_out y value of ouput point in world coordinates
 * @param z_out z value of ouput point in world coordinates
 */
void transformLM2World(const T* const x_in, const T* const y_in, const T* const lm_pose, T* const x_out, T* const y_out,
                       T* const z_out) {
    // Create point in Landmark coordinates
    T p_lm[3];
    p_lm[0] = *x_in;
    p_lm[1] = *y_in;
    p_lm[2] = T(0.);

    // Transform point from Landmark to world coordinates
    T p_w[3];
    T angleAxisLM[3];
    angleAxisLM[0] = lm_pose[(int)POSE::Rx];
    angleAxisLM[1] = lm_pose[(int)POSE::Ry];
    angleAxisLM[2] = lm_pose[(int)POSE::Rz];

    ceres::AngleAxisRotatePoint(&angleAxisLM[0], &p_lm[0], &p_w[0]);

    // lm_pose[0,1,2] are the translation.
    p_w[0] += lm_pose[(int)POSE::X];
    p_w[1] += lm_pose[(int)POSE::Y];
    p_w[2] += lm_pose[(int)POSE::Z];

    *x_out = p_w[0];
    *y_out = p_w[1];
    *z_out = p_w[2];
}

template <typename T>
/**

 * @brief This function will transform a point, given in world coordinates into image coordinates
 *
 * @param x_in  x value of input point in world coordinates
 * @param y_in  y value of input point in world coordinates
 * @param z_in  z value of input point in world coordinates
 * @param camera_pose the six dimensional pose of the camera (rotation in rodriguez angles)
 * @param camera_intrinsics the cameras intrinsic parameters
 * @param x_out x value of ouput point in image coordinates
 * @param y_out y value of ouput point in image coordinates
 */
void transformWorld2Img(const T* const x_in, const T* const y_in, const T* const z_in, const T* const camera_pose,
                        const T* const camera_intrinsics, T* const x_out, T* const y_out) {
    // Create point in wolrd coordinates
    T p_w[3];
    p_w[0] = *x_in;
    p_w[1] = *y_in;
    p_w[2] = *z_in;

    // Transform point to camera coordinates
    T p_c[3];

    // This time we go from world -> cam
    // camera_pose[3,4,5] are the translation.
    p_c[0] = p_w[0] - camera_pose[(int)POSE::X];
    p_c[1] = p_w[1] - camera_pose[(int)POSE::Y];
    p_c[2] = p_w[2] - camera_pose[(int)POSE::Z];
    T angleAxisCam[3];
    angleAxisCam[0] = -camera_pose[(int)POSE::Rx];
    angleAxisCam[1] = -camera_pose[(int)POSE::Ry];
    angleAxisCam[2] = -camera_pose[(int)POSE::Rz];
    ceres::AngleAxisRotatePoint(&angleAxisCam[0], &p_c[0], &p_c[0]);

    /* Transform point to image coordinates
    intrinsics[0] is focal length
    intrinsics[1] is x0
    intrinsics[2] is y0
    intrinsics[3] is alpha
    intrinsics[4] is beta
    intrinsics[5] is theta --> assumed 90° */
    T p_i[3];
    p_i[0] = camera_intrinsics[(int)INTRINSICS::f] * camera_intrinsics[(int)INTRINSICS::alpha] * p_c[0] +
             T(0) * p_c[1] + camera_intrinsics[(int)INTRINSICS::u0] * p_c[2];
    p_i[1] = T(0) * p_c[0] +
             camera_intrinsics[(int)INTRINSICS::f] * camera_intrinsics[(int)INTRINSICS::beta] * p_c[1] / T(1) +
             camera_intrinsics[(int)INTRINSICS::v0] * p_c[2];
    p_i[2] = p_c[2];
    if (p_i[2] == T(0)) {
        std::cout << "WARNING; Attempt to divide by 0!" << std::endl;
        return;
    }
    p_i[0] = p_i[0] / p_i[2];
    p_i[1] = p_i[1] / p_i[2];
    p_i[2] = p_i[2] / p_i[2];

    *x_out = p_i[0];
    *y_out = p_i[1];
}

template <typename T>
/**
 * @brief This function will transform a point, given in landmark coordinates into image coordinates
 *
 * @param x_in  x value of input point in landmark coordinates
 * @param y_in  y value of input point in landmark coordinates
 * @param lm_pose the six dimensional pose of the landmark (rotation in rodriguez angles)
 * @param camera_pose the six dimensional pose of the camera (rotation in rodriguez angles)
 * @param camera_intrinsics the cameras intrinsic parameters
 * @param x_out x value of ouput point in image coordinates
 * @param y_out y value of ouput point in image coordinates
 */
void transformLM2Img(const T* const x_in, const T* const y_in, const T* const lm_pose, const T* const camera_pose,
                     const T* const camera_intrinsics, T* const x_out, T* const y_out) {
    T x_w, y_w, z_w;
    transformLM2World(x_in, y_in, lm_pose, &x_w, &y_w, &z_w);
    transformWorld2Img(&x_w, &y_w, &z_w, camera_pose, camera_intrinsics, x_out, y_out);
}

} // namespace stargazer
