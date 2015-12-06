// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the SPHERICALHARMONICMAP_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// SPHERICALHARMONICMAP_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef SPHERICALHARMONICMAP_EXPORTS
#define SPHERICALHARMONICMAP_API __declspec(dllexport)
#else
#define SPHERICALHARMONICMAP_API __declspec(dllimport)
#endif

#include <LBO.h>
#include <vector>

// This class is exported from the SphericalHarmonicMap.dll
class SPHERICALHARMONICMAP_API SphericalHarmonicMap {
public:
	typedef Eigen::SparseMatrix<double> SparseMatrix;

	SphericalHarmonicMap();
	~SphericalHarmonicMap();

	void AssignMesh(CMesh *mesh_);
	void SetRelativeEnergyVar(double energyVar_) { energyVar = energyVar_; }
	void SetIterationLimit(int iterLimit_) { iterLimit = iterLimit_; }
	void SetStep(double step_) { step = step_; }

	void Execute();

	const Eigen::Matrix<double, Eigen::Dynamic, 3>& GetNormals() { return normals; }

private:
	double CalcEnergy();

private:

	CMesh *mesh;
	LBO *lbo;

	double energyVar;
	int iterLimit;
	double step;

	Eigen::Matrix<double, Eigen::Dynamic, 3> normals;
};
