// HeatMethod.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "HeatMethod.h"
#include <ctime>

using namespace std;

// This is the constructor of a class that has been exported.
// see HeatMethod.h for the class definition
HEATMETHOD_API HeatMethod::HeatMethod()
{
	lbo = NULL;
	return;
}

HEATMETHOD_API HeatMethod::~HeatMethod()
{
	if (lbo) delete lbo;
}

HEATMETHOD_API void HeatMethod::AssignMesh(CMesh *mesh_)
{
	mesh = mesh_;
	lbo = new LBO();
	lbo->AssignMesh(mesh_);
	X.resize(mesh->m_nFace);
	GX.resize(mesh->m_nVertex);
	delta.resize(mesh->m_nVertex);
}

HEATMETHOD_API void HeatMethod::AssignStep(double step_)
{
	step = step_;
}

HEATMETHOD_API void HeatMethod::PreFactor()
{
	double h = CalcMeanEdgeLen();
	double t = step * h*h;

	lbo->Construct();

	clock_t start = clock();
	chol1.compute(lbo->AMatrix() - t * lbo->LMatrix());
	clock_t end = clock();
	cout << "Cholesky Decomposition Time: " << (double)(end - start) / (double)CLOCKS_PER_SEC << endl;

	start = clock();
	chol2.compute(lbo->LMatrix());
	end = clock();
	cout << "Cholesky Decomposition Time: " << (double)(end - start) / (double)CLOCKS_PER_SEC << endl;
}

HEATMETHOD_API void HeatMethod::AssignSources(const std::list<unsigned int> &sources_)
{
	sources = sources_;
	ConstructDelta(delta);
}

HEATMETHOD_API void HeatMethod::BuildDistanceField()
{
	clock_t start = clock();
	Eigen::VectorXd u = chol1.solve(delta);
	clock_t end = clock();
	cout << "Solving Time: " << (double)(end - start) / (double)CLOCKS_PER_SEC << endl;

	//Calculate X
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
		X[i].normalizeStrict(); X[i] *= -1;
	}

	//Calculate Gradiant*X
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
					v[0] = i; v[1] = mesh->m_pFace[curFace].m_piVertex[(k + 1) % 3];
					v[2] = mesh->m_pFace[curFace].m_piVertex[(k + 2) % 3];
					ei[0] = mesh->m_pFace[curFace].m_piEdge[k];
					ei[1] = mesh->m_pFace[curFace].m_piEdge[(k + 1) % 3];
					ei[2] = mesh->m_pFace[curFace].m_piEdge[(k + 2) % 3];
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

			double cos2 = (l1*l1 + l3*l3 - l2*l2) / (2 * l1*l3);
			double cos1 = (l2*l2 + l3*l3 - l1*l1) / (2 * l2*l3);

			double cot1 = cos1 >= 1.0 ? 0.0 : cos1 / sqrt(1 - cos1*cos1);
			double cot2 = cos2 >= 1.0 ? 0.0 : cos2 / sqrt(1 - cos2*cos2);

			GX[i] += cot1*(X[curFace] * e1) + cot2*(X[curFace] * e2);
		}
		GX[i] /= 2.0;
	}

	start = clock();
	phi = chol2.solve(GX);
	end = clock();
	cout << "Solving Time: " << (double)(end - start) / (double)CLOCKS_PER_SEC << endl;

	double srcDist = phi[sources.front()];
	for (unsigned i = 0; i < mesh->m_nVertex; ++i)
		phi[i] -= srcDist;
}

HEATMETHOD_API const Eigen::VectorXd& HeatMethod::GetDistances()
{
	return phi;
}

HEATMETHOD_API double HeatMethod::CalcMeanEdgeLen()
{
	double avgEdgeLen = 0;
	unsigned edgeCnt = 0;
	for (unsigned i = 0; i < mesh->m_nEdge; ++i)
		if (i < mesh->m_pEdge[i].m_iTwinEdge) { avgEdgeLen += mesh->m_pEdge[i].m_length; ++edgeCnt; }
	return avgEdgeLen / edgeCnt;
}

HEATMETHOD_API void HeatMethod::ConstructDelta(Eigen::VectorXd &delta)
{
	for (unsigned i = 0; i < mesh->m_nVertex; ++i)
		delta[i] = 0.0;
	for (list<unsigned>::iterator iter = sources.begin(); iter != sources.end(); ++iter)
		delta[*iter] = 1.0;
}