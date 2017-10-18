#include "geo.hpp"

//------------------------------------------------------------------------------
//    ___ _____ _
//   / __|_   _| |
//   \__ \ | | | |__
//   |___/ |_| |____|
//
STLModel::STLModel(int fd)
{
	assert(fd > 0);

	// read the header
	assert(read(fd, this->header, STL_HEADER_SIZE) == STL_HEADER_SIZE);

	// get the number of triangles
	assert(read(fd, &this->tri_count, sizeof(this->tri_count)) == sizeof(this->tri_count));

	// allocate space for the model's geometry
	unsigned int vert_count = this->tri_count * 3;
	all_verts = new Vertex[vert_count];
	all_positions = new Vec3[vert_count];
	all_normals = new Vec3[this->tri_count];
	this->tris = new STLTri[this->tri_count];
	this->_indices = new unsigned int[vert_count];

	// fill out a list of indicies, one for each vert
	// doing this just to appease ode. Since stl files
	// have no such concept
	for(int i = vert_count; i--;)
	{
		_indices[i] = i;
	}

	assert(this->tris && all_positions && all_normals);

	// read all the geometry
	for(int i = 0; i < this->tri_count; ++i)
	{
		STLTri* tri = this->tris + i;

		// point the verts and normal at the appropriate
		// spot allocated in the contiguious arrays
		tri->verts = all_positions + (i * 3);
		tri->normal = all_normals + i;

		// read from file
		read(fd, tri->normal, sizeof(Vec3));
		read(fd, tri->verts, sizeof(Vec3) * 3);
		read(fd, &tri->attr, sizeof(tri->attr));

		// Copy positions and normals into contiguious array
		// for rendering
		all_verts[(i * 3) + 0].position = tri->verts[0];
		all_verts[(i * 3) + 0].normal   = *tri->normal;
		all_verts[(i * 3) + 1].position = tri->verts[1];
		all_verts[(i * 3) + 1].normal   = *tri->normal;
		all_verts[(i * 3) + 2].position = tri->verts[2];
		all_verts[(i * 3) + 2].normal   = *tri->normal;
	}
}
//------------------------------------------------------------------------------
STLModel::~STLModel()
{
	delete this->tris[0].verts;
	delete this->tris[0].normal;
	delete this->tris;
	delete this->_indices;
}
//------------------------------------------------------------------------------
dGeomID STLModel::create_collision_geo(dSpaceID ode_space)
{
	ode_tri_mesh_dat = dGeomTriMeshDataCreate();

	unsigned int vert_count = tri_count * 3;
	dGeomTriMeshDataBuildSingle1(
		ode_tri_mesh_dat,
		tris->verts,     // verts
		sizeof(Vec3),    // vert stride
		vert_count,      // vert count
		_indices,        // indicies
		vert_count,      // ind count
		sizeof(unsigned int) * 3, // tri ind stride
		tris->normal
	);

	return dCreateTriMesh(ode_space, ode_tri_mesh_dat, 0, 0, 0);
}
//------------------------------------------------------------------------------
Vec3 STLModel::min_position()
{
	if(_min) return *_min;
	_min = new Vec3(all_positions->x, all_positions->y, all_positions->z);

	for(int i = tri_count * 3; i--;)
	{
		for(int j = 3; j--;)
		{
			if(all_positions[i].v[j] < _min->v[j])
			{
				_min->v[j] = all_positions[i].v[j];
			}
		}
	}

	return *_min;
}
//------------------------------------------------------------------------------
Vec3 STLModel::max_position()
{
	if(_max) return *_max;
	_max = new Vec3(all_positions->x, all_positions->y, all_positions->z);

	for(int i = tri_count * 3; i--;)
	{
		for(int j = 3; j--;)
		{
			if(all_positions[i].v[j] > _max->v[j])
			{
				_max->v[j] = all_positions[i].v[j];
			}
		}
	}

	return *_max;
}
