/*
 * FilterBase.hpp
 *
 *  Created on: Feb 9, 2014
 *      Author: Bloeschm
 */

#ifndef FilterBase_HPP_
#define FilterBase_HPP_

#include <Eigen/Dense>
#include <iostream>
#include "kindr/rotations/RotationEigen.hpp"
#include <map>
#include "Prediction.hpp"

namespace LWF{

template<typename State>
class FilterState{
 public:
  typedef State mtState;
  typedef typename mtState::CovMat mtCovMat;
  static const unsigned int D_ = mtState::D_;
  mtCovMat cov_;
  mtState state_;
  double t_;
  FilterState(){
    t_ = 0.0;
  };
  virtual ~FilterState(){};
};

template<typename State>
class FilterBase{
 public:
  typedef State mtState;
  typedef typename mtState::CovMat mtCovMat;
  static const unsigned int D_ = mtState::D_;
  FilterState<State> safe_;
  FilterState<State> front_;
  bool validFront_;

  std::map<double,PredictionBase<mtState>*> predictionMeasMap_;
  std::map<double,PredictionBase<mtState>*> updateMeasMap_; //TODO: adapt
  FilterBase(){
    validFront_ = false;
  };
  virtual ~FilterBase(){};
  void updateSafe(){
    if(predictionMeasMap_.empty() || updateMeasMap_.empty()) return;
    double nextSafeTime = std::min(predictionMeasMap_.rbegin()->first,updateMeasMap_.rbegin()->first);
    if(front_.t_<=nextSafeTime && validFront_ && front_.t_>safe_.t_){
      safe_.state_ = front_.state_;
      safe_.cov_ = front_.cov_;
    }
    update(safe_.state_,safe_.cov_,nextSafeTime);
    clean(nextSafeTime);
  }
  void update(mtState& state, mtCovMat& cov,const double& tEnd){
    typename std::map<double,PredictionBase<mtState>>::iterator itPredictionMeas;
    typename std::map<double,PredictionBase<mtState>>::iterator itUpdateMeas; // Todo adapt
    itPredictionMeas = predictionMeasMap_.upper_bound(state.t_);
    itUpdateMeas = updateMeasMap_.upper_bound(state.t_);
    double tNext = state.t_;
    PredictionBase<mtState>* usedPredictionMeas;
    while(state.t_<tEnd){
      bool isFullUpdate = false;
      tNext = tEnd;
      if(itPredictionMeas!=predictionMeasMap_.end()){
        if(itPredictionMeas->first<tNext){
          tNext = itPredictionMeas->first;
        }
      }
      if(itUpdateMeas!=updateMeasMap_.end()){
        if(itUpdateMeas->first<=tNext){
          tNext = itUpdateMeas->first;
          isFullUpdate = true;
        }
      }
//      if(itPredictionMeas != predictionMeasMap_.end()){
        usedPredictionMeas = itPredictionMeas->second;
//      } else {
//        usedPredictionMeas = defaultPredictionMeas_;
//      }
      if(isFullUpdate){
//        predictAndUpdate(&usedPredictionMeas,&itUpdateMeas->second,tNext-t_);
      } else {
        usedPredictionMeas->predictEKF(state,cov,tNext);
      }
      itPredictionMeas = predictionMeasMap_.upper_bound(state.t_);
      itUpdateMeas = updateMeasMap_.upper_bound(state.t_);
    }
  }
  void clean(const double& t){
    while(!predictionMeasMap_.empty() && predictionMeasMap_.begin()->first<=t){
      predictionMeasMap_.erase(predictionMeasMap_.begin());
    }
    while(!updateMeasMap_.empty() && updateMeasMap_.begin()->first<=t){
      updateMeasMap_.erase(updateMeasMap_.begin());
    }
  }
};

}

#endif /* FilterBase_HPP_ */