/**********************************************************************************************************************
This file is part of the Control Toolbox (https://adrlab.bitbucket.io/ct), copyright by ETH Zurich, Google Inc.
Authors:  Michael Neunert, Markus Giftthaler, Markus Stäuble, Diego Pardo, Farbod Farshidian
Licensed under Apache2 license (see LICENSE file in main directory)
**********************************************************************************************************************/

#pragma once

#include "EstimatorBase.h"

namespace ct {
namespace optcon {

template <size_t STATE_DIM_T, typename SCALAR = double>
class ExtendedKalmanFilter : public EstimatorBase<STATE_DIM_T, SCALAR>
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    ExtendedKalmanFilter(
        const ct::core::StateVector<STATE_DIM_T, SCALAR>& x0 = ct::core::StateVector<STATE_DIM_T, SCALAR>::Zero(),
        const Eigen::Matrix<SCALAR, STATE_DIM_T, STATE_DIM_T>& P0 = Eigen::Matrix<SCALAR, STATE_DIM_T, STATE_DIM_T>::Identity())
        : EstimatorBase<STATE_DIM_T, SCALAR>(x0), P_(P0)
    {
    }

    template <size_t CONTROL_DIM>
    const ct::core::StateVector<STATE_DIM_T, SCALAR>& predict(SystemModelBase<STATE_DIM_T, CONTROL_DIM, SCALAR>& f,
        const Eigen::Matrix<SCALAR, CONTROL_DIM, 1>& u,
        const Eigen::Matrix<SCALAR, STATE_DIM_T, STATE_DIM_T>& Q,
        ct::core::Time t = 0)
    {
        f.updateJacobians(this->x_, u, t);
        this->x_ = f.computeDynamics(this->x_, u, t);
        P_       = (f.dFdx() * P_ * f.dFdx().transpose()) + f.dFdv() * Q * f.dFdv().transpose();
        return this->x_;
    }

    template <size_t OBS_DIM>
    const ct::core::StateVector<STATE_DIM_T, SCALAR>& update(const Eigen::Matrix<SCALAR, OBS_DIM, 1>& y,
        MeasurementModelBase<OBS_DIM, STATE_DIM_T, SCALAR>& h,
        const Eigen::Matrix<SCALAR, OBS_DIM, OBS_DIM>& R)
    {
        h.updateJacobians(this->x_);
        const Eigen::Matrix<SCALAR, STATE_DIM_T, OBS_DIM> K =
            P_ * h.dHdx().transpose() *
            (h.dHdx() * P_ * h.dHdx().transpose() + h.dHdw() * R * h.dHdw().transpose()).inverse();

        this->x_ += K * (y - h.computeMeasurement(this->x_));
        P_ -= (K * h.dHdx() * P_).eval();

        return this->x_;
    }

private:
    Eigen::Matrix<SCALAR, STATE_DIM_T, STATE_DIM_T> P_;
};

}  // optcon
}  // ct
