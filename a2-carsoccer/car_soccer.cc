/** CSci-4611 Assignment 2:  Car Soccer
 */

#include "car_soccer.h"
#include "config.h"
#include <math.h>
#include <cmath>
#include <time.h>

CarSoccer::CarSoccer() : GraphicsApp(1024,768, "Car Soccer") {
    // Define a search path for finding data files (images and shaders)
    searchPath_.push_back(".");
    searchPath_.push_back("./data");
    searchPath_.push_back(DATA_DIR_INSTALL);
    searchPath_.push_back(DATA_DIR_BUILD);
}

CarSoccer::~CarSoccer() {
}


Vector2 CarSoccer::joystick_direction() {
    Vector2 dir;
    if (IsKeyDown(GLFW_KEY_LEFT))
        dir[0]--;
    if (IsKeyDown(GLFW_KEY_RIGHT))
        dir[0]++;
    if (IsKeyDown(GLFW_KEY_UP))
        dir[1]++;
    if (IsKeyDown(GLFW_KEY_DOWN))
        dir[1]--;
    return dir;
}


void CarSoccer::OnSpecialKeyDown(int key, int scancode, int modifiers) {
    if (key == GLFW_KEY_SPACE) {
        // Here's where you could call some form of launch_ball();
        int x = rand() % 41 + -20;
        int y = rand() % 41 + -20;
        int z = rand() % 41 + -20;
        
        car_.Reset();
        ball_.Reset(Vector3(x ,y ,z));
    }
    if (key == GLFW_KEY_F1)
        fp_view = !fp_view;

}


void CarSoccer::UpdateSimulation(double timeStep) {
    // Here's where you shound do your "simulation", updating the positions of the
    // car and ball as needed and checking for collisions.  Filling this routine
    // in is the main part of the assignment.

    Point3 bP, cP; // ball, car position
    Vector3 bV, cV; // ball, car velocity
    Vector3 distanceV; // vector from car to the ball
    float distance; // distance from car to the ball
    Vector2 kb; // Vector of the arrows pressed
    float thrust, drag, turnRate;

        

    // Move the car
    kb = joystick_direction();
    turnRate = kb[0];
    thrust = -kb[1];
    drag = car_.speed()/MAX_SPEED; 
    car_.set_speed(car_.speed() + (thrust-drag) * MAX_SPEED * timeStep);
    Vector3 tempV =  Matrix4::RotationY(car_.dirAngle() * M_PI / 180) * Vector3(0, 0, car_.speed());
    car_.set_velocity(tempV);
    car_.set_dirAngle(car_.dirAngle() +  2 * turnRate * car_.speed() * timeStep);
    car_.set_position(car_.position() + timeStep * tempV);
    cP = car_.position();
    cV = car_.velocity();

    float radA = car_.dirAngle() * M_PI/180;
    // Check if the car hit the right wall
    if (cP[0] + car_.collision_radius() > 40){
        //car_.set_velocity(0.8 *Vector3(cV[0] * -1, 0, cV[2]));
        car_.set_speed(0);
        car_.set_velocity(Vector3(0,0,0));
        car_.set_position(Point3(80 - (cP[0] + 2 * car_.collision_radius()), 0, cP[2]));
    }
    // Check if the car hit the left wall
    if (cP[0] - car_.collision_radius() < -40){
        //car_.set_velocity(0.8 * Vector3(cV[0] * -1, cV[1], cV[2]));
        car_.set_speed(0);
        car_.set_velocity(Vector3(0,0,0));
        car_.set_position(Point3(-80 + 2 * car_.collision_radius() - cP[0], cP[1], cP[2]));
    }
    // Check if the car hit the front wall
    if (cP[2] + car_.collision_radius() > 50){
        //car_.set_velocity(0.8 * Vector3(cV[0], cV[1], cV[2] * -1));
        car_.set_speed(0);
        car_.set_velocity(Vector3(0,0,0));
        car_.set_position(Point3(cP[0], cP[1], 100 - (cP[2] + 2 * car_.collision_radius())));
    }
    // Check if the carl hit the back wall
    if (cP[2] - car_.collision_radius() < -50){
        //car_.set_velocity(0.8 * Vector3(cV[0], cV[1], cV[2] * -1));
        car_.set_speed(0);
        car_.set_velocity(Vector3(0,0,0));
        car_.set_position(Point3(cP[0], cP[1], -100 + 2 * car_.collision_radius() - cP[2] ));
    }
    
    // printf("%f\n", car_.dirAngle());
    
    // Set of kinematic equation for calculating the new position and velocity of the ball.
    bP = 0.5 * gravity_  * pow(timeStep, 2) + timeStep * ball_.velocity() + ball_.position();
    ball_.set_position(bP);
    ball_.set_velocityY(ball_.velocity()[1] + timeStep * gravity_[1]); 
    bV = ball_.velocity();
    distanceV =  car_.position() - bP; 
    distance = sqrt(pow(distanceV[0],2) + pow(distanceV[1],2) + pow(distanceV[2],2)); 


    // Check if the ball hit the car
    if (distance < car_.collision_radius() + ball_.radius()){
        Vector3 normalV = distanceV/distanceV.Length();
        Vector3 postV = 0.8 * (ball_.velocity() - 2 * ( ball_.velocity().Dot(normalV)) * normalV);

        ball_.set_position(bP + normalV * (distance - (car_.collision_radius() + ball_.radius())));   
        ball_.set_velocity(postV + car_.velocity());
    }
    // Check if the ball hit the ground
    if (bP[1] - ball_.radius() < 0){  // The ball hit the ground
        ball_.set_velocity(Vector3(0.8 * bV[0], 0.6 * bV[1]*-1, 0.8 * bV[2])); 
        ball_.set_position(Point3(bP[0], 2* ball_.radius() - bP[1], bP[2]));
    }
    // Check if the ball hit the ceiling
    if (bP[1] + ball_.radius() > 35){
        ball_.set_velocity(0.8 * Vector3(bV[0], bV[1]*-1, bV[2]));
        ball_.set_position(Point3(bP[0],  70 - (ball_.radius() * 2 + bP[1]), bP[2]));
    }
    // Check if the ball hit the right wall
    if (bP[0] + ball_.radius() > 40){
        ball_.set_velocity(0.8 *Vector3(bV[0] * -1, bV[1], bV[2]));
        ball_.set_position(Point3(80 - (bP[0] + 2 * ball_.radius()), bP[1], bP[2]));
    }
    // Check if the ball hit the left wall
    if (bP[0] - ball_.radius() < -40){
        ball_.set_velocity(0.8 * Vector3(bV[0] * -1, bV[1], bV[2]));
        ball_.set_position(Point3(-80 + 2 * ball_.radius() - bP[0], bP[1], bP[2]));
    }
    // Check if the ball hit the front wall
    if (bP[2] + ball_.radius() > 50){
        // Check if the ball hits the goal
        if (abs(bP[0]) < 10 && bP[1]  < 10){
            int x = rand() % 41 + -20;
            int y = rand() % 41 + -20;
            int z = rand() % 41 + -20;
            
            car_.Reset();
            ball_.Reset(Vector3(x ,y ,z));
            return; 
        }
        ball_.set_velocity(0.8 * Vector3(bV[0], bV[1], bV[2] * -1));
        ball_.set_position(Point3(bP[0], bP[1], 100 - (bP[2] + 2 * ball_.radius())));
    }
    // Check if the ball hit the back wall
    if (bP[2] - ball_.radius() < -50){
        // Check if the ball hits the goal
        if (abs(bP[0]) < 10 && bP[1] < 10){
            int x = rand() % 41 + -20;
            int y = rand() % 41 + -20;
            int z = rand() % 41 + -20;
            
            car_.Reset();
            ball_.Reset(Vector3(x ,y ,z));
            return; 
        }
        ball_.set_velocity(0.8 * Vector3(bV[0], bV[1], bV[2] * -1));
        ball_.set_position(Point3(bP[0], bP[1], -100 + 2 * ball_.radius() - bP[2] ));
    }
    


}


