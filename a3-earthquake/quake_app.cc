/** CSci-4611 Assignment 3:  Earthquake
 */

#include "quake_app.h"
#include "config.h"

#include <iostream>
#include <sstream>
#include <gfxmath.h>

#define FLAT    0
#define MORPH_FLAT_GLOBE    1
#define MORPH_GLOBE_FLAT    2
#define GLOBE   3

// Number of seconds in 1 year (approx.)
const int PLAYBACK_WINDOW = 12 * 28 * 24 * 60 * 60;

using namespace std;

QuakeApp::QuakeApp() : GraphicsApp(1280,720, "Earthquake"),
    playback_scale_(15000000.0), debug_mode_(false)
{
    // Define a search path for finding data files (images and earthquake db)
    search_path_.push_back(".");
    search_path_.push_back("./data");
    search_path_.push_back(DATA_DIR_INSTALL);
    search_path_.push_back(DATA_DIR_BUILD);
    
    quake_db_ = EarthquakeDatabase(Platform::FindFile("earthquakes.txt", search_path_));
    current_time_ = quake_db_.earthquake(quake_db_.min_index()).date().ToSeconds();

 }


QuakeApp::~QuakeApp() {
}


void QuakeApp::InitNanoGUI() {
    // Setup the GUI window
    nanogui::Window *window = new nanogui::Window(screen(), "Earthquake Controls");
    window->setPosition(Eigen::Vector2i(10, 10));
    window->setSize(Eigen::Vector2i(400,200));
    window->setLayout(new nanogui::GroupLayout());
    
    date_label_ = new nanogui::Label(window, "Current Date: MM/DD/YYYY", "sans-bold");
    
    globe_btn_ = new nanogui::Button(window, "Globe");
    globe_btn_->setCallback(std::bind(&QuakeApp::OnGlobeBtnPressed, this));
    globe_btn_->setTooltip("Toggle between map and globe.");
    
    new nanogui::Label(window, "Playback Speed", "sans-bold");
    
    nanogui::Widget *panel = new nanogui::Widget(window);
    panel->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal,
                                            nanogui::Alignment::Middle, 0, 20));
    
    nanogui::Slider *slider = new nanogui::Slider(panel);
    slider->setValue(0.5f);
    slider->setFixedWidth(120);
    
    speed_box_ = new nanogui::TextBox(panel);
    speed_box_->setFixedSize(Eigen::Vector2i(60, 25));
    speed_box_->setValue("50");
    speed_box_->setUnits("%");
    slider->setCallback(std::bind(&QuakeApp::OnSliderUpdate, this, std::placeholders::_1));
    speed_box_->setFixedSize(Eigen::Vector2i(60,25));
    speed_box_->setFontSize(20);
    speed_box_->setAlignment(nanogui::TextBox::Alignment::Right);
    
    nanogui::Button* debug_btn = new nanogui::Button(window, "Toggle Debug Mode");
    debug_btn->setCallback(std::bind(&QuakeApp::OnDebugBtnPressed, this));
    debug_btn->setTooltip("Toggle displaying mesh triangles and normals (can be slow)");
    
    screen()->performLayout();
}

void QuakeApp::OnLeftMouseDrag(const Point2 &pos, const Vector2 &delta) {
    // mouse is dragged left/right,up/down on the screen.
    rotation_angle += delta[0]/10; 
    tilt_angle += delta[1]/10;
}


void QuakeApp::OnGlobeBtnPressed() {
    // TODO: This is where you can switch between flat earth mode and globe mode
    
    if (globe_state == 0){
        globe_state = 1;
    }
    
    if (globe_state == 3){
        globe_state = 2;
    }
    
}

void QuakeApp::OnDebugBtnPressed() {
    debug_mode_ = !debug_mode_;
}

void QuakeApp::OnSliderUpdate(float value) {
    speed_box_->setValue(std::to_string((int) (value * 100)));
    playback_scale_ = 30000000.0*value;
}


