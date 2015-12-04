// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the DISKHARMONICMAP_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// DISKHARMONICMAP_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef DISKHARMONICMAP_EXPORTS
#define DISKHARMONICMAP_API __declspec(dllexport)
#else
#define DISKHARMONICMAP_API __declspec(dllimport)
#endif

#include <LBO.h>
#include <vector>

// This class is exported from the DiskHarmonicMap.dll
class DISKHARMONICMAP_API DiskHarmonicMap {
public:
	typedef Eigen::SparseMatrix<double> SparseMatrix;
	typedef Eigen::Triplet<double> Triplet;

	DiskHarmonicMap();
	~DiskHarmonicMap();
	
	void AssignMesh(CMesh *mesh_);
	void Execute();

	const Eigen::VectorXd & GetX1() { return x1; }
	const Eigen::VectorXd & GetX2() { return x2; }

private:

	// create boundaries. Note that here only one of the boundaries will be used if there are multiple.
	bool ConstructBoundaryConditions(std::vector<unsigned> &boundVerts, std::vector<double> &thetas);
	// construct boundary conditions
	void ConstructB(Eigen::VectorXd &b1, Eigen::VectorXd &b2, 
		std::vector<unsigned> &boundVerts, std::vector<double> &thetas);
	// calculate distortion
	void computeDistortion(Eigen::VectorXd &vt1, Eigen::VectorXd &vt2, double &angleDistortion, double &areaDistortion);

private:
	CMesh *mesh;
	LBO *lbo;

	Eigen::VectorXd x1, x2;
};
