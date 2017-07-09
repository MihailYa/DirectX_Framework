#pragma once

#pragma pack(push, 1)

struct MS3DHeader
{
	char id[10];		// always "MS3D000000"
	unsigned int version;	// 3
};

struct MS3DVertex
{
	unsigned char flags;	// SELECTED | SELECTED2 | HIDDEN
	float vertex[3];
	char boneId;		// -1 = no bone
	unsigned char referenceCount;
};

struct MS3DTriangle
{
	unsigned short	flags;	// SELECTED | SELECTED2 | HIDDEN
	unsigned short	vertexIndices[3];
	float			vertexNormals[3][3];
	float			s[3];
	float			t[3];
	unsigned char	smoothingGroup;	// 1 - 32
	unsigned char	groupIndex;
};

struct MS3DGroup
{
	unsigned char	flags;                 // SELECTED | HIDDEN
	char            name[32];
	unsigned short	numtriangles;
	unsigned short  *triangleIndices;      // the groups group the triangles
	char            materialIndex;         // -1 = no material
};

struct MS3DMaterial
{
	char            name[32];
	float           ambient[4];
	float           diffuse[4];
	float           specular[4];
	float           emissive[4];
	float           shininess;                          // 0.0f - 128.0f
	float           transparency;                       // 0.0f - 1.0f
	char            mode;                               // 0, 1, 2 is unused now
	char            texture[128];                       // texture.bmp
	char            alphamap[128];                      // alpha.bmp
};

#pragma pack(pop, 1)