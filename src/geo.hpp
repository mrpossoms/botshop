#pragma once

#include "core.h"

#define STL_HEADER_SIZE 80

namespace botshop
{

//    ___ _____ _
//   / __|_   _| |
//   \__ \ | | | |__
//   |___/ |_| |____|
//
struct STLTri
{
	Vec3* normal;
	Vec3* verts;
	uint16_t attr;
};

struct STLVert
{
	Vec3 position;
	Vec3 normal;
};

struct Vertex
{
	vec3 position;
	vec3 normal;
	vec3 texture;
};

struct Model
{
	virtual dGeomID create_collision_geo(dSpaceID ode_space) = 0;
	virtual unsigned int vert_count() = 0;
	virtual Vertex* verts() = 0;

	Vec3 *_min, *_max;

	Vec3 min_position();
	Vec3 max_position();
	Vec3 box_dimensions();
};

struct STLModel : Model
{
	uint8_t header[STL_HEADER_SIZE];
	uint32_t tri_count;

	Vertex* all_verts;
	Vec3* all_positions;
	Vec3* all_normals;
	STLTri* tris;

	dTriMeshDataID ode_tri_mesh_dat;

	STLModel(int fd);
	~STLModel();
	dGeomID create_collision_geo(dSpaceID ode_space);
	unsigned int vert_count();
	Vertex* verts();

	Vec3 min_position();
	Vec3 max_position();

private:
	Vec3 *_min, *_max;
	unsigned int* _indices;
};
//------------------------------------------------------------------------------
class ModelFactory
{
public:
	static Model* get_model(std::string path);
};
//------------------------------------------------------------------------------
struct OBJModel : Model
{
	OBJModel(int fd);
	~OBJModel();

	dGeomID create_collision_geo(dSpaceID ode_space);
	unsigned int vert_count();
	Vertex* verts();

	dTriMeshDataID ode_tri_mesh_dat;
private:
	std::vector<Vec3> positions;
	std::vector<Vec3> tex_coords;
	std::vector<Vec3> normals;
	std::vector<Vec3> params;
	std::vector<Vertex> vertices;
};

}
