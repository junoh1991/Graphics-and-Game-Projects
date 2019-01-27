/** CSci-4611 Assignment 2:  Car Soccer
 */

#ifndef BALL_H_
#define BALL_H_

#include <mingfx.h>

/// Small data structure for a ball
class Ball {
public:
    
    /// The constructor sets the radius and calls Reset() to start the ball at
    /// the center of the field
    Ball() : radius_(2.6) {
        Reset();
    }
    
    /// Nothing special needed in the constructor
    virtual ~Ball() {}

    
    void Reset() {
        position_ = Point3(0, 35-radius_, 0);
        velocity_ = Vector3(0,-25,5);
    }

    void Reset(Vector3 v){
        position_ = Point3(0, 35-radius_, 0);
        velocity_ = v;
    }

    float radius() { return radius_; }
    
    Point3 position() { return position_; }
    Vector3 velocity() { return velocity_;}

    void set_position(const Point3 &p) { position_ = p; }
    void set_positionX(const float &p) { position_[0] = p; }
    void set_positionY(const float &p) { position_[1] = p; }
    void set_positionZ(const float &p) { position_[2] = p; }


    void set_velocity(const Vector3 &v) { velocity_ = v; }
    void set_velocityX(const float &v){ velocity_[0] = v; }
    void set_velocityY(const float &v){ velocity_[1] = v; }
    void set_velocityZ(const float &v){ velocity_[2] = v; }
    
    
private:
    // You will probably need to store some additional data here, e.g., velocity
    
    Point3 position_;
    float radius_;
    Vector3 velocity_;
};

#endif
