/** CSci-4611 Assignment 3:  Earthquake
 */

#include "earth.h"
#include "config.h"

#include <vector>

// for M_PI constant
#define _USE_MATH_DEFINES
#include <math.h>


Earth::Earth() {
}

Earth::~Earth() {
}

void Earth::Init(const std::vector<std::string> &search_path) {
    // init shader program
    shader_.Init();
    
    // init texture: you can change to a lower-res texture here if needed
    earth_tex_.InitFromFile(Platform::FindFile("earth-2k.png", search_path));

    std::vector<unsigned int> indices;
    std::vector<Point2> tex_coords;


    // Vertices, indices, and textures of the Earth for 2D
    float i, j, radius;
    for (i = 0; i <= nstacks; i++){
        for (j = 0; j <= nslices; j++){
            double longitude = -180 + j/nslices * 360;
            double latitude = 90 - i/nstacks * 180;
            Point3 p = LatLongToSphere( latitude, longitude);
            vertices3d.push_back(p);
            normals3d.push_back(p - Point3(0, 0, 0));

            vertices2d.push_back(Point3(-M_PI + 2 * M_PI/nslices * j, M_PI/2 - M_PI/nstacks * i, 0));
            normals2d.push_back(Vector3(0, 0, 1));
            tex_coords.push_back(Point2(1.0/nslices * j, 1.0/nstacks * i));
            if (i >= 1 && j >= 1){
              indices.push_back((nslices + 1) * (i-1) + (j - 1));
              indices.push_back((nslices + 1) * i + (j - 1));
              indices.push_back((nslices + 1) * i + j);

              indices.push_back((nslices + 1) * (i-1) + (j - 1));
              indices.push_back((nslices + 1) * i + j);
              indices.push_back((nslices + 1) * (i-1) + j);
            }
        }
    } 
    
    earth_mesh_.SetVertices(vertices2d);
    earth_mesh_.SetIndices(indices);
    earth_mesh_.SetNormals(normals2d);
    earth_mesh_.SetTexCoords(0, tex_coords);
    earth_mesh_.UpdateGPUMemory();
}



void Earth::Draw(const Matrix4 &model_matrix, const Matrix4 &view_matrix, const Matrix4 &proj_matrix) {
    // Define a really bright white light.  Lighting is a property of the "shader"
    DefaultShader::LightProperties light;
    light.position = Point3(10,10,10);
    light.ambient_intensity = Color(1,1,1);
    light.diffuse_intensity = Color(1,1,1);
    light.specular_intensity = Color(1,1,1);
    shader_.SetLight(0, light);


    // Adust the material properties, material is a property of the thing
    // (e.g., a mesh) that we draw with the shader.  The reflectance properties
    // affect the lighting.  The surface texture is the key for getting the
    // image of the earth to show up.
    DefaultShader::MaterialProperties mat;
    mat.ambient_reflectance = Color(0.5, 0.5, 0.5);
    mat.diffuse_reflectance = Color(0.75, 0.75, 0.75);
    mat.specular_reflectance = Color(0.75, 0.75, 0.75);
    mat.surface_texture = earth_tex_;

    // Draw the earth mesh using these settings
    if (earth_mesh_.num_triangles() > 0) {
        shader_.Draw(model_matrix, view_matrix, proj_matrix, &earth_mesh_, mat);
    }

    
}


Point3 Earth::LatLongToSphere(double latitude, double longitude) const {
    // lat,long --> sphere calculations in one place.
    float x =  cos(latitude/90 * M_PI/2) * sin(longitude/180 * M_PI); 
    float y =  sin(latitude/90 * M_PI/2);
    float z =  cos(latitude/90 * M_PI/2) * cos(longitude/180 * M_PI);
    
    return Point3(x,y,z);
}

Point3 Earth::LatLongToPlane(double latitude, double longitude) const {
    // lat,long --> plane calculations in one place.
    float x = longitude/180 * M_PI; 
    float y = latitude/90 * M_PI/2;
    
    return Point3(x, y, 0);
}



void Earth::DrawDebugInfo(const Matrix4 &model_matrix, const Matrix4 &view_matrix, const Matrix4 &proj_matrix) {
    // This draws a cylinder for each line segment on each edge of each triangle in your mesh.
    // So it will be very slow if you have a large mesh, but it's quite useful when you are
    // debugging your mesh code, especially if you start with a small mesh.
    for (int t=0; t<earth_mesh_.num_triangles(); t++) {
        std::vector<unsigned int> indices = earth_mesh_.triangle_vertices(t);
        std::vector<Point3> loop;
        loop.push_back(earth_mesh_.vertex(indices[0]));
        loop.push_back(earth_mesh_.vertex(indices[1]));
        loop.push_back(earth_mesh_.vertex(indices[2]));
        quick_shapes_.DrawLines(model_matrix, view_matrix, proj_matrix,
            Color(1,1,0), loop, QuickShapes::LinesType::LINE_LOOP, 0.005);
    }
}