void QuakeApp::UpdateSimulation(double dt)  {
    // Advance the current time and loop back to the start if time is past the last earthquake
    current_time_ += playback_scale_ * dt;
    if (current_time_ > quake_db_.earthquake(quake_db_.max_index()).date().ToSeconds()) {
        current_time_ = quake_db_.earthquake(quake_db_.min_index()).date().ToSeconds();
    }
    if (current_time_ < quake_db_.earthquake(quake_db_.min_index()).date().ToSeconds()) {
        current_time_ = quake_db_.earthquake(quake_db_.max_index()).date().ToSeconds();
    }
    
    Date d(current_time_);
    stringstream s;
    s << "Current date: " << d.month()
        << "/" << d.day()
        << "/" << d.year();
    date_label_->setCaption(s.str());
    
    // TODO: Any animation, morphing, rotation of the earth, or other things that should
    // be updated once each frame would go here.
    vector<Point3> morphvertices;
    vector<Vector3> morphnormals;
    
    // going from flat to globe
    if (globe_state == 1 && t < 1){
        t += dt;
        Mesh mesh;
        
        for (int i = 0; i < earth_.vertices3d.size(); i++)
        {
            Point3 p = Point3::Lerp(earth_.vertices2d[i], earth_.vertices3d[i], t);
            Vector3 v = Vector3::Lerp(earth_.normals2d[i], earth_.normals3d[i], t);
            morphvertices.push_back(p); 
            morphnormals.push_back(v);
        }
        tilt_angle = 0;
        rotation_angle = 0;
        earth_.earth_mesh_.SetVertices(morphvertices);
        earth_.earth_mesh_.SetNormals(morphnormals);
    }
    else if (globe_state == 1 && t >= 1){
        t = 0;
        globe_state = 3;
        
        earth_.earth_mesh_.SetVertices(earth_.vertices3d);
        earth_.earth_mesh_.SetNormals(earth_.normals3d);
    }

    // going from globe to flat
    if (globe_state == 2 && t <1 ){
        t += dt;
        Mesh mesh;

        for (int i = 0; i <earth_.vertices3d.size(); i++)
        {
            Point3 p = Point3::Lerp(earth_.vertices3d[i], earth_.vertices2d[i],t);
            Vector3 v = Vector3::Lerp(earth_.normals3d[i], earth_.normals2d[i], t);
            morphvertices.push_back(p); 
            morphnormals.push_back(v);
        }
        earth_.earth_mesh_.SetVertices(morphvertices);
        earth_.earth_mesh_.SetNormals(morphnormals);
    }
    else if (globe_state == 2 && t >= 1){
        t = 0;
        globe_state = 0;
        earth_.earth_mesh_.SetVertices(earth_.vertices2d);
        earth_.earth_mesh_.SetNormals(earth_.normals2d);
    }
}


void QuakeApp::InitOpenGL() {
    // Set up the camera in a good position to see the entire earth in either mode
    proj_matrix_ = Matrix4::Perspective(60, aspect_ratio(), 0.1, 50);
    view_matrix_ = Matrix4::LookAt(Point3(0,0,3.5), Point3(0,0,0), Vector3(0,1,0));
    glClearColor(0.0, 0.0, 0.0, 1);
    
    // Initialize the earth object
    earth_.Init(search_path_);

    // Initialize the texture used for the background image
    stars_tex_.InitFromFile(Platform::FindFile("iss006e40544.png", search_path_));

    // display min,max magnitude earthquake upon startup
    printf("max magnitude: %f, min magnitude: %f \n", quake_db_.max_magnitude(), quake_db_.min_magnitude());
}


