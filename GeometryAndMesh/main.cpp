#include "Mesh\Mesh.h"
#include <string>

using namespace std;

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		cout << "USAGE: [.exe] [.obj]" << endl;
		return -1;
	}

	string baseFileName = argv[1];
	baseFileName = baseFileName.substr(0, baseFileName.rfind("."));

	CMesh mesh;
	if (!mesh.Load(argv[1]))
	{
		cout << "Cannot load mesh " << argv[1] << endl;
		return -2;
	}

	cout << "Mesh Loaded." << endl;
	cout << "nVert: " << mesh.m_nVertex << endl;
	cout << "nFace: " << mesh.m_nFace << endl;

	unsigned e0 = mesh.m_pVertex[0].m_piEdge[0];
	mesh.split(e0);
	mesh.Save("test0.obj");
	e0 = mesh.m_pVertex[0].m_piEdge[1];
	mesh.split(e0);
	mesh.Save("test1.obj");
// 	unsigned v0 = mesh.m_pFace[0].m_piVertex[0];
// 	unsigned v1 = mesh.m_pFace[0].m_piVertex[1];
// 	unsigned v2 = mesh.m_pFace[0].m_piVertex[2];
// 
// 	Vector3D pos = (mesh.m_pVertex[v0].m_vPosition + mesh.m_pVertex[v1].m_vPosition +
// 		mesh.m_pVertex[v2].m_vPosition) / 3.0;
// 	mesh.add(pos, 0);
// 	mesh.Save((baseFileName + ".add.obj").c_str());
// 
// 	unsigned lastFace = mesh.m_nFace - 1;
// 
// 	v0 = mesh.m_pFace[lastFace].m_piVertex[0];
// 	v1 = mesh.m_pFace[lastFace].m_piVertex[1];
// 	v2 = mesh.m_pFace[lastFace].m_piVertex[2];
// 
// 	pos = (mesh.m_pVertex[v0].m_vPosition + mesh.m_pVertex[v1].m_vPosition +
// 		mesh.m_pVertex[v2].m_vPosition) / 3.0;
// 	mesh.add(pos, lastFace);
// 	mesh.Save((baseFileName + ".add1.obj").c_str());

// 	for (int i = 0; i < mesh.m_nVertex; ++i)
// 		mesh.m_pVertex[i].m_vPosition = Rotate(mesh.m_pVertex[i].m_vPosition, PI/2, Vector3D(1.0, 0.0, 0.0));
// 	mesh.Save("rotateTest.obj");
}