void CarSoccer::InitOpenGL() {
    // Set up the camera in a good position to see the entire field
    projMatrix_ = Matrix4::Perspective(60, aspect_ratio(), 1, 1000);
    Vector3 tempV =  Matrix4::RotationY(car_.dirAngle() * M_PI / 180) * Vector3(0, 0,abs(car_.speed()));
    modelMatrix_ = Matrix4::LookAt(Point3(0,60,70), Point3(0, 0, 10) , Vector3(0,1,0));
 
    // Set a background color for the screen
    glClearColor(0.8,0.8,0.8, 1);
    
    // Load some image files we'll use
    fieldTex_.InitFromFile(Platform::FindFile("pitch.png", searchPath_));
    crowdTex_.InitFromFile(Platform::FindFile("crowd.png", searchPath_));

    // Initialize goal post vector
    int i, j;
    for (i = -10; i < 10; i ++){
        for (j =0; j < 10; j++){
            goalPost.push_back(Point3(i, j, 0) );
            goalPost.push_back(Point3(i, j + 1, 0) );
            goalPost.push_back(Point3(i + 1, j + 1, 0) );
            goalPost.push_back(Point3(i + 1, j, 0) );
            goalPost.push_back(Point3(i, j, 0) );
        }
        goalPost.push_back(Point3(i + 1, j, 0) );
    }

    lBound.push_back(Point3(-40, 0, 0));
    lBound.push_back(Point3(-40, 35, 0));
    lBound.push_back(Point3(40, 35, 0));
    lBound.push_back(Point3(40, 0, 0));
    lBound.push_back(Point3(-40, 0, 0));

    wBound.push_back(Point3(0, 0, 50));
    wBound.push_back(Point3(0, 35, 50));
    wBound.push_back(Point3(0, 35, -50));
    wBound.push_back(Point3(0, 0, -50));
    wBound.push_back(Point3(0, 0, 50));
}


