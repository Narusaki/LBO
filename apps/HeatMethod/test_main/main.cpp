#include <iostream>
#include <HeatMethod.h>
#include <algorithm>

using namespace std;

int main(int argc, char **argv)
{
	if (argc < 4)
	{
		cout << "USAGE: [.exe] [in.obj] [srcFile] [step]" << endl;
		cout << "EXAMPLE: test_main.exe bunny.obj bunny.sources.txt 1.0" << endl;
		return -1;
	}
	CMesh *mesh = new CMesh();
	if (!mesh->Load(argv[1]))
	{
		cout << "Cannot load mesh " << argv[1] << endl;
		return -2;
	}

	vector< list<unsigned> > sourcesSet;
	// load source
	ifstream input(argv[2]);
	stringstream sin;
	string curLine;
	while (getline(input, curLine))
	{
		list<unsigned> curSetOfSource;
		sin.clear();
		sin << curLine;
		unsigned curSrc;
		while (sin >> curSrc)
			curSetOfSource.push_back(curSrc);
		sourcesSet.push_back(curSetOfSource);
	}
	input.close();

	double step = atof(argv[3]);
	string baseFileName = argv[1];
	baseFileName = baseFileName.substr(0, baseFileName.length() - 4);

	HeatMethod heatMethod;
	heatMethod.AssignMesh(mesh);
	heatMethod.AssignStep(step);
	heatMethod.SetOutputInfo(true);
	heatMethod.PreFactor();

	for (int i = 0; i < sourcesSet.size(); ++i)
	{
		cout << "Computing Source Set " << i << "......" << endl;

		heatMethod.AssignSources(sourcesSet[i]);
		heatMethod.BuildDistanceField();

		const Eigen::VectorXd &dist = heatMethod.GetDistances();
		double maxDist = *max_element(&dist[0], &dist[dist.size()-1]);

		//output
		char outputMeshName[255];
		sprintf_s(outputMeshName, "%s_src%d.obj", baseFileName.c_str(), i);
		ofstream output(outputMeshName);
		output << "mtllib texture.mtl" << endl;
		output << "usemtl Textured" << endl;
		for (unsigned i = 0; i < mesh->m_nVertex; ++i)
			output << "v " << mesh->m_pVertex[i].m_vPosition << endl;
		for (unsigned i = 0; i < mesh->m_nVertex; ++i)
			output << "vt " << dist[i]/maxDist << " " << dist[i]/maxDist << endl;
		for (unsigned i = 0; i < mesh->m_nFace; ++i)
			output << "f " << mesh->m_pFace[i].m_piVertex[0] + 1 << "/" << mesh->m_pFace[i].m_piVertex[0] + 1 << " "
			<< mesh->m_pFace[i].m_piVertex[1] + 1 << "/" << mesh->m_pFace[i].m_piVertex[1] + 1 << " "
			<< mesh->m_pFace[i].m_piVertex[2] + 1 << "/" << mesh->m_pFace[i].m_piVertex[2] + 1 << endl;
		output.close();
	}

	delete mesh;
	return 0;
}
