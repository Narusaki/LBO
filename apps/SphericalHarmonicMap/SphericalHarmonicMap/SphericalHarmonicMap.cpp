// SphericalHarmonicMap.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "SphericalHarmonicMap.h"
#include <iostream>

using namespace std;

// This is the constructor of a class that has been exported.
// see SphericalHarmonicMap.h for the class definition
SphericalHarmonicMap::SphericalHarmonicMap()
{
	lbo = NULL;
	mesh = NULL;
}

SphericalHarmonicMap::~SphericalHarmonicMap()
{
	if (lbo) delete lbo;
}

void SphericalHarmonicMap::AssignMesh(CMesh *mesh_)
{
	mesh = mesh_;
	lbo = new LBO();
	lbo->AssignMesh(mesh);
	normals.resize(mesh->m_nVertex, 3);
}

void SphericalHarmonicMap::Execute()
{
	lbo->Construct();

	bool hasRawNormal = mesh->m_pVertex[0].m_vRawNormal.length() != 0.0;
	if (hasRawNormal)
	{
		for (int i = 0; i < mesh->m_nVertex; ++i)
		{
			for (int j = 0; j < 3; ++j)
				normals(i, j) = mesh->m_pVertex[i].m_vRawNormal[j];
		}
	}
	else
	{
		for (int i = 0; i < mesh->m_nVertex; ++i)
		{
			for (int j = 0; j < 3; ++j)
				normals(i, j) = mesh->m_pVertex[i].m_vNormal[j];
		}
	}

	Eigen::Matrix<double, Eigen::Dynamic, 3> newNormals;
	double curEnergy = CalcEnergy(), prevEnergy = 0.0;
	unsigned iteration = 0;
	double curEnergyVar = 0.0;

	do 
	{
		prevEnergy = curEnergy;

		newNormals = -lbo->LMatrix() * normals;
		for (int i = 0; i < normals.rows(); ++i)
		{
			normals.row(i) = normals.row(i) - (newNormals.row(i) - newNormals.row(i).dot(normals.row(i)) * normals.row(i)) * step;
			normals.row(i).normalize();
		}
		
		curEnergy = CalcEnergy();
		curEnergyVar = fabs(curEnergy - prevEnergy) / fabs(prevEnergy);
		printf("iteration %d, energy: %f, energy var: %f\n", iteration++, curEnergy, curEnergyVar);
	} while (iteration < iterLimit && curEnergyVar > energyVar);

}

double SphericalHarmonicMap::CalcEnergy()
{
	double energy = 0.0;
	for (unsigned i = 0; i < mesh->m_nEdge; ++i)
	{
		if (i > mesh->m_pEdge[i].m_iTwinEdge) continue;
		unsigned v0 = mesh->m_pEdge[i].m_iVertex[0];
		unsigned v1 = mesh->m_pEdge[i].m_iVertex[1];

		double w = lbo->LMatrix().coeff(v0, v1);
		energy += -lbo->LMatrix().coeff(v0, v1) * 
			(normals.row(v0) - normals.row(v1)).dot(normals.row(v0) - normals.row(v1));
	}
	return energy;
}