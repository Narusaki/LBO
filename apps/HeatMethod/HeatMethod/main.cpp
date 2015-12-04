#include <iostream>
#include <map>
#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <Eigen/Eigen/Sparse>
#include <ctime>

#include "Mesh.h"

using namespace std;

typedef Eigen::SparseMatrix<double> SparseMatrix;
typedef Eigen::Triplet<double> Triplet;

double CalcWeight(CMesh *mesh, unsigned edgeIndex)
{
	unsigned e[5];
	e[0] = edgeIndex;
	e[1] = mesh->m_pEdge[e[0]].m_iNextEdge;
	e[2] = mesh->m_pEdge[e[1]].m_iNextEdge;
	e[3] = -1, e[4] = -1;
	if (mesh->m_pEdge[e[0]].m_iTwinEdge != -1)
	{
		e[3] = mesh->m_pEdge[mesh->m_pEdge[e[0]].m_iTwinEdge].m_iNextEdge;
		e[4] = mesh->m_pEdge[e[3]].m_iNextEdge;
	}

	double l[5];
	for (int i = 0; i < 5; ++i) 
		l[i] = (e[i] != -1 ? mesh->m_pEdge[e[i]].m_length : 0);

	double cos1 = (l[1]*l[1] + l[2]*l[2] - l[0]*l[0]) / (2*l[1]*l[2]);
	double cos2 = (l[3] != 0 ? (l[3]*l[3] + l[4]*l[4] - l[0]*l[0]) / (2*l[3]*l[4]) : 0);

	double sin1 = sqrt(fabs(1-cos1*cos1));
	double sin2 = sqrt(fabs(1-cos2*cos2));
	
	return ((sin1 == 0 ? 0 : cos1/sin1) + (sin2 == 0 ? 0 : cos2/sin2)) / 2;
}

void ConstructLaplacian(CMesh *mesh, SparseMatrix &matrix)
{
	vector<Triplet> tripletList;
	vector<double> diag;
	diag.resize(mesh->m_nVertex, 0);

	for (unsigned i = 0; i < mesh->m_nEdge; ++i)
	{
		if (i > mesh->m_pEdge[i].m_iTwinEdge) continue;

		double wij = CalcWeight(mesh, i);
		unsigned v0 = mesh->m_pEdge[i].m_iVertex[0];
		unsigned v1 = mesh->m_pEdge[i].m_iVertex[1];
		tripletList.push_back(Triplet(v0, v1, wij));
		tripletList.push_back(Triplet(v1, v0, wij));
		diag[v0] -= wij; diag[v1] -= wij;
	}
	for (unsigned i = 0; i < diag.size(); ++i)
	{
		tripletList.push_back(Triplet(i, i, diag[i]));
	}

	matrix.setFromTriplets(tripletList.begin(), tripletList.end());
}

void ConstructAreaDiag(CMesh *mesh, SparseMatrix &matrix)
{
	vector<Triplet> tripletList;

	for (unsigned i = 0; i < mesh->m_nVertex; ++i)
	{
		double curArea = 0.0;
		for (unsigned j = 0; j < mesh->m_pVertex[i].m_nValence; ++j)
			curArea += mesh->m_pFace[mesh->m_pEdge[mesh->m_pVertex[i].m_piEdge[j]].m_iFace].m_dArea / 3.0;
		tripletList.push_back(Triplet(i, i, curArea));
	}
	matrix.setFromTriplets(tripletList.begin(), tripletList.end());
}

double CalcMeanEdgeLen(CMesh *mesh)
{
	double avgEdgeLen = 0;
	unsigned edgeCnt = 0;
	for (unsigned i = 0; i < mesh->m_nEdge; ++i)
		if (i < mesh->m_pEdge[i].m_iTwinEdge) { avgEdgeLen += mesh->m_pEdge[i].m_length; ++edgeCnt; }
	return avgEdgeLen/edgeCnt;
}

