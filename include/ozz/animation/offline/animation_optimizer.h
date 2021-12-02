//----------------------------------------------------------------------------//
//                                                                            //
// ozz-animation is hosted at http://github.com/guillaumeblanc/ozz-animation  //
// and distributed under the MIT License (MIT).                               //
//                                                                            //
// Copyright (c) Guillaume Blanc                                              //
//                                                                            //
// Permission is hereby granted, free of charge, to any person obtaining a    //
// copy of this software and associated documentation files (the "Software"), //
// to deal in the Software without restriction, including without limitation  //
// the rights to use, copy, modify, merge, publish, distribute, sublicense,   //
// and/or sell copies of the Software, and to permit persons to whom the      //
// Software is furnished to do so, subject to the following conditions:       //
//                                                                            //
// The above copyright notice and this permission notice shall be included in //
// all copies or substantial portions of the Software.                        //
//                                                                            //
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR //
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   //
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    //
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER //
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    //
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        //
// DEALINGS IN THE SOFTWARE.                                                  //
//                                                                            //
//----------------------------------------------------------------------------//

#ifndef OZZ_OZZ_ANIMATION_OFFLINE_ANIMATION_OPTIMIZER_H_
#define OZZ_OZZ_ANIMATION_OFFLINE_ANIMATION_OPTIMIZER_H_

#include "ozz/animation/offline/export.h"
#include "ozz/base/containers/map.h"

namespace ozz {
namespace animation {

// Forward declare runtime skeleton type.
class Skeleton;
namespace offline {

// Forward declare offline animation type.
struct RawAnimation;

// Defines the class responsible of optimizing an offline raw animation
// instance. Optimization is performed using a key frame reduction technique. It
// deciamtes redundant / interpolable key frames, within error tolerances given
// as input. The optimizer takes into account for each joint the error
// generated on its whole child hierarchy. This allows for example to take into
// consideration the error generated on a finger when optimizing the shoulder. A
// small error on the shoulder can be magnified when propagated to the finger
// indeed.
// It's possible to override optimization settings for a joint. This implicitely
// have an effect on the whole chain, up to that joint. This allows for example
// to have aggressive optimization for a whole skeleton, except for the chain
// that leads to the hand if user wants it to be precise. Default optimization
// tolerances are set in order to favor quality over runtime performances and
// memory footprint.
class OZZ_ANIMOFFLINE_DLL AnimationOptimizer {
 public:
  // Initializes the optimizer with default tolerances (favoring quality).
  AnimationOptimizer();

  // Optimizes _input using *this parameters. _skeleton is required to evaluate
  // optimization error along joint hierarchy (see hierarchical_tolerance).
  // Returns true on success and fills _output animation with the optimized
  // version of _input animation.
  // *_output must be a valid RawAnimation instance.
  // Returns false on failure and resets _output to an empty animation.
  // See RawAnimation::Validate() for more details about failure reasons.
  bool operator()(const RawAnimation& _input, const Skeleton& _skeleton,
                  RawAnimation* _output) const;

  // Optimization settings.
  struct Setting {
    // Default settings
    Setting()
        : tolerance(1e-3f),  // 1mm
          distance(1e-1f)    // 10cm
    {}

    Setting(float _tolerance, float _distance)
        : tolerance(_tolerance), distance(_distance) {}

    // The maximum error that an optimization is allowed to generate on a whole
    // joint hierarchy.
    float tolerance;

    // The distance (from the joint) at which error is measured (if bigger that
    // joint hierarchy). This allows to emulate effect on skinning.
    float distance;
  };

  // Global optimization settings. These settings apply to all joints of the
  // hierarchy, unless overriden by joint specific settings.
  Setting setting;

  // Per joint override of optimization settings.
  typedef ozz::map<int, Setting> JointsSetting;
  JointsSetting joints_setting_override;

  // Optional observer class, used to report optimization algorithm steps and
  // progress.
  class Observer;
  Observer* observer;
};

// Observer interface.
class AnimationOptimizer::Observer {
 public:
  virtual ~Observer() {}

  struct Data {
    int iteration;                // Iteration number.
    int joint;                    // Joint number
    int type;                     // Track type (Translation, rotation, scale).
    float target_error;           // Target error value.
    float distance;               // Distance at which error is measured.
    int original_size;            // Original track size.
    int validated_size;           // Last validated track size.
    int candidate_size;           // Candidate track size.
    float own_tolerance;          // Current decimation tolerance.
    float own_error;              // Track own error metric .
    float hierarchy_error_ratio;  // Error ratio for track hierarchy.
    float optimization_delta;     // Optimization delta for this candidate.
  };

  virtual bool Push(const Data&) = 0;
};

// Defines the class responsible of stripping constant keyframes from an offline
// raw animation instance.
class AnimationConstantOptimizer {
 public:
  // Initializes the optimizer with default tolerances.
  AnimationConstantOptimizer();

  // Optimizes _input using *this parameters.
  // *_output must be a valid RawAnimation instance.
  // Returns false on failure and resets _output to an empty animation.
  // See RawAnimation::Validate() for more details about failure reasons.
  bool operator()(const RawAnimation& _input, RawAnimation* _output) const;

  // Translation tolerance in meters. Uses euclidean distance.
  float translation_tolerance;

  // Rotation tolerance, as the cosine of half tolerance angle.
  // Uses ozz::math::Quaternion::Compare functions.
  // This allows to provide small numbers that cos function would round to 1.
  float rotation_tolerance;

  // Scale tolerance. Uses euclidean distance.
  float scale_tolerance;
};
}  // namespace offline
}  // namespace animation
}  // namespace ozz
#endif  // OZZ_OZZ_ANIMATION_OFFLINE_ANIMATION_OPTIMIZER_H_
