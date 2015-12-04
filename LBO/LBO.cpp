// LBO.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "LBO.h"

using namespace std;


// This is an example of an exported variable
// LBO_API int nLBO=0;
// 
// This is an example of an exported function.
// LBO_API int fnLBO(void)
// {
// 	return 42;
// }

// This is the constructor of a class that has been exported.
// see LBO.h for the class definition
LBO_API LBO::LBO()
{
	return;
}

LBO_API void LBO::AssignMesh(CMesh *mesh_)
{
	mesh = mesh_;
	L.resize(mesh->m_nVertex, mesh->m_nVertex);
	A.resize(mesh->m_nVertex, mesh->m_nVertex);

}

LBO_API void LBO::Construct()
{
	ConstructLaplacian();
	ConstructAreaDiag();
}

LBO_API double LBO::CalcCotWeight(unsigned edgeIndex)
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

	double cos1 = (l[1] * l[1] + l[2] * l[2] - l[0] * l[0]) / (2 * l[1] * l[2]);
	double cos2 = (l[3] != 0 ? (l[3] * l[3] + l[4] * l[4] - l[0] * l[0]) / (2 * l[3] * l[4]) : 0);

	double sin1 = sqrt(fabs(1 - cos1*cos1));
	double sin2 = sqrt(fabs(1 - cos2*cos2));

	return ((sin1 == 0 ? 0 : cos1 / sin1) + (sin2 == 0 ? 0 : cos2 / sin2)) / 2;
}

LBO_API void LBO::ConstructLaplacian()
{
	vector<Triplet> tripletList;
	vector<double> diag;
	diag.resize(mesh->m_nVertex, 0);

	for (unsigned i = 0; i < mesh->m_nEdge; ++i)
	{
		if (i > mesh->m_pEdge[i].m_iTwinEdge) continue;

		double wij = CalcCotWeight(i);
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

	L.setFromTriplets(tripletList.begin(), tripletList.end());
}

LBO_API void LBO::ConstructAreaDiag()
{
	vector<Triplet> tripletList;

	for (unsigned i = 0; i < mesh->m_nVertex; ++i)
	{
		double curArea = 0.0;
		for (unsigned j = 0; j < mesh->m_pVertex[i].m_nValence; ++j)
			curArea += mesh->m_pFace[mesh->m_pEdge[mesh->m_pVertex[i].m_piEdge[j]].m_iFace].m_dArea / 3.0;
		tripletList.push_back(Triplet(i, i, curArea));
	}
	A.setFromTriplets(tripletList.begin(), tripletList.end());
}