void CarSoccer::DrawUsingOpenGL() {
    Vector3 dir =  Matrix4::RotationY(car_.dirAngle() * M_PI / 180) * Vector3(0, 0, -1);

    // Draw the crowd as a fullscreen background image
    if (fp_view)
        modelMatrix_ = Matrix4::LookAt(car_.position() + Vector3(0, 4, 0)  - 3 * dir , car_.position() + 30 * dir, Vector3(0,1,0));
    else{
        modelMatrix_ = Matrix4::LookAt(Point3(0,60,70), Point3(0,0,10) , Vector3(0,1,0));
        quickShapes_.DrawFullscreenTexture(Color(1,1,1), crowdTex_);
    }
    // Draw the field with the field texture on it.
    Color col(16.0/255.0, 46.0/255.0, 9.0/255.0);
    Matrix4 M = Matrix4::Translation(Vector3(0,-0.201,0)) * Matrix4::Scale(Vector3(50, 1, 60));
    quickShapes_.DrawSquare(modelMatrix_ * M, viewMatrix_, projMatrix_, col);
    M = Matrix4::Translation(Vector3(0,-0.2,0)) * Matrix4::Scale(Vector3(40, 1, 50));
    quickShapes_.DrawSquare(modelMatrix_ * M, viewMatrix_, projMatrix_, Color(1,1,1), fieldTex_);
    
    // Draw the car
    Color carcol(0.8, 0.2, 0.2);
    Matrix4 Mcar =
        Matrix4::Translation(car_.position() - Point3(0,0,0)) *
        Matrix4::Scale(car_.size()) *
        Matrix4::Scale(Vector3(0.5,0.5,0.5)) *
        Matrix4::RotationY(car_.dirAngle() * M_PI / 180);
    quickShapes_.DrawCube(modelMatrix_ * Mcar, viewMatrix_, projMatrix_, carcol);
    if (!fp_view)
        quickShapes_.DrawArrow(modelMatrix_, viewMatrix_, projMatrix_, Color(1,1,0), car_.position(), 4*dir, 0.1);
    
    
    // Draw the ball
    Color ballcol(1,1,1);
    Matrix4 Mball =
        Matrix4::Translation(ball_.position() - Point3(0,0,0)) *
        Matrix4::Scale(Vector3(ball_.radius(), ball_.radius(), ball_.radius()));
    quickShapes_.DrawSphere(modelMatrix_ * Mball, viewMatrix_, projMatrix_, ballcol);
    
    
    // Draw the ball's shadow -- this is a bit of a hack, scaling Y by zero
    // flattens the sphere into a pancake, which we then draw just a bit
    // above the ground plane.
    Color shadowcol(0.2,0.4,0.15);
    Matrix4 Mshadow =
        Matrix4::Translation(Vector3(ball_.position()[0], -0.1, ball_.position()[2])) *
        Matrix4::Scale(Vector3(ball_.radius(), 0, ball_.radius())) *
        Matrix4::RotationX(90);
    quickShapes_.DrawSphere(modelMatrix_ * Mshadow, viewMatrix_, projMatrix_, shadowcol);
    
    
    // You should add drawing the goals and the boundary of the playing area
    // using quickShapes_.DrawLines()
    Color goalcol1(1,0,0);
    Matrix4 goals1 = 
        Matrix4::Translation(Vector3(0, 0, 50)); 
    quickShapes_.DrawLines(modelMatrix_ * goals1, viewMatrix_, projMatrix_, goalcol1, goalPost, QuickShapes::LinesType::LINE_STRIP, 0.1); 

    Color goalcol2(0,0,1);
    Matrix4 goals2 = 
        Matrix4::Translation(Vector3(0, 0, -50)); 
    quickShapes_.DrawLines(modelMatrix_ * goals2, viewMatrix_, projMatrix_, goalcol2, goalPost, QuickShapes::LinesType::LINE_STRIP, 0.1); 


    // Draw the boundary box
    Color boundary(1,1,1);

    Matrix4 bound1 = 
        Matrix4::Translation(Vector3(0 , 0, 50));
    Matrix4 bound2 = 
        Matrix4::Translation(Vector3(0, 0, -50));
    Matrix4 bound3 =
        Matrix4::Translation(Vector3(-40, 0, 0));
    Matrix4 bound4 =
        Matrix4::Translation(Vector3(40, 0 ,0));

    quickShapes_.DrawLines(modelMatrix_ * bound1, viewMatrix_, projMatrix_, boundary, lBound, QuickShapes::LinesType::LINE_LOOP, 0.1); 
    quickShapes_.DrawLines(modelMatrix_ * bound2, viewMatrix_, projMatrix_, boundary, lBound, QuickShapes::LinesType::LINE_LOOP, 0.1); 
    quickShapes_.DrawLines(modelMatrix_ * bound3, viewMatrix_, projMatrix_, boundary, wBound, QuickShapes::LinesType::LINE_LOOP, 0.1); 
    quickShapes_.DrawLines(modelMatrix_ * bound4, viewMatrix_, projMatrix_, boundary, wBound, QuickShapes::LinesType::LINE_LOOP, 0.1); 
}



















