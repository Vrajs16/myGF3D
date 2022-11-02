#include "simple_logger.h"

#include "gf3d_model.h"

#include "gf3d_draw.h"

typedef struct
{
    Model *cube;
    Model *icube;
    Model *sphere;
    Model *isphere;
    Model *icylinder;
}GF3D_DrawManager;

static GF3D_DrawManager gf3d_draw_manager = {0};

void gf3d_draw_close()
{
    gf3d_model_free(gf3d_draw_manager.cube);
    gf3d_model_free(gf3d_draw_manager.icube);
    gf3d_model_free(gf3d_draw_manager.sphere);
    gf3d_model_free(gf3d_draw_manager.isphere);
    gf3d_model_free(gf3d_draw_manager.icylinder);
}

void gf3d_draw_init()
{
    gf3d_draw_manager.cube = gf3d_model_load_full("assets/primitives/cube.obj","assets/primitives/flatwhite.png");
    gf3d_draw_manager.icube = gf3d_model_load_full("assets/primitives/icube.obj","assets/primitives/flatwhite.png");
    gf3d_draw_manager.sphere = gf3d_model_load_full("assets/primitives/sphere.obj","assets/primitives/flatwhite.png");
    gf3d_draw_manager.isphere = gf3d_model_load_full("assets/primitives/isphere.obj","assets/primitives/flatwhite.png");
    gf3d_draw_manager.icylinder= gf3d_model_load_full("assets/primitives/icylinder.obj","assets/primitives/flatwhite.png");
    
    atexit(gf3d_draw_close);
}

void gf3d_draw_edge_3d(Edge3D edge,Vector3D position,Vector3D rotation,Vector3D scale,float radius,Color color)
{
    Matrix4 modelMat;
    Vector3D v,angles;
    float d;
    
    if (!radius)return;// zero radius doesn't draw anyway
    
    if (!gf3d_draw_manager.icylinder)return;
    //calculate rotation of the vector from a to b
    vector3d_sub(v,edge.b,edge.a);// vector from a to b
    vector3d_angles (v, &angles);//rotation from a to b
    //angles.y -= GFC_HALF_PI;
    
    //z scale based on vector length
    d = vector3d_magnitude(v);
    if (!d)return;// can't draw a zero length edge
    scale.x *= d /gf3d_draw_manager.icylinder->mesh->bounds.w;
    // y and z scale based on radius
    scale.y *= radius / gf3d_draw_manager.icylinder->mesh->bounds.h;
    scale.z *= radius / gf3d_draw_manager.icylinder->mesh->bounds.d;

    gfc_matrix4_from_vectors(
        modelMat,
        vector3d(position.x + edge.a.x,position.y + edge.a.y,position.z + edge.a.z),
        vector3d(rotation.x + angles.x,rotation.y + angles.y,rotation.z + angles.z),
        scale);
//    gf3d_model_draw_highlight(gf3d_draw_manager.icylinder,modelMat,gfc_color_to_vector4f(color));
    gf3d_model_draw(gf3d_draw_manager.icylinder,modelMat,gfc_color_to_vector4f(color),vector4d(1,1,1,0));
}

void gf3d_draw_cube_wireframe(Box cube,Vector3D position,Vector3D rotation,Vector3D scale,Color color)
{
    Matrix4 modelMat;
    
    gfc_matrix4_from_vectors(
        modelMat,
        vector3d(position.x + cube.x,position.y + cube.y,position.z + cube.z),
        rotation,
        vector3d(scale.x * cube.w,scale.y * cube.h,scale.z * cube.d));
    gf3d_model_draw_highlight(gf3d_draw_manager.icube,modelMat,gfc_color_to_vector4f(color));
}

void gf3d_draw_cube_solid(Box cube,Vector3D position,Vector3D rotation,Vector3D scale,Color color)
{
    Matrix4 modelMat;
    
    gfc_matrix4_from_vectors(
        modelMat,
        vector3d(position.x + cube.x,position.y + cube.y,position.z + cube.z),
        rotation,
        vector3d(scale.x * cube.w,scale.y * cube.h,scale.z * cube.d));
    gf3d_model_draw(gf3d_draw_manager.cube,modelMat,gfc_color_to_vector4f(color),vector4d(1,1,1,1));
}


void gf3d_draw_sphere_wireframe(Sphere sphere,Vector3D position,Vector3D rotation,Vector3D scale,Color color)
{
    Matrix4 modelMat;
    
    gfc_matrix4_from_vectors(
        modelMat,
        vector3d(position.x + sphere.x,position.y + sphere.y,position.z + sphere.z),
        rotation,
        vector3d(scale.x * sphere.r,scale.y * sphere.r,scale.z * sphere.r));
    gf3d_model_draw_highlight(gf3d_draw_manager.isphere,modelMat,gfc_color_to_vector4f(color));
}

void gf3d_draw_sphere_solid(Sphere sphere,Vector3D position,Vector3D rotation,Vector3D scale,Color color,Color ambient)
{
    Matrix4 modelMat;
    
    gfc_matrix4_from_vectors(
        modelMat,
        vector3d(position.x + sphere.x,position.y + sphere.y,position.z + sphere.z),
        rotation,
        vector3d(scale.x * sphere.r,scale.y * sphere.r,scale.z * sphere.r));
    gf3d_model_draw(gf3d_draw_manager.sphere,modelMat,gfc_color_to_vector4f(color),gfc_color_to_vector4f(ambient));
}


void gf3d_draw_circle(Circle circle,Vector3D position,Vector3D rotation,Vector3D scale,Color color)
{
    Matrix4 modelMat;
    
    circle.r /= gf3d_draw_manager.sphere->mesh->bounds.w;
    
    gfc_matrix4_from_vectors(
        modelMat,
        vector3d(position.x + circle.x,position.y + circle.y,position.z),
        rotation,
        vector3d(scale.x * circle.r,scale.y * circle.r,scale.z * circle.r));
    gf3d_model_draw(gf3d_draw_manager.sphere,modelMat,vector4d(0,0,0,0),vector4d(0,0,0,0));
    gf3d_model_draw_highlight(gf3d_draw_manager.sphere,modelMat,gfc_color_to_vector4f(color));
}



/*eol@eof*/
