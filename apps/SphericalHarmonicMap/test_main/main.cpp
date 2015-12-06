#include <iostream>
#include <SphericalHarmonicMap.h>

using namespace std;

int main(int argc, char **argv)
{
	if (argc < 5)
	{
		cout << "USAGE: [.exe] [in.obj] [step] [energyVar] [iterLimit]" << endl;
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

	SphericalHarmonicMap harmonicMap;
	harmonicMap.AssignMesh(mesh);
	harmonicMap.SetStep(atof(argv[2]));
	harmonicMap.SetRelativeEnergyVar(atof(argv[3]));
	harmonicMap.SetIterationLimit(atoi(argv[4]));
	harmonicMap.Execute();

	char outputModel[255];
	sprintf(outputModel, "%s.sphereharmonic.obj", baseFileName.c_str());
	ofstream output(outputModel);
	output << "mtllib texture.mtl" << endl;
	output << "usemtl Textured" << endl;
	for (unsigned i = 0; i < mesh->m_nVertex; ++i)
		output << "v " << harmonicMap.GetNormals()(i, 0) << " "
		<< harmonicMap.GetNormals()(i, 1) << " "
		<< harmonicMap.GetNormals()(i, 2) << endl;
	for (unsigned i = 0; i < mesh->m_nFace; ++i)
		output << "f " << mesh->m_pFace[i].m_piVertex[0] + 1 << " "
		<< mesh->m_pFace[i].m_piVertex[1] + 1 << " "
		<< mesh->m_pFace[i].m_piVertex[2] + 1 << endl;
	return 0;
}