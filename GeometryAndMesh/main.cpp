#include "Mesh\Mesh.h"

using namespace std;

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		cout << "USAGE: [.exe] [.obj]" << endl;
		return -1;
	}

	CMesh mesh;
	if (!mesh.Load(argv[1]))
	{
		cout << "Cannot load mesh " << argv[1] << endl;
		return -2;
	}

	cout << "Mesh Loaded." << endl;
	cout << "nVert: " << mesh.m_nVertex << endl;
	cout << "nFace: " << mesh.m_nFace << endl;

	for (int i = 0; i < mesh.m_nVertex; ++i)
		mesh.m_pVertex[i].m_vPosition = Rotate(mesh.m_pVertex[i].m_vPosition, PI/2, Vector3D(1.0, 0.0, 0.0));
	mesh.Save("rotateTest.obj");
}