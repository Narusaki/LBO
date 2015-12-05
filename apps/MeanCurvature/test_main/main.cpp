#include <iostream>
#include <MeanCurvature.h>

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

	MeanCurvature meanCurvature;
	meanCurvature.AssignMesh(mesh);
	meanCurvature.Execute();

	double maxMeanCurvature = 0.0;
	for (int i = 0; i < mesh->m_nVertex; ++i)
		maxMeanCurvature = __max(maxMeanCurvature, meanCurvature.GetMeanCurvature()[i].length());

	char outputModel[255];
	sprintf_s(outputModel, "%s.curvature.obj", baseFileName.c_str());
	ofstream output(outputModel);
	output << "mtllib texture.mtl" << endl;
	output << "usemtl Textured" << endl;
	for (unsigned i = 0; i < mesh->m_nVertex; ++i)
		output << "v " << mesh->m_pVertex[i].m_vPosition << endl;
	for (unsigned i = 0; i < mesh->m_nVertex; ++i)
		output << "vt " << meanCurvature.GetMeanCurvature()[i].length() / maxMeanCurvature << " "
		<< meanCurvature.GetMeanCurvature()[i].length() / maxMeanCurvature << endl;
	for (unsigned i = 0; i < mesh->m_nFace; ++i)
		output << "f " << mesh->m_pFace[i].m_piVertex[0] + 1 << "/" << mesh->m_pFace[i].m_piVertex[0] + 1 << " "
		<< mesh->m_pFace[i].m_piVertex[1] + 1 << "/" << mesh->m_pFace[i].m_piVertex[1] + 1 << " "
		<< mesh->m_pFace[i].m_piVertex[2] + 1 << "/" << mesh->m_pFace[i].m_piVertex[2] + 1 << endl;
	output.close();

	sprintf_s(outputModel, "%s.curvature", baseFileName.c_str());
	output.open(outputModel);
	for (unsigned i = 0; i < mesh->m_nVertex; ++i)
		output << meanCurvature.GetMeanCurvature()[i] << " " << meanCurvature.GetMeanCurvature()[i].length() << endl;
	output.close();

	return 0;
}