void QuakeApp::DrawUsingOpenGL() {
    quick_shapes_.DrawFullscreenTexture(Color(1,1,1), stars_tex_);
    Date d(current_time_);
    Date e(d.month(), d.day(), d.year()-1); // new date that corresponds to one year before the current time
    int eq_past_index, eq_index;
    Earthquake eq;
    Matrix4 model_matrix, globe_matrix;
    Matrix4 identity;


   if (globe_state == 3){
        globe_matrix = identity * Matrix4::RotationX(tilt_angle * 2*M_PI/180)
                                * Matrix4::RotationY(rotation_angle * 2*M_PI/180);
    }

    // Draw the earth
    if (globe_state == 3)
        earth_.Draw(globe_matrix, view_matrix_, proj_matrix_);
    else
        earth_.Draw(model_matrix, view_matrix_, proj_matrix_);
    
    // If debug mode is on, display mesh triangles.
    if (debug_mode_) { 
        earth_.DrawDebugInfo(model_matrix, view_matrix_, proj_matrix_);
    }

    // Draw Earthquakes
    eq_index = quake_db_.FindMostRecentQuake(d);
    eq_past_index = quake_db_.FindMostRecentQuake(e);

    // Earthquake max: 9.6 min: 5.5.
    // Vary the size of the earthquakes, leanearly from the magnitude
    // Change the color changing from yellow to red, from least severe to most severe
    int i;
    for (i = eq_past_index; i < eq_index; i++){ 
        eq = quake_db_.earthquake(i);
        Matrix4 eq_pos;
        double eq_radius = GfxMath::Lerp(0.001, 0.075, (eq.magnitude()-5.5)/4.1 );
        double eq_color = 1 - (eq.magnitude() - 5.5)/4.1;
        Point3 eq_coord_3d = earth_.LatLongToSphere(eq.latitude(), eq.longitude());
        Point3 eq_coord_2d = earth_.LatLongToPlane(eq.latitude(), eq.longitude());

        if (globe_state == 3){ // 3d mode
            eq_pos = identity * Matrix4::RotationX(tilt_angle * 2*M_PI/180)
                                      * Matrix4::RotationY(rotation_angle * 2*M_PI/180)
                                      * Matrix4::Translation(earth_.LatLongToSphere(eq.latitude(), eq.longitude()) - Point3(0,0,0))
                                      * Matrix4::Scale(eq_radius * Vector3(1, 1, 1));
        }
        else if (globe_state == 0){ // 2d mode
            eq_pos = identity * Matrix4::Translation(earth_.LatLongToPlane(eq.latitude(), eq.longitude()) - Point3(0,0,0))
                                          * Matrix4::Scale(eq_radius * Vector3(1, 1, 1));
        }
        else if (globe_state == 1){ // morphing from flat to globe
            Point3 eq_coord_lerp = Point3(GfxMath::Lerp(eq_coord_2d[0], eq_coord_3d[0], t),
                                    GfxMath::Lerp(eq_coord_2d[1], eq_coord_3d[1], t),
                                    GfxMath::Lerp(eq_coord_2d[2], eq_coord_3d[2], t));
            eq_pos = identity * Matrix4::Translation(eq_coord_lerp - Point3(0,0,0)) 
                                      * Matrix4::Scale(eq_radius * Vector3(1, 1, 1));
        }
        else if (globe_state == 2){ // morphing from globe to flat
            Point3 eq_coord_lerp = Point3(GfxMath::Lerp(eq_coord_3d[0], eq_coord_2d[0], t),
                                    GfxMath::Lerp(eq_coord_3d[1], eq_coord_2d[1], t),
                                    GfxMath::Lerp(eq_coord_3d[2], eq_coord_2d[2], t));
            eq_pos = identity * Matrix4::Translation(eq_coord_lerp - Point3(0,0,0)) 
                                      * Matrix4::Scale(eq_radius * Vector3(1, 1, 1));
        }
        quick_shapes_.DrawSphere(eq_pos, view_matrix_, proj_matrix_, Color(1, eq_color, 0));
    }
}










