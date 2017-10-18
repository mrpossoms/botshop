#pragma once

#include "core.h"

#define STL_HEADER_SIZE 80

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
	Vec3 position;
	Vec3 normal;
	Vec3 texture;
};

struct Model
{
	virtual dGeomID create_collision_geo(dSpaceID ode_space) = 0;
	virtual unsigned int vert_count() = 0;
	virtual Vertex* verts() = 0;
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

	Vec3 min_position();
	Vec3 max_position();

private:
	Vec3 *_min, *_max;
	unsigned int* _indices;
};
//------------------------------------------------------------------------------
class STLFactory
{
private:
	static std::map<std::string, STLModel> _cached_models;

public:
	static STLModel* get_model(const char* path);
};
//------------------------------------------------------------------------------