void ConstructDelta(CMesh *mesh, list<unsigned> sourceSet, Eigen::VectorXd &delta)
{
	for (unsigned i = 0; i < mesh->m_nVertex; ++i)
		delta[i] = 0.0;
	for (list<unsigned>::iterator iter = sourceSet.begin(); iter != sourceSet.end(); ++iter)
		delta[*iter] = 1.0;
}

int main(int argc, char **argv)
{
	if (argc < 5)
	{
		cout << "USAGE: [.exe] [in.obj] [srcFile] [step] [normalizing] [out.obj]" << endl;
		cout << "EXAMPLE: HeatMethod.exe bunny.obj bunny.sources.txt 1.0 1 bunny.heat.obj" << endl;
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
	bool normalizing = atoi(argv[4]);
	string baseFileName = argv[5];
	baseFileName = baseFileName.substr(0, baseFileName.length() - 4);

	SparseMatrix L(mesh->m_nVertex, mesh->m_nVertex);
	SparseMatrix A(mesh->m_nVertex, mesh->m_nVertex);
	Eigen::VectorXd delta(mesh->m_nVertex);

	ConstructLaplacian(mesh, L);
	ConstructAreaDiag(mesh, A);
	double h = CalcMeanEdgeLen(mesh);
	double t = step * h*h;

	Eigen::SimplicialCholesky<SparseMatrix> chol1, chol2;
	clock_t start = clock();
	chol1.compute(A - t * L);
	clock_t end = clock();
	cout << "Cholesky Decomposition Time: " << (double)(end - start) / (double)CLOCKS_PER_SEC << endl;

	start = clock();
	chol2.compute(L);
	end = clock();
	cout << "Cholesky Decomposition Time: " << (double)(end - start) / (double)CLOCKS_PER_SEC << endl;

	for (int ii = 0; ii < sourcesSet.size(); ++ii)
	{
		cout << "Computing Source Set " << ii << "......" << endl;
// 		for (list<unsigned>::iterator iter = sourcesSet[ii].begin(); iter != sourcesSet[ii].end(); ++iter)
// 			cout << *iter << " ";
// 		cout << endl;
		ConstructDelta(mesh, sourcesSet[ii], delta);
		start = clock();
		Eigen::VectorXd u = chol1.solve(delta);
		end = clock();
		cout << "Solving Time: " << (double)(end - start) / (double)CLOCKS_PER_SEC << endl;

		//Calculate X
		Vector3D *X = new Vector3D[mesh->m_nFace];
		for (unsigned i = 0; i < mesh->m_nFace; ++i)
		{
			X[i] = Vector3D(0, 0, 0);
			for (unsigned j = 0; j < 3; ++j)
			{
				unsigned ei = mesh->m_pFace[i].m_piEdge[j];
				unsigned vi = mesh->m_pEdge[mesh->m_pEdge[ei].m_iNextEdge].m_iVertex[1];
				Vector3D eveci = mesh->m_pVertex[mesh->m_pEdge[ei].m_iVertex[1]].m_vPosition - mesh->m_pVertex[mesh->m_pEdge[ei].m_iVertex[0]].m_vPosition;
				X[i] += u[vi] * (mesh->m_pFace[i].m_vNormal ^ eveci);
			}
			X[i] /= 2 * mesh->m_pFace[i].m_dArea;
			X[i].normalize(); X[i] *= -1;
		}

		//Calculate Gradiant*X
		Eigen::VectorXd GX(mesh->m_nVertex);

		for (unsigned i = 0; i < mesh->m_nVertex; ++i)
		{
			GX[i] = 0.0;
			for (unsigned j = 0; j < mesh->m_pVertex[i].m_nValence; ++j)
			{
				unsigned curFace = mesh->m_pEdge[mesh->m_pVertex[i].m_piEdge[j]].m_iFace;

				unsigned v[3], ei[3];
				for (int k = 0; k < 3; ++k)
				{
					if (mesh->m_pFace[curFace].m_piVertex[k] == i)
					{
						v[0] = i; v[1] = mesh->m_pFace[curFace].m_piVertex[(k+1)%3];
						v[2] = mesh->m_pFace[curFace].m_piVertex[(k+2)%3];
						ei[0] = mesh->m_pFace[curFace].m_piEdge[k];
						ei[1] = mesh->m_pFace[curFace].m_piEdge[(k+1)%3];
						ei[2] = mesh->m_pFace[curFace].m_piEdge[(k+2)%3];
						break;
					}
				}
				Vector3D e1 = mesh->m_pVertex[v[1]].m_vPosition - mesh->m_pVertex[v[0]].m_vPosition;
				Vector3D e2 = mesh->m_pVertex[v[2]].m_vPosition - mesh->m_pVertex[v[0]].m_vPosition;
				Vector3D e3 = mesh->m_pVertex[v[2]].m_vPosition - mesh->m_pVertex[v[1]].m_vPosition;

				double l1 = mesh->m_pEdge[ei[0]].m_length;
				double l2 = mesh->m_pEdge[ei[2]].m_length;
				double l3 = mesh->m_pEdge[ei[1]].m_length;

				/*double l1 = e1.length(), l2 = e2.length(), l3 = e3.length();*/

				double cos2 = (l1*l1+l3*l3-l2*l2) / (2*l1*l3);
				double cos1 = (l2*l2+l3*l3-l1*l1) / (2*l2*l3);

				double cot1 = cos1 >= 1.0 ? 0.0 : cos1/sqrt(1-cos1*cos1);
				double cot2 = cos2 >= 1.0 ? 0.0 : cos2/sqrt(1-cos2*cos2);

				GX[i] += cot1*(X[curFace]*e1) + cot2*(X[curFace]*e2);
			}
			GX[i] /= 2.0;
		}

		start = clock();
		Eigen::VectorXd phi = chol2.solve(GX);
		end = clock();
		cout << "Solving Time: " << (double)(end - start) / (double)CLOCKS_PER_SEC << endl;

		double maxDist = 0;
		double srcDist = phi[sourcesSet[ii].front()];
		for (unsigned i = 0; i < mesh->m_nVertex; ++i)
		{
			phi[i] -= srcDist;
			maxDist = max(maxDist, phi[i]);
		}

		if (normalizing)
		{
			for (unsigned i = 0; i < mesh->m_nVertex; ++i)
				phi[i] /= maxDist;
		}

		//output
		char outputMeshName[255];
		sprintf_s(outputMeshName, "%s_src%d.obj", baseFileName.c_str(), ii);
		ofstream output(outputMeshName);
		output << "mtllib texture.mtl" << endl;
		output << "usemtl Textured" << endl;
		for (unsigned i = 0; i < mesh->m_nVertex; ++i)
			output << "v " << mesh->m_pVertex[i].m_vPosition.x << " " << 
			mesh->m_pVertex[i].m_vPosition.y << " " << 
			mesh->m_pVertex[i].m_vPosition.z << endl;
		for (unsigned i = 0; i < mesh->m_nVertex; ++i)
			output << "vt " << phi[i] << " " << phi[i] << endl;
		for (unsigned i = 0; i < mesh->m_nFace; ++i)
			output << "f " << mesh->m_pFace[i].m_piVertex[0] + 1 << "/" << mesh->m_pFace[i].m_piVertex[0] + 1 << " "
			<< mesh->m_pFace[i].m_piVertex[1] + 1 << "/" << mesh->m_pFace[i].m_piVertex[1] + 1 << " "
			<< mesh->m_pFace[i].m_piVertex[2] + 1 << "/" << mesh->m_pFace[i].m_piVertex[2] + 1 << endl;
	}

	
	
	delete mesh;

	return 0;
}