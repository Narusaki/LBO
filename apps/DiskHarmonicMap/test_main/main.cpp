#include <iostream>
#include <DiskHarmonicMap.h>

using namespace std;

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		cout << "USAGE: [.exe] [in.obj]" << endl;
		return -1;
	}

	CMesh *mesh = new CMesh();
	if (!mesh->Load(argv[1]))
	{
		cout << "Cannot load mesh " << argv[1] << endl;
		return -2;
	}

	string baseFileName = argv[1];
	baseFileName = baseFileName.substr(0, baseFileName.length() - 4);

	DiskHarmonicMap harmonicMap;
	harmonicMap.AssignMesh(mesh);
	harmonicMap.Execute();

	char outputModel[255];
	sprintf(outputModel, "%s.harmonicmap.obj", baseFileName.c_str());
	ofstream output(outputModel);
	output << "mtllib texture.mtl" << endl;
	output << "usemtl Textured" << endl;
	for (unsigned i = 0; i < mesh->m_nVertex; ++i)
		output << "v " << mesh->m_pVertex[i].m_vPosition << endl;
	for (unsigned i = 0; i < mesh->m_nVertex; ++i)
		output << "vt " << harmonicMap.GetX1()[i] << " " << harmonicMap.GetX2()[i] << endl;
	for (unsigned i = 0; i < mesh->m_nFace; ++i)
		output << "f " << mesh->m_pFace[i].m_piVertex[0] + 1 << "/" << mesh->m_pFace[i].m_piVertex[0] + 1 << " "
		<< mesh->m_pFace[i].m_piVertex[1] + 1 << "/" << mesh->m_pFace[i].m_piVertex[1] + 1 << " "
		<< mesh->m_pFace[i].m_piVertex[2] + 1 << "/" << mesh->m_pFace[i].m_piVertex[2] + 1 << endl;
	return 0;
}