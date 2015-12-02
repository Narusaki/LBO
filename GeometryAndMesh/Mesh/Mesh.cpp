// Mesh.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "Mesh.h"

#include <map>
#include <cassert>
#include <iomanip>

using namespace std;

#define ITERATE_TIMES 1
#define	ALPHA_STRETCH 1.0

//////////////////////////////////////////////////////
//						CVertex						//
//////////////////////////////////////////////////////
CVertex::~CVertex()
{
	if (m_piEdge != NULL)
		delete[] m_piEdge;
	m_lEdgeList.clear();
}

CVertex& CVertex::operator = (CVertex& v)
{
	m_vPosition = v.m_vPosition;//
	m_nValence = v.m_nValence;//Ķ
	m_piEdge = new UINT[m_nValence];//Ӹõ㷢halfedge,ҪݵĶ̬
	for (short i = 0; i<m_nValence; i++)
		m_piEdge[i] = v.m_piEdge[i];
	//	m_lEdgeList = v.m_lEdgeList;//m_piEdgeʱ

	m_bIsBoundary = v.m_bIsBoundary;
	m_vNormal = v.m_vNormal;

	m_nCutValence = v.m_nCutValence;

	m_color = v.m_color;
	m_bValid = v.m_bValid;

	return *this;
}


//////////////////////////////////////////////////////
//						CEdge						//
//////////////////////////////////////////////////////

CEdge::~CEdge()
{
}

CEdge& CEdge::operator = (const CEdge& e)
{
	m_iVertex[0] = e.m_iVertex[0];//ߵ˵㣬Vertex0>Vertex1
	m_iVertex[1] = e.m_iVertex[1];
	m_iTwinEdge = e.m_iTwinEdge;//ñ߷෴һߣΪ-1ñΪ߽
	m_iNextEdge = e.m_iNextEdge;//ʱ뷽һ
	m_iFace = e.m_iFace;//ñ棬Ӧ
	m_bCut = e.m_bCut;
	m_nCutTag = e.m_nCutTag;
	m_color = e.m_color;
	m_length = e.m_length;
	m_bValid = e.m_bValid;

	return *this;
}

//////////////////////////////////////////////////////
//						CFace						//
//////////////////////////////////////////////////////
CFace::CFace(short s)
{
	m_nType = s;
	m_vNormal = Vector3D(0.0, 0.0, 1.0);
	m_dArea = 0.0;
	m_piEdge = new UINT[s];
	m_piVertex = new UINT[s];
	m_bValid = true;
}

CFace::~CFace()
{
	if (m_piEdge != NULL)
		delete[] m_piEdge;
	if (m_piVertex != NULL)
		delete[] m_piVertex;
	if (m_pdAngle != NULL)
		delete[] m_pdAngle;
}

void CFace::Create(short s)
{
	m_nType = s;
	m_vNormal = Vector3D(0.0, 0.0, 1.0);
	m_dArea = 0.0;
	m_piEdge = new UINT[s];
	m_piVertex = new UINT[s];
	m_pdAngle = new double[s];
}

CFace& CFace::operator =(const CFace& f)
{
	m_nType = f.m_nType;//
	m_piVertex = new UINT[m_nType];//е
	m_piEdge = new UINT[m_nType];//б
	for (short i = 0; i<m_nType; i++)
	{
		m_piVertex[i] = f.m_piVertex[i];
		m_piEdge[i] = f.m_piEdge[i];
	}

	m_vNormal = f.m_vNormal;//
	m_vMassPoint = f.m_vMassPoint;//
	m_dArea = f.m_dArea;//
	m_color = f.m_color;
	m_bValid = f.m_bValid;

	return (*this);
}

//////////////////////////////////////////////////////
//						CMesh						//
//////////////////////////////////////////////////////
void CMesh::clear()
{
	if (m_pVertex != NULL)	{ delete[] m_pVertex; m_pVertex = NULL; }
	if (m_pEdge != NULL)	{ delete[] m_pEdge; m_pEdge = NULL; }
	if (m_pFace != NULL)	{ delete[] m_pFace; m_pFace = NULL; }
	if (m_pAngles != NULL) { delete[] m_pAngles; m_pAngles = NULL; }
	m_nVertex = m_nEdge = m_nFace = 0;
	m_lFocusEdge.clear();
	m_lFocusVertex.clear();
}

CMesh::CMesh(CMesh* pMesh)
{
	m_nVertexCapacity = pMesh->m_nVertexCapacity;
	m_nEdgeCapacity = pMesh->m_nEdgeCapacity;
	m_nFaceCapacity = pMesh->m_nFaceCapacity;

	m_nVertex = pMesh->m_nVertex;				//
	m_pVertex = new CVertex[m_nVertexCapacity];			//
	UINT i;
	for (i = 0; i<m_nVertex; i++)
		m_pVertex[i] = pMesh->m_pVertex[i];
	m_pAngles = new double[m_nVertexCapacity];
	for (i = 0; i < m_nVertex; ++i)
		m_pAngles[i] = pMesh->m_pAngles[i];

	m_nEdge = pMesh->m_nEdge;					//
	m_pEdge = new CEdge[m_nEdgeCapacity]; 				//߱
	for (i = 0; i<m_nEdge; i++)
		m_pEdge[i] = pMesh->m_pEdge[i];

	m_nFace = pMesh->m_nFace;	 				//
	m_pFace = new CFace[m_nFaceCapacity];				//
	for (i = 0; i<m_nFace; i++)
		m_pFace[i] = pMesh->m_pFace[i];

// 	m_pAngles = new double[m_nVertex];
// 	for (i = 0; i < m_nVertex; ++i)
// 		m_pAngles[i] = pMesh->m_pAngles[i];

	m_lFocusEdge = pMesh->m_lFocusEdge;
	m_lFocusVertex = pMesh->m_lFocusVertex;
	m_lFocusFace = pMesh->m_lFocusFace;
	//	_UINTLIST m_lPickedVertex;

	m_iPickedVertex = pMesh->m_iPickedVertex;
	m_iPickedEdge = pMesh->m_iPickedEdge;
	m_iPickedFace = pMesh->m_iPickedFace;

	filename = pMesh->filename;
	isVisible = pMesh->isVisible;

	m_pTexture = pMesh->m_pTexture;
	maxTex = pMesh->maxTex;

	color = pMesh->color;
	isVisible = pMesh->isVisible;
	m_nValidVertNum = pMesh->m_nValidVertNum;
	m_nValidFaceNum = pMesh->m_nValidFaceNum;
	m_nValidEdgeNum = pMesh->m_nValidEdgeNum;
	isolatedPoints = pMesh->isolatedPoints;
	scaleD = pMesh->scaleD;
	origin = pMesh->origin;
	bboxSize = pMesh->bboxSize;
	m_bClosed = pMesh->m_bClosed;
}

CMesh* CMesh::clone()
{
	return NULL;
}

CMesh::~CMesh()
{
	clear();
}

bool CMesh::Load(const char* sFileName)
{
	clear();
	filename = sFileName;

	std::string suffix = filename.substr(filename.rfind('.'), filename.length() - filename.rfind('.'));
	if (suffix == ".obj" || suffix == ".OBJ")
		return loadOBJ(sFileName);
	if (suffix == ".off" || suffix == ".OFF")
		return loadOFF(sFileName);
}

bool CMesh::loadOFF(const char* sFileName)
{
	_VECTORLIST VertexList;
	_UINTLIST FaceList;
	std::vector<Vector3D> normals;

	//open the file
	filename = sFileName;
	ifstream fin;
	fin.open(sFileName);
	if (!fin) return false;

	Vector3D vector;
	Vector2D vector2d;
	Vector3D vMin(1e30, 1e30, 1e30);
	Vector3D vMax(-1e30, -1e30, -1e30);
	bool bFirst = true;

	maxTex = CTexture(0, 0);

	UINT l[3];
	short j;

	std::string curLine;
	getline(fin, curLine);
	getline(fin, curLine);
	int nVert = atoi(curLine.substr(0, curLine.find(" ")).c_str());
	curLine = curLine.substr(curLine.find(" ") + 1, curLine.length() - curLine.find(" ") - 1);
	int nFace = atoi(curLine.substr(0, curLine.find(" ")).c_str());

	for (int i = 0; i < nVert; ++i)
	{
		getline(fin, curLine);
		vector.x = atof(curLine.substr(0, curLine.find(" ")).c_str());
		curLine = curLine.substr(curLine.find(" ") + 1, curLine.length() - curLine.find(" ") - 1);
		vector.y = atof(curLine.substr(0, curLine.find(" ")).c_str());
		curLine = curLine.substr(curLine.find(" ") + 1, curLine.length() - curLine.find(" ") - 1);
		vector.z = atof(curLine.c_str());

		if (vector.x < vMin.x) vMin.x = vector.x;
		if (vector.x > vMax.x) vMax.x = vector.x;
		if (vector.y < vMin.y) vMin.y = vector.y;
		if (vector.y > vMax.y) vMax.y = vector.y;
		if (vector.z < vMin.z) vMin.z = vector.z;
		if (vector.z > vMax.z) vMax.z = vector.z;

		VertexList.push_back(vector);
	}

	for (int i = 0; i < nFace; ++i)
	{
		getline(fin, curLine);
		curLine = curLine.substr(curLine.find(" ") + 1, curLine.length() - curLine.find(" ") - 1);
		l[0] = atoi(curLine.substr(0, curLine.find(" ")).c_str());
		curLine = curLine.substr(curLine.find(" ") + 1, curLine.length() - curLine.find(" ") - 1);
		l[1] = atoi(curLine.substr(0, curLine.find(" ")).c_str());
		curLine = curLine.substr(curLine.find(" ") + 1, curLine.length() - curLine.find(" ") - 1);
		l[2] = atoi(curLine.c_str());
		FaceList.push_back(l[0]); FaceList.push_back(l[1]); FaceList.push_back(l[2]);
	}

	vector = vMax - vMin;
	scaleD = max(vector.x, vector.y);
	scaleD = 0.5*max(scaleD, vector.z);
	bboxSize = vector;
	origin = 0.5*(vMax + vMin);//ƫ

	return loadFromSMF(VertexList, FaceList, normals);
}

bool CMesh::loadOBJ(const char* sFileName)
{
	_VECTORLIST VertexList;
	_UINTLIST FaceList;
	std::vector<Vector3D> normals;

	//open the file
	filename = sFileName;
	ifstream fin;
	fin.open(sFileName);
	if (!fin) return false;

	Vector3D vector;
	Vector2D vector2d;
	Vector3D vMin(1e30, 1e30, 1e30);
	Vector3D vMax(-1e30, -1e30, -1e30);
	bool bFirst = true;

	maxTex = CTexture(0, 0);

	UINT l[3];
	char tmpC;
	short j;

	std::map<Vector3D, UINT> redunVertex;
	std::vector<UINT> realIndex;
	UINT vertexIndex = 0;

	string buf;
	while (getline(fin, buf))
	{
		if (buf.size() > 0) //non-empty line
		{
			if (buf[0] == 'v' && buf[1] == ' ')//A line starting with a 'v' designating a vertex position information
			{
				//%S: S same as ls. A wide character string is expected; the corresponding argument should be a wide character pointer pointing to an array of wide characters large enough to accept the wide character string and a terminating \0
				istringstream sin(buf);
				sin >> tmpC >> vector.x >> vector.y >> vector.z;
// 				if(bFirst)
// 				{
// 					bFirst=false;
// 					vMin=vMax=vector;
// 					continue;
// 				}
// 				map<Vector3D, UINT>::iterator iter = redunVertex.find(vector);
// 				if (iter == redunVertex.end())
// 				{
				redunVertex[vector] = vertexIndex;
				realIndex.push_back(vertexIndex++);
				VertexList.push_back(Vector3D(vector));
// 				}
// 				else
// 					realIndex.push_back(iter->second);

				if (vector.x<vMin.x) vMin.x = vector.x;
				if (vector.x>vMax.x) vMax.x = vector.x;
				if (vector.y<vMin.y) vMin.y = vector.y;
				if (vector.y>vMax.y) vMax.y = vector.y;
				if (vector.z<vMin.z) vMin.z = vector.z;
				if (vector.z>vMax.z) vMax.z = vector.z;
			}
			else if (buf[0] == 'v' && buf[1] == 't')
			{
				string buftmp = buf.substr(3, buf.length() - 3);
				istringstream sin(buftmp);
				sin >> vector2d.x >> vector2d.y;
				CTexture curTexture(vector2d);
				m_pTexture.push_back(curTexture);

				if (vector2d.x > maxTex.m_vPosition.x) maxTex.m_vPosition.x = vector2d.x;
				if (vector2d.y > maxTex.m_vPosition.y) maxTex.m_vPosition.y = vector2d.y;
			}
			else if (buf[0] == 'v' && buf[1] == 'n')
			{
				istringstream sin(buf);
				sin >> tmpC >> tmpC >> vector.x >> vector.y >> vector.z;
				normals.push_back(vector);
			}
			else if (buf[0] == 'f')
			{
				istringstream sin(buf);
				int tx, ty, tz;
				size_t slashPos = buf.find("/");
				if (slashPos == string::npos) sscanf(buf.c_str(), "f %d %d %d", &l[0], &l[1], &l[2]);
				else if (buf[slashPos + 1] != '/') sscanf(buf.c_str(), "f %d/%d %d/%d %d/%d", &l[0], &tx, &l[1], &ty, &l[2], &tz);
				else if (buf[slashPos + 2] != '/') sscanf(buf.c_str(), "f %d//%d %d//%d %d//%d", &l[0], &tx, &l[1], &ty, &l[2], &tz);

				if (realIndex[l[0] - 1] == realIndex[l[1] - 1] || realIndex[l[1] - 1] == realIndex[l[2] - 1] || realIndex[l[2] - 1] == realIndex[l[0] - 1]) continue;
				//sin>>tmpC>>l[0]>>l[1]>>l[2];
				for (j = 0; j < 3; j++)
					FaceList.push_back(realIndex[l[j] - 1]);
			}
			else if (buf[0] == 'p')
			{
				istringstream sin(buf);
				sin >> tmpC >> vector.x >> vector.y >> vector.z;
				isolatedPoints.push_back(vector);
			}
		}
	}
	fin.close();

	assert(normals.empty() || VertexList.size() == normals.size());

	vector = vMax - vMin;
	scaleD = max(vector.x, vector.y);
	scaleD = 0.5*max(scaleD, vector.z);
	bboxSize = vector;
	origin = 0.5*(vMax + vMin);//ƫ

	return loadFromSMF(VertexList, FaceList, normals);
}

bool CMesh::loadFromSMF(std::list<Vector3D> &VertexList, std::list<UINT> &FaceList, std::vector<Vector3D> &normals)
// -----  format: smf,obj,dat -----
//vertex:
//      v x y z,
//face(triangle):
//      f v1 v2 v3  (the vertex index is 1-based)
{
	m_nVertex = (UINT)VertexList.size();
	m_nFace = (UINT)FaceList.size() / 3;
	m_nEdge = (UINT)3 * m_nFace;

	m_nVertexCapacity = m_nVertex * 2;
	m_nEdgeCapacity = m_nEdge * 2;
	m_nFaceCapacity = m_nFace * 2;

	//read vertices and faces
	m_pVertex = new CVertex[m_nVertexCapacity];
	if (m_pVertex == NULL) { clear(); return false; }//out of memory
	m_pFace = new CFace[m_nFaceCapacity];
	if (m_pFace == NULL) { clear(); return false; }//out of memory
	m_pAngles = new double[m_nVertexCapacity];
	if (!m_pAngles) { clear(); return false; }
	memset(m_pAngles, 0, sizeof(double) * m_nVertexCapacity);

	UINT i;
	_VECTORLIST::iterator iVertex = VertexList.begin();
	_UINTLIST::iterator iFace = FaceList.begin();

	for (i = 0; i<m_nVertex; i++)
		/*m_pVertex[i].m_vPosition=(*(iVertex++)-vector)/d;*/
		m_pVertex[i].m_vPosition = *(iVertex++);
	for (i = 0; i < isolatedPoints.size(); ++i)
		isolatedPoints[i] = (isolatedPoints[i] - origin) / scaleD;
	for (i = 0; i<m_nFace; i++)
	{
		m_pFace[i].Create(3);
		for (int j = 0; j<3; j++)
			m_pFace[i].m_piVertex[j] = *iFace++;
	}

	VertexList.clear();
	FaceList.clear();
	if (!construct()) return false;

	if (!normals.empty())
	{
		for (unsigned i = 0; i < m_nVertex; ++i)
		{
			m_pVertex[i].m_vRawNormal = normals[i];
			/*m_pVertex[i].m_vRawNormal.normalize();*/
		}
	}
	return true;
}

bool CMesh::Save(const char* sFileName)
{
	//this->filename = sFileName;
	ofstream fout(sFileName);
	fout << "# vertices: " << m_nValidVertNum << endl;
	fout << "# faces: " << m_nValidFaceNum << endl;

	std::vector<unsigned> realIndex;
	unsigned idx = 0;
	for (UINT i = 0; i < m_nVertex; ++i) {
		realIndex.push_back(idx);
		if (!m_pVertex[i].m_bValid) continue;
		Vector3D v = m_pVertex[i].m_vPosition;
		fout << "v " << setprecision(20) << v.x << " " << v.y << " " << v.z << endl;
		++idx;
	}
	for (UINT i = 0; i < m_nFace; ++i) {
		if (!m_pFace[i].m_bValid) continue;
		UINT* v = m_pFace[i].m_piVertex;
		fout << "f " << realIndex[v[0]] + 1 << " " << realIndex[v[1]] + 1 << " " << realIndex[v[2]] + 1 << endl;
	}
	fout.close();
	return true;
}

MESH_API std::istream& operator >> (std::istream &input, CMesh &mesh)
{
	_VECTORLIST VertexList;
	_UINTLIST FaceList;
	std::vector<Vector3D> normals;

	Vector3D vector;
	Vector2D vector2d;
	Vector3D vMin(1e30, 1e30, 1e30);
	Vector3D vMax(-1e30, -1e30, -1e30);
	bool bFirst = true;

	mesh.maxTex = CTexture(0, 0);

	UINT l[3];
	short j;

	std::string curLine;
	getline(input, curLine);
	getline(input, curLine);
	int nVert = atoi(curLine.substr(0, curLine.find(" ")).c_str());
	curLine = curLine.substr(curLine.find(" ") + 1, curLine.length() - curLine.find(" ") - 1);
	int nFace = atoi(curLine.substr(0, curLine.find(" ")).c_str());

	for (int i = 0; i < nVert; ++i)
	{
		getline(input, curLine);
		vector.x = atof(curLine.substr(0, curLine.find(" ")).c_str());
		curLine = curLine.substr(curLine.find(" ") + 1, curLine.length() - curLine.find(" ") - 1);
		vector.y = atof(curLine.substr(0, curLine.find(" ")).c_str());
		curLine = curLine.substr(curLine.find(" ") + 1, curLine.length() - curLine.find(" ") - 1);
		vector.z = atof(curLine.c_str());

		if (vector.x < vMin.x) vMin.x = vector.x;
		if (vector.x > vMax.x) vMax.x = vector.x;
		if (vector.y < vMin.y) vMin.y = vector.y;
		if (vector.y > vMax.y) vMax.y = vector.y;
		if (vector.z < vMin.z) vMin.z = vector.z;
		if (vector.z > vMax.z) vMax.z = vector.z;

		VertexList.push_back(vector);
	}

	for (int i = 0; i < nFace; ++i)
	{
		getline(input, curLine);
		curLine = curLine.substr(curLine.find(" ") + 1, curLine.length() - curLine.find(" ") - 1);
		l[0] = atoi(curLine.substr(0, curLine.find(" ")).c_str());
		curLine = curLine.substr(curLine.find(" ") + 1, curLine.length() - curLine.find(" ") - 1);
		l[1] = atoi(curLine.substr(0, curLine.find(" ")).c_str());
		curLine = curLine.substr(curLine.find(" ") + 1, curLine.length() - curLine.find(" ") - 1);
		l[2] = atoi(curLine.c_str());
		FaceList.push_back(l[0]); FaceList.push_back(l[1]); FaceList.push_back(l[2]);
	}

	vector = vMax - vMin;
	mesh.scaleD = max(vector.x, vector.y);
	mesh.scaleD = 0.5*max(mesh.scaleD, vector.z);
	mesh.bboxSize = vector;
	mesh.origin = 0.5*(vMax + vMin);//ƫ

	mesh.loadFromSMF(VertexList, FaceList, normals);
	return input;
}

void CMesh::calcAllEdgeLength(){
	for (UINT i = 0; i < m_nEdge; ++i) {
		UINT nodeA = m_pEdge[i].m_iVertex[0];
		UINT nodeB = m_pEdge[i].m_iVertex[1];
		double len = Vector3D(m_pVertex[nodeA].m_vPosition
			- m_pVertex[nodeB].m_vPosition).length();
		m_pEdge[i].m_length = len;
	}
}

bool CMesh::saveToSMF(const char* sFileName)
{
	return true;
}

bool CMesh::reConstruct()
{
	_VECTORLIST VertexList;//ʱ洢е
	_UINTLIST FaceList;//ʱ洢ĵϢ

	Vector3D vector;
	Vector3D vMin;
	Vector3D vMax;
	bool bFirst = true;

	UINT i;
	UINT* old2new = new UINT[m_nVertex];
	UINT iNewIndex = 0;
	for (i = 0; i<m_nVertex; i++)
	{
		VertexList.push_back(m_pVertex[i].m_vPosition);
		old2new[i] = iNewIndex++;
	}

	for (i = 0; i<m_nFace; i++)
		for (short j = 0; j<3; j++)
			FaceList.push_back(old2new[m_pFace[i].m_piVertex[j]]);

	clear();

	m_nVertex = (UINT)VertexList.size();
	m_nFace = (UINT)FaceList.size() / 3;
	m_nEdge = (UINT)3 * m_nFace;
	//read vertices and faces

	m_nVertexCapacity = m_nVertex * 2;
	m_nFaceCapacity = m_nFace * 2;

	m_pVertex = new CVertex[m_nVertexCapacity];
	if (m_pVertex == NULL) { clear(); return false; }//out of memory
	m_pFace = new CFace[m_nFaceCapacity];
	if (m_pFace == NULL) { clear(); return false; }//out of memory
	m_pAngles = new double[m_nVertexCapacity];
	if (!m_pAngles) { clear(); return false; }
	memset(m_pAngles, 0, sizeof(double) * m_nVertexCapacity);

	_VECTORLIST::iterator iVertex = VertexList.begin();
	_UINTLIST::iterator iFace = FaceList.begin();

	for (i = 0; i<m_nVertex; i++)
		m_pVertex[i].m_vPosition = *iVertex++;
	for (i = 0; i<m_nFace; i++)
	{
		m_pFace[i].Create(3);
		for (short j = 0; j<3; j++)
			m_pFace[i].m_piVertex[j] = *iFace++;
	}

	delete[] old2new;
	VertexList.clear();
	FaceList.clear();
	return construct();
}

bool CMesh::construct()
{
	if ((m_pVertex == NULL) || (m_pFace == NULL)) return false;//empty

	if (m_pEdge != NULL){ delete[] m_pEdge; m_pEdge = NULL; }//delete old edgelist

	m_bClosed = true;

	UINT i;
	if (m_nEdge == 0)//loadӦѾδɨ棬õܱ
		for (i = 0; i<m_nFace; i++)
			m_nEdge += m_pFace[i].m_nType;

	m_nEdgeCapacity = m_nEdge * 2;
	m_pEdge = new CEdge[m_nEdgeCapacity];

	short j, nType;

	UINT iEdge = 0;
	UINT iVertex;

	for (i = 0; i<m_nFace; i++)//ɨ
	{
		calFaceNormal(i);//˳ķ

		nType = m_pFace[i].m_nType;
		for (j = 0; j<nType; j++)//ÿ潨صĵ㡢ߡϢ
		{
			//Ϣ
			m_pFace[i].m_piEdge[j] = iEdge;//˳ÿ߷
			//Ϣ
			iVertex = m_pFace[i].m_piVertex[j];
			m_pVertex[iVertex].m_nValence++;//1
			m_pVertex[iVertex].m_lEdgeList.push_back(iEdge);//Ƚñ߼ʱ߱
			//ߵϢ
			m_pEdge[iEdge].m_iFace = i;
			m_pEdge[iEdge].m_iVertex[0] = iVertex;//Դ
			m_pEdge[iEdge].m_iVertex[1] = m_pFace[i].m_piVertex[(j + 1) % nType];//Ŀ
			m_pEdge[iEdge].m_iNextEdge = iEdge + 1;
			iEdge++;

		}
		m_pEdge[iEdge - 1].m_iNextEdge -= nType;

	}

	_UINTLIST::iterator iv;
	UINT iSrcVertex, iDesVertex;
	bool bFlag;
	for (i = 0; i<m_nEdge; i++)//ɨбߣбߵm_iTwinEdgeӹϵ
	{
		if (m_pEdge[i].m_iTwinEdge != -1)//Ѿ
			continue;
		iSrcVertex = m_pEdge[i].m_iVertex[0];//Դ
		iDesVertex = m_pEdge[i].m_iVertex[1];//Ŀ
		//Ŀ㷢бߣûлصԴı
		bFlag = true;
		for (iv = m_pVertex[iDesVertex].m_lEdgeList.begin(); iv != m_pVertex[iDesVertex].m_lEdgeList.end(); iv++)
		{
			iEdge = *iv;
			if (m_pEdge[iEdge].m_iVertex[1] == iSrcVertex)
			{
				m_pEdge[i].m_iTwinEdge = iEdge;
				m_pEdge[iEdge].m_iTwinEdge = i;
				bFlag = false;
				break;
			}
		}
		//ûУ˵ԴǱ߽
		if (bFlag)
		{
			m_pVertex[iSrcVertex].m_bIsBoundary = true;
			m_bClosed = false;
		}

	}
	short nValence;
	UINT iTwinEdge;

	//ʼWedgeĴСΪ2m_nVertexԱԺչ

	for (i = 0; i<m_nVertex; i++)//ɨе㣬ÿm_lEdgelist,˳ʱ뷽m_piEdge,
	{
		nValence = m_pVertex[i].m_nValence;
		if (nValence<2)//ĳĶС2
			m_pVertex[i].m_bIsBoundary = true;

		if (nValence == 0)//аĵ㣬Ӧýֹ
			continue;

		if (m_pVertex[i].m_piEdge != NULL)
			delete[] m_pVertex[i].m_piEdge;
		m_pVertex[i].m_piEdge = new UINT[nValence];


		//		if(m_pVertex[i].m_piEdge[0]==-1)//temp
		//			return false;

		if (m_pVertex[i].m_bIsBoundary)//߽
		{
			for (iv = m_pVertex[i].m_lEdgeList.begin(); iv != m_pVertex[i].m_lEdgeList.end(); iv++)
			{
				bFlag = true;
				m_pVertex[i].m_piEdge[0] = *iv;
				for (j = 1; j<nValence; j++)
				{
					iTwinEdge = m_pEdge[m_pVertex[i].m_piEdge[j - 1]].m_iTwinEdge;
					if (iTwinEdge == -1)//߽Edge
					{
						bFlag = false;
						break;
					}
					m_pVertex[i].m_piEdge[j] = m_pEdge[iTwinEdge].m_iNextEdge;
				}
				if (bFlag)//success
					break;
			}

		}
		else//ڲ
		{
			iv = m_pVertex[i].m_lEdgeList.begin();
			m_pVertex[i].m_piEdge[0] = *iv;
			for (j = 1; j<nValence; j++)//õ㷢half edge˳ʱm_piEdge
			{
				iTwinEdge = m_pEdge[m_pVertex[i].m_piEdge[j - 1]].m_iTwinEdge;
				//				if(iTwinEdge==-1)//temp
				//					return false;
				m_pVertex[i].m_piEdge[j] = m_pEdge[iTwinEdge].m_iNextEdge;
			}
		}
		m_pVertex[i].m_lEdgeList.clear();//ʱ
		for (j = 0; j<nValence; j++)
			m_pVertex[i].m_lEdgeList.push_back(m_pVertex[i].m_piEdge[j]);

		calVertexNormal(i);//ݵϢõƽ
		//		calVertexMeanCurvature(i);
	}

	calcAllEdgeLength();
	calcVertexAngle();

	m_nValidVertNum = m_nVertex;
	m_nValidFaceNum = m_nFace;
	m_nValidEdgeNum = m_nEdge;

	return true;
}

void CMesh::expandCapacity(){
	if ((m_nVertex + 8 >= m_nVertexCapacity) ||
		(m_nEdge + 8 >= m_nEdgeCapacity) ||
		(m_nFace + 8 >= m_nFaceCapacity)){

		m_nVertexCapacity *= 2;
		m_nEdgeCapacity *= 2;
		m_nFaceCapacity *= 2;

		CVertex* m_pVertexNew = new CVertex[m_nVertexCapacity];
		for (UINT i = 0; i < m_nVertex; ++i) {
			m_pVertexNew[i] = m_pVertex[i];
		}

		CEdge* m_pEdgeNew = new CEdge[m_nEdgeCapacity];
		for (UINT i = 0; i < m_nEdge; ++i) {
			m_pEdgeNew[i] = m_pEdge[i];
		}

		CFace* m_pFaceNew = new CFace[m_nFaceCapacity];
		for (UINT i = 0; i < m_nFace; ++i) {
			m_pFaceNew[i] = m_pFace[i];
		}

		double *m_pAnglesNew = new double[m_nVertexCapacity];
		for (unsigned i = 0; i < m_nVertex; ++i)
			m_pAnglesNew[i] = m_pAngles[i];

		if (m_pVertex != NULL) delete[] m_pVertex;
		if (m_pEdge != NULL) delete[] m_pEdge;
		if (m_pFace != NULL) delete[] m_pFace;
		if (m_pAngles) delete[] m_pAngles;
		m_pVertex = m_pVertexNew;
		m_pEdge = m_pEdgeNew;
		m_pFace = m_pFaceNew;
		m_pAngles = m_pAnglesNew;
	}
}

void CMesh::calcVertexAngle()
{
	for (unsigned i = 0; i < m_nFace; ++i)
	{
		for (unsigned j = 0; j < m_pFace[i].m_nType; ++j)
		{
			unsigned curVert = m_pFace[i].m_piVertex[j];
			double edge0 = m_pEdge[m_pFace[i].m_piEdge[j]].m_length;
			double edge1 = m_pEdge[m_pFace[i].m_piEdge[(j + m_pFace[i].m_nType - 1) % m_pFace[i].m_nType]].m_length;

			Vector3D vec0 = m_pVertex[m_pFace[i].m_piVertex[j]].m_vPosition;
			Vector3D vec1 = m_pVertex[m_pFace[i].m_piVertex[(j + 1) % m_pFace[i].m_nType]].m_vPosition;
			Vector3D vec2 = m_pVertex[m_pFace[i].m_piVertex[(j - 1 + m_pFace[i].m_nType) % m_pFace[i].m_nType]].m_vPosition;

			double edge2 = (vec1 - vec2).length();

			double curAngle = acos((edge0*edge0 + edge1*edge1 - edge2*edge2) / (2 * edge0*edge1));
			if (((vec0 - vec2) ^ (vec1 - vec0)) * m_pFace[i].m_vNormal < 0) curAngle = 2 * PI - curAngle;
			m_pFace[i].m_pdAngle[j] = curAngle;
			m_pAngles[curVert] += curAngle;
		}
	}
}

/******************************************************
O                         O
/ \                       /|\
/   \                     / | \
O-----O   =========>      O--O--O
\   /                     \ | /
\ /                       \|/
O                         O
*******************************************************/
UINT CMesh::split(UINT iEdge, double pos){	//split at the middle of the edge
	if (iEdge < 0 || iEdge >= m_nEdge) return -1;

	double originEdgeLen = m_pEdge[iEdge].m_length;
	UINT e2 = iEdge;
	UINT e6 = m_pEdge[e2].m_iTwinEdge;
	UINT e3 = m_pEdge[e2].m_iNextEdge;
	UINT e1 = m_pEdge[e3].m_iNextEdge;

	UINT f1 = m_pEdge[e2].m_iFace;

	UINT v2 = m_pEdge[e2].m_iVertex[0];
	UINT v4 = m_pEdge[e2].m_iVertex[1];
	UINT v1 = m_pEdge[e1].m_iVertex[0];

	Vector3D newVertPos = m_pVertex[v4].m_vPosition - m_pVertex[v2].m_vPosition;
	newVertPos.normalize();
	if (pos > 0.0) newVertPos = m_pVertex[v2].m_vPosition + newVertPos * pos;
	else newVertPos = (m_pVertex[v2].m_vPosition + m_pVertex[v4].m_vPosition) / 2.0;

	double edgeLen1 = (newVertPos - m_pVertex[v2].m_vPosition).length();
	double edgeLen2 = (newVertPos - m_pVertex[v4].m_vPosition).length();

	double newEdgeLen = (m_pVertex[v1].m_vPosition - newVertPos).length();

	if (e6 == -1) {
		UINT v5 = (m_nVertex++);
		UINT e8 = (m_nEdge++);
		UINT e9 = (m_nEdge++);
		UINT e10 = (m_nEdge++);
		UINT f3 = (m_nFace++);

		//Changes on v5
		m_pVertex[v5].m_vPosition = newVertPos;
		m_pVertex[v5].m_nValence = 2;
		m_pVertex[v5].m_piEdge = new UINT[2];
		m_pVertex[v5].m_piEdge[0] = e9;
		m_pVertex[v5].m_piEdge[1] = e8;
		m_pVertex[v5].m_bIsBoundary = true;
		m_pAngles[v5] = PI;

		//No changes on v2 and v4

		//Changes on v1
		m_pVertex[v1].m_nValence++;
		unsigned newEdgeListIndex = 0;
		UINT* v1piEdge = new UINT[m_pVertex[v1].m_nValence];
		for (short i = 0; i < m_pVertex[v1].m_nValence - 1; ++i) {
			v1piEdge[newEdgeListIndex] = m_pVertex[v1].m_piEdge[i];
			if (m_pEdge[m_pVertex[v1].m_piEdge[(i + 1) % (m_pVertex[v1].m_nValence - 1)]].m_iFace == f1) {
				++newEdgeListIndex; v1piEdge[newEdgeListIndex] = e10;
			}
			++newEdgeListIndex;
		}
		if (m_pVertex[v1].m_bIsBoundary && newEdgeListIndex == m_pVertex[v1].m_nValence - 1)
		{
			for (unsigned i = 0; i < m_pVertex[v1].m_nValence; ++i)
				v1piEdge[m_pVertex[v1].m_nValence - i] = v1piEdge[m_pVertex[v1].m_nValence - i - 1];
			v1piEdge[0] = e10;
		}
		delete[] m_pVertex[v1].m_piEdge;
		m_pVertex[v1].m_piEdge = v1piEdge;

		//No changes on e1

		//Changes on e2
		m_pEdge[e2].m_iNextEdge = e9;
		m_pEdge[e2].m_iVertex[1] = v5;
		m_pEdge[e2].m_length = edgeLen1;

		//Changes on e3
		m_pEdge[e3].m_iFace = f3;
		m_pEdge[e3].m_iNextEdge = e10;

		//Changes on e8
		m_pEdge[e8].m_iFace = f3;
		m_pEdge[e8].m_iNextEdge = e3;
		m_pEdge[e8].m_iTwinEdge = -1;
		m_pEdge[e8].m_iVertex[0] = v5;
		m_pEdge[e8].m_iVertex[1] = v4;
		m_pEdge[e8].m_length = edgeLen2;

		//Changes on e9
		m_pEdge[e9].m_iFace = f1;
		m_pEdge[e9].m_iNextEdge = e1;
		m_pEdge[e9].m_iTwinEdge = e10;
		m_pEdge[e9].m_iVertex[0] = v5;
		m_pEdge[e9].m_iVertex[1] = v1;
		m_pEdge[e9].m_length = newEdgeLen;

		//Changes on e10
		m_pEdge[e10].m_iFace = f3;
		m_pEdge[e10].m_iNextEdge = e8;
		m_pEdge[e10].m_iTwinEdge = e9;
		m_pEdge[e10].m_iVertex[0] = v1;
		m_pEdge[e10].m_iVertex[1] = v5;
		m_pEdge[e10].m_length = newEdgeLen;

		//Changes on f1
		for (int i = 0; i < 3; ++i) {
			if (m_pFace[f1].m_piEdge[i] == e3) {
				m_pFace[f1].m_piEdge[i] = e9;
			}
			if (m_pFace[f1].m_piVertex[i] == v4) {
				m_pFace[f1].m_piVertex[i] = v5;
			}
		}
		calFaceNormal(f1);

		//Changes on f3
		m_pFace[f3].Create(3);
		m_pFace[f3].m_piEdge[0] = e10;
		m_pFace[f3].m_piEdge[1] = e8;
		m_pFace[f3].m_piEdge[2] = e3;
		m_pFace[f3].m_piVertex[0] = v1;
		m_pFace[f3].m_piVertex[1] = v5;
		m_pFace[f3].m_piVertex[2] = v4;
		m_pFace[f3].m_color = m_pFace[f1].m_color;
		calFaceNormal(f3);

		calVertexNormal(v5);
		expandCapacity();

		return v5;
	}
	else {
		UINT f2 = m_pEdge[e6].m_iFace;
		UINT e4 = m_pEdge[e6].m_iNextEdge;
		UINT e5 = m_pEdge[e4].m_iNextEdge;
		UINT v3 = m_pEdge[e4].m_iVertex[1];
		double newEdgeLen2 =
			(m_pVertex[v3].m_vPosition - newVertPos).length();

		UINT e7 = (m_nEdge++);
		UINT e8 = (m_nEdge++);
		UINT e9 = (m_nEdge++);
		UINT e10 = (m_nEdge++);
		UINT e11 = (m_nEdge++);
		UINT e12 = (m_nEdge++);
		UINT v5 = (m_nVertex++);
		UINT f3 = (m_nFace++);
		UINT f4 = (m_nFace++);

		//Changes on v5
		m_pVertex[v5].m_vPosition = newVertPos;
		m_pVertex[v5].m_nValence = 4;
		m_pVertex[v5].m_piEdge = new UINT[4];
		m_pVertex[v5].m_piEdge[0] = e9;
		m_pVertex[v5].m_piEdge[1] = e8;
		m_pVertex[v5].m_piEdge[2] = e12;
		m_pVertex[v5].m_piEdge[3] = e7;
		m_pVertex[v5].m_bIsBoundary = false;
		m_pAngles[v5] = 2 * PI;

		//No Changes on v2 and v4

		//Changes on v1
		m_pVertex[v1].m_nValence++;
		unsigned newEdgeListIndex = 0;
		UINT* v1piEdge = new UINT[m_pVertex[v1].m_nValence];
		for (short i = 0; i < m_pVertex[v1].m_nValence - 1; ++i) {
			v1piEdge[newEdgeListIndex] = m_pVertex[v1].m_piEdge[i];
			if (m_pEdge[m_pVertex[v1].m_piEdge[(i + 1) % (m_pVertex[v1].m_nValence - 1)]].m_iFace == f1) {
				++newEdgeListIndex; v1piEdge[newEdgeListIndex] = e10;
			}
			++newEdgeListIndex;
		}
		if (m_pVertex[v1].m_bIsBoundary && newEdgeListIndex == m_pVertex[v1].m_nValence - 1)
		{
			for (unsigned i = 0; i < m_pVertex[v1].m_nValence; ++i)
				v1piEdge[m_pVertex[v1].m_nValence - i] = v1piEdge[m_pVertex[v1].m_nValence - i - 1];
			v1piEdge[0] = e10;
		}
		delete[] m_pVertex[v1].m_piEdge;
		m_pVertex[v1].m_piEdge = v1piEdge;

		//Changes on v3
		m_pVertex[v3].m_nValence++;
		newEdgeListIndex = 0;
		UINT* v3piEdge = new UINT[m_pVertex[v3].m_nValence];
		for (short i = 0; i < m_pVertex[v3].m_nValence - 1; ++i) {
			v3piEdge[newEdgeListIndex] = m_pVertex[v3].m_piEdge[i];
			if (m_pEdge[m_pVertex[v3].m_piEdge[(i + 1) % (m_pVertex[v3].m_nValence - 1)]].m_iFace == f2) {
				++newEdgeListIndex; v3piEdge[newEdgeListIndex] = e11;
			}
			++newEdgeListIndex;
		}
		if (m_pVertex[v3].m_bIsBoundary && newEdgeListIndex == m_pVertex[v3].m_nValence - 1)
		{
			for (unsigned i = 0; i < m_pVertex[v3].m_nValence; ++i)
				v3piEdge[m_pVertex[v3].m_nValence - i] = v1piEdge[m_pVertex[v3].m_nValence - i - 1];
			v3piEdge[0] = e11;
		}
		delete[] m_pVertex[v3].m_piEdge;
		m_pVertex[v3].m_piEdge = v3piEdge;

		//No Changes on e1 and e5

		//Changes on e2
		m_pEdge[e2].m_iNextEdge = e9;
		m_pEdge[e2].m_iTwinEdge = e7;
		m_pEdge[e2].m_iVertex[1] = v5;
		m_pEdge[e2].m_length = edgeLen1;

		//Changes on e3
		m_pEdge[e3].m_iFace = f3;
		m_pEdge[e3].m_iNextEdge = e10;

		//Changes on e4
		m_pEdge[e4].m_iFace = f4;
		m_pEdge[e4].m_iNextEdge = e11;

		//Changes on e6
		m_pEdge[e6].m_iNextEdge = e12;
		m_pEdge[e6].m_iTwinEdge = e8;
		m_pEdge[e6].m_iVertex[1] = v5;
		m_pEdge[e6].m_length = edgeLen2;

		//Changes on e7
		m_pEdge[e7].m_iFace = f4;
		m_pEdge[e7].m_iNextEdge = e4;
		m_pEdge[e7].m_iTwinEdge = e2;
		m_pEdge[e7].m_iVertex[0] = v5;
		m_pEdge[e7].m_iVertex[1] = v2;
		m_pEdge[e7].m_length = edgeLen1;

		//Changes on e8
		m_pEdge[e8].m_iFace = f3;
		m_pEdge[e8].m_iNextEdge = e3;
		m_pEdge[e8].m_iTwinEdge = e6;
		m_pEdge[e8].m_iVertex[0] = v5;
		m_pEdge[e8].m_iVertex[1] = v4;
		m_pEdge[e8].m_length = edgeLen2;

		//Changes on e9
		m_pEdge[e9].m_iFace = f1;
		m_pEdge[e9].m_iNextEdge = e1;
		m_pEdge[e9].m_iTwinEdge = e10;
		m_pEdge[e9].m_iVertex[0] = v5;
		m_pEdge[e9].m_iVertex[1] = v1;
		m_pEdge[e9].m_length = newEdgeLen;

		//Changes on e10
		m_pEdge[e10].m_iFace = f3;
		m_pEdge[e10].m_iNextEdge = e8;
		m_pEdge[e10].m_iTwinEdge = e9;
		m_pEdge[e10].m_iVertex[0] = v1;
		m_pEdge[e10].m_iVertex[1] = v5;
		m_pEdge[e10].m_length = newEdgeLen;

		//Changes on e11
		m_pEdge[e11].m_iFace = f4;
		m_pEdge[e11].m_iNextEdge = e7;
		m_pEdge[e11].m_iTwinEdge = e12;
		m_pEdge[e11].m_iVertex[0] = v3;
		m_pEdge[e11].m_iVertex[1] = v5;
		m_pEdge[e11].m_length = newEdgeLen2;

		//Changes on e12
		m_pEdge[e12].m_iFace = f2;
		m_pEdge[e12].m_iNextEdge = e5;
		m_pEdge[e12].m_iTwinEdge = e11;
		m_pEdge[e12].m_iVertex[0] = v5;
		m_pEdge[e12].m_iVertex[1] = v3;
		m_pEdge[e12].m_length = newEdgeLen2;

		//Changes on f1
		for (int i = 0; i < 3; ++i) {
			if (m_pFace[f1].m_piEdge[i] == e3) {
				m_pFace[f1].m_piEdge[i] = e9;
			}
			if (m_pFace[f1].m_piVertex[i] == v4) {
				m_pFace[f1].m_piVertex[i] = v5;
			}
		}
		calFaceNormal(f1);

		//Changes on f2
		for (int i = 0; i < 3; ++i) {
			if (m_pFace[f2].m_piEdge[i] == e4) {
				m_pFace[f2].m_piEdge[i] = e12;
			}
			if (m_pFace[f2].m_piVertex[i] == v2) {
				m_pFace[f2].m_piVertex[i] = v5;
			}
		}
		calFaceNormal(f2);

		//Changes on f3
		m_pFace[f3].Create(3);
		m_pFace[f3].m_piEdge[0] = e10;
		m_pFace[f3].m_piEdge[1] = e8;
		m_pFace[f3].m_piEdge[2] = e3;
		m_pFace[f3].m_piVertex[0] = v1;
		m_pFace[f3].m_piVertex[1] = v5;
		m_pFace[f3].m_piVertex[2] = v4;
		m_pFace[f3].m_color = m_pFace[f1].m_color;
		calFaceNormal(f3);

		//Changes on f4
		m_pFace[f4].Create(3);
		m_pFace[f4].m_piEdge[0] = e11;
		m_pFace[f4].m_piEdge[1] = e7;
		m_pFace[f4].m_piEdge[2] = e4;
		m_pFace[f4].m_piVertex[0] = v3;
		m_pFace[f4].m_piVertex[1] = v5;
		m_pFace[f4].m_piVertex[2] = v2;
		m_pFace[f4].m_color = m_pFace[f2].m_color;
		calFaceNormal(f4);

		calVertexNormal(v5);
		expandCapacity();
		return v5;
	}
	return -1;
}

void CMesh::flip(unsigned iEdge)
{
	unsigned twinEdge = m_pEdge[iEdge].m_iTwinEdge;
	if (twinEdge == -1) return;

	unsigned e0 = m_pEdge[iEdge].m_iNextEdge;
	unsigned e1 = m_pEdge[e0].m_iNextEdge;
	unsigned e2 = m_pEdge[twinEdge].m_iNextEdge;
	unsigned e3 = m_pEdge[e2].m_iNextEdge;

	unsigned v0 = m_pEdge[m_pEdge[iEdge].m_iNextEdge].m_iVertex[1];
	unsigned v1 = m_pEdge[m_pEdge[twinEdge].m_iNextEdge].m_iVertex[1];

	unsigned v2 = m_pEdge[iEdge].m_iVertex[0];
	unsigned v3 = m_pEdge[iEdge].m_iVertex[1];

	unsigned f0 = m_pEdge[iEdge].m_iFace;
	unsigned f1 = m_pEdge[twinEdge].m_iFace;

	// flip edge
	m_pEdge[iEdge].m_iVertex[0] = v1; m_pEdge[iEdge].m_iVertex[1] = v0;
	m_pEdge[iEdge].m_iNextEdge = e1; m_pEdge[e1].m_iNextEdge = e2; m_pEdge[e2].m_iNextEdge = iEdge;
	m_pEdge[iEdge].m_length = (m_pVertex[v1].m_vPosition - m_pVertex[v0].m_vPosition).length();
	m_pEdge[e2].m_iFace = f0;

	m_pEdge[twinEdge].m_iVertex[0] = v0; m_pEdge[twinEdge].m_iVertex[1] = v1;
	m_pEdge[twinEdge].m_iNextEdge = e3; m_pEdge[e3].m_iNextEdge = e0; m_pEdge[e0].m_iNextEdge = twinEdge;
	m_pEdge[twinEdge].m_length = (m_pVertex[v0].m_vPosition - m_pVertex[v1].m_vPosition).length();
	m_pEdge[e0].m_iFace = f1;

	// reset face
	unsigned topI = 0;
	for (; topI < 3; ++topI) if (m_pFace[f0].m_piVertex[topI] == v0) break;
	assert(topI < 3);
	m_pFace[f0].m_piVertex[topI] = v2; m_pFace[f0].m_piVertex[(topI + 1) % 3] = v1; m_pFace[f0].m_piVertex[(topI + 2) % 3] = v0;
	m_pFace[f0].m_piEdge[topI] = e2; m_pFace[f0].m_piEdge[(topI + 1) % 3] = iEdge; m_pFace[f0].m_piEdge[(topI + 2) % 3] = e1;

	topI = 0;
	for (; topI < 3; ++topI) if (m_pFace[f1].m_piVertex[topI] == v1) break;
	assert(topI < 3);
	m_pFace[f1].m_piVertex[topI] = v3; m_pFace[f1].m_piVertex[(topI + 1) % 3] = v0; m_pFace[f1].m_piVertex[(topI + 2) % 3] = v1;
	m_pFace[f1].m_piEdge[topI] = e0; m_pFace[f1].m_piEdge[(topI + 1) % 3] = twinEdge; m_pFace[f1].m_piEdge[(topI + 2) % 3] = e3;

	// expand v0, v1
	unsigned *v0mpiEdge = new unsigned[m_pVertex[v0].m_nValence + 1];
	unsigned eIndex = 0;
	for (unsigned i = 0; i < m_pVertex[v0].m_nValence; ++i)
	{
		v0mpiEdge[eIndex++] = m_pVertex[v0].m_piEdge[i];
		unsigned nextEdge = m_pVertex[v0].m_piEdge[(i + 1) % m_pVertex[v0].m_nValence];
		if (nextEdge != e1) continue;
		v0mpiEdge[eIndex++] = twinEdge;
	}
	assert(eIndex == m_pVertex[v0].m_nValence + 1 || m_pVertex[v0].m_bIsBoundary && eIndex == m_pVertex[v0].m_nValence);
	// if e1 is v0's first edge
	if (m_pVertex[v0].m_bIsBoundary && eIndex == m_pVertex[v0].m_nValence)
	{
		for (unsigned i = 0; i < m_pVertex[v0].m_nValence; ++i)
			v0mpiEdge[m_pVertex[v0].m_nValence - i] = v0mpiEdge[m_pVertex[v0].m_nValence - i - 1];
		v0mpiEdge[0] = twinEdge;
	}
	delete[] m_pVertex[v0].m_piEdge;
	m_pVertex[v0].m_piEdge = v0mpiEdge;
	++m_pVertex[v0].m_nValence;

	unsigned *v1mpiEdge = new unsigned[m_pVertex[v1].m_nValence + 1];
	eIndex = 0;
	for (unsigned i = 0; i < m_pVertex[v1].m_nValence; ++i)
	{
		v1mpiEdge[eIndex++] = m_pVertex[v1].m_piEdge[i];
		unsigned nextEdge = m_pVertex[v1].m_piEdge[(i + 1) % m_pVertex[v1].m_nValence];
		if (nextEdge != e3) continue;
		v1mpiEdge[eIndex++] = iEdge;
	}
	assert(eIndex == m_pVertex[v1].m_nValence + 1 || m_pVertex[v1].m_bIsBoundary && eIndex == m_pVertex[v1].m_nValence);
	// if e3 is v1's first edge
	if (m_pVertex[v1].m_bIsBoundary && eIndex == m_pVertex[v1].m_nValence)
	{
		for (unsigned i = 0; i < m_pVertex[v1].m_nValence; ++i)
			v0mpiEdge[m_pVertex[v1].m_nValence - i] = v0mpiEdge[m_pVertex[v1].m_nValence - i - 1];
		v1mpiEdge[0] = iEdge;
	}
	delete[] m_pVertex[v1].m_piEdge;
	m_pVertex[v1].m_piEdge = v1mpiEdge;
	++m_pVertex[v1].m_nValence;

	// shrink v2, v3
	unsigned *v2mpiEdge = new unsigned[m_pVertex[v2].m_nValence - 1];
	eIndex = 0;
	for (unsigned i = 0; i < m_pVertex[v2].m_nValence; ++i)
	{
		if (m_pVertex[v2].m_piEdge[i] == iEdge) continue;
		v2mpiEdge[eIndex++] = m_pVertex[v2].m_piEdge[i];
	}
	assert(eIndex == m_pVertex[v2].m_nValence - 1);
	delete[] m_pVertex[v2].m_piEdge;
	m_pVertex[v2].m_piEdge = v2mpiEdge;
	--m_pVertex[v2].m_nValence;

	unsigned *v3mpiEdge = new unsigned[m_pVertex[v3].m_nValence - 1];
	eIndex = 0;
	for (unsigned i = 0; i < m_pVertex[v3].m_nValence; ++i)
	{
		if (m_pVertex[v3].m_piEdge[i] == twinEdge) continue;
		v3mpiEdge[eIndex++] = m_pVertex[v3].m_piEdge[i];
	}
	assert(eIndex == m_pVertex[v3].m_nValence - 1);
	delete[] m_pVertex[v3].m_piEdge;
	m_pVertex[v3].m_piEdge = v3mpiEdge;
	--m_pVertex[v3].m_nValence;

	// update face areas
	calFaceNormal(f0);
	calFaceNormal(f1);
}

void CMesh::collapse(unsigned iEdge, Vector3D newPos)
{
	unsigned v0 = m_pEdge[iEdge].m_iVertex[0];
	unsigned v1 = m_pEdge[iEdge].m_iVertex[1];
	unsigned v2 = m_pEdge[m_pEdge[iEdge].m_iNextEdge].m_iVertex[1];
	unsigned v3 = -1;

	// update v0's position
	// m_pVertex[v0].m_vPosition = (m_pVertex[v0].m_vPosition + m_pVertex[v1].m_vPosition) / 2.0;
	m_pVertex[v0].m_vPosition = newPos;

	unsigned twinEdge = m_pEdge[iEdge].m_iTwinEdge;
	if (twinEdge != -1) v3 = m_pEdge[m_pEdge[twinEdge].m_iNextEdge].m_iVertex[1];

	unsigned connectFace0 = m_pEdge[iEdge].m_iFace;
	unsigned connectFace1 = (twinEdge == -1 ? -1 : m_pEdge[twinEdge].m_iFace);

	list<unsigned> v0AdjFaces, v1AdjFaces;

	for (unsigned i = 0; i < m_pVertex[v0].m_nValence; ++i)
		v0AdjFaces.push_back(m_pEdge[m_pVertex[v0].m_piEdge[i]].m_iFace);

	for (unsigned i = 0; i < m_pVertex[v1].m_nValence; ++i)
		v1AdjFaces.push_back(m_pEdge[m_pVertex[v1].m_piEdge[i]].m_iFace);

	if (connectFace1 != -1)
	{
		list<unsigned>::iterator f0I, f0J, f1I, f1J;
		for (f0I = v0AdjFaces.begin(); f0I != v0AdjFaces.end(); ++f0I)
		{
			f0J = f0I; ++f0J; if (f0J == v0AdjFaces.end()) f0J = v0AdjFaces.begin();
			if (*f0I == connectFace0 && *f0J == connectFace1) break;
		}
		for (f1I = v1AdjFaces.begin(); f1I != v1AdjFaces.end(); ++f1I)
		{
			f1J = f1I; ++f1J; if (f1J == v1AdjFaces.end()) f1J = v1AdjFaces.begin();
			if (*f1I == connectFace1 && *f1J == connectFace0) break;
		}

		assert(*f0I == connectFace0 && *f0J == connectFace1);
		assert(*f1I == connectFace1 && *f1J == connectFace0);

		// insert from 1 to 0 one by one
		++f1J;
		if (f1J == v1AdjFaces.end()) f1J = v1AdjFaces.begin();

		while (f1J != f1I)
		{
			for (unsigned i = 0; i < 3; ++i)
			{
				if (m_pFace[*f1J].m_piVertex[i] == v1) m_pFace[*f1J].m_piVertex[i] = v0;
				unsigned curEdge = m_pFace[*f1J].m_piEdge[i];
				if (m_pEdge[curEdge].m_iVertex[0] == v1)
				{
					m_pEdge[curEdge].m_iVertex[0] = v0;
					// 					m_pEdge[curEdge].m_length = 
					// 						(m_pVertex[m_pEdge[curEdge].m_iVertex[0]].m_vPosition - m_pVertex[m_pEdge[curEdge].m_iVertex[1]].m_vPosition).length();
				}
				if (m_pEdge[curEdge].m_iVertex[1] == v1)
				{
					m_pEdge[curEdge].m_iVertex[1] = v0;
					// 					m_pEdge[curEdge].m_length = 
					// 						(m_pVertex[m_pEdge[curEdge].m_iVertex[0]].m_vPosition - m_pVertex[m_pEdge[curEdge].m_iVertex[1]].m_vPosition).length();
				}
			}
			v0AdjFaces.insert(f0I, *f1J);
			++f1J;
			if (f1J == v1AdjFaces.end()) f1J = v1AdjFaces.begin();
		}

		v0AdjFaces.erase(f0I); v0AdjFaces.erase(f0J);
	}
	else
	{
		list<unsigned>::iterator f0I, f1I;
		f0I = v0AdjFaces.end(); --f0I;
		f1I = v1AdjFaces.begin();
		// 		for (f0I = v0AdjFaces.begin(); f0I != v0AdjFaces.end(); ++f0I)
		// 			if (*f0I == connectFace0) break;
		// 		for (f1I = v1AdjFaces.begin(); f1I != v1AdjFaces.end(); ++f1I)
		// 			if (*f1I == connectFace0) break;

		assert(*f0I == connectFace0);
		assert(*f1I == connectFace0);

		// insert from 1 to 0 one by one
		++f1I;
		while (f1I != v1AdjFaces.end())
		{
			for (unsigned i = 0; i < 3; ++i)
			{
				if (m_pFace[*f1I].m_piVertex[i] == v1) m_pFace[*f1I].m_piVertex[i] = v0;
				unsigned curEdge = m_pFace[*f1I].m_piEdge[i];
				if (m_pEdge[curEdge].m_iVertex[0] == v1)
				{
					m_pEdge[curEdge].m_iVertex[0] = v0;
					// 					m_pEdge[curEdge].m_length = 
					// 						(m_pVertex[m_pEdge[curEdge].m_iVertex[0]].m_vPosition - m_pVertex[m_pEdge[curEdge].m_iVertex[1]].m_vPosition).length();
				}
				if (m_pEdge[curEdge].m_iVertex[1] == v1)
				{
					m_pEdge[curEdge].m_iVertex[1] = v0;
					// 					m_pEdge[curEdge].m_length = 
					// 						(m_pVertex[m_pEdge[curEdge].m_iVertex[0]].m_vPosition - m_pVertex[m_pEdge[curEdge].m_iVertex[1]].m_vPosition).length();
				}
			}
			v0AdjFaces.insert(f0I, *f1I);
			++f1I;
		}
		v0AdjFaces.erase(f0I);
	}

	// update twin edges;
	unsigned e0 = iEdge;
	unsigned e1 = m_pEdge[e0].m_iNextEdge;
	unsigned e2 = m_pEdge[e1].m_iNextEdge;

	if (m_pEdge[e2].m_iTwinEdge != -1)
		m_pEdge[m_pEdge[e2].m_iTwinEdge].m_iTwinEdge = m_pEdge[e1].m_iTwinEdge;
	if (m_pEdge[e1].m_iTwinEdge != -1)
		m_pEdge[m_pEdge[e1].m_iTwinEdge].m_iTwinEdge = m_pEdge[e2].m_iTwinEdge;

	if (twinEdge != -1)
	{
		e0 = twinEdge;
		e1 = m_pEdge[e0].m_iNextEdge;
		e2 = m_pEdge[e1].m_iNextEdge;

		if (m_pEdge[e2].m_iTwinEdge != -1)
			m_pEdge[m_pEdge[e2].m_iTwinEdge].m_iTwinEdge = m_pEdge[e1].m_iTwinEdge;
		if (m_pEdge[e1].m_iTwinEdge != -1)
			m_pEdge[m_pEdge[e1].m_iTwinEdge].m_iTwinEdge = m_pEdge[e2].m_iTwinEdge;
	}

	// adjust order
	// is this necessary?
	if (m_pVertex[v0].m_bIsBoundary || m_pVertex[v1].m_bIsBoundary)
	{
		unsigned curStartFace = *v0AdjFaces.begin();
		unsigned boundaryEdge = -1;
		for (unsigned i = 0; i < 3; ++i)
		{
			if (m_pEdge[m_pFace[curStartFace].m_piEdge[i]].m_iVertex[1] == v0 &&
				m_pEdge[m_pFace[curStartFace].m_piEdge[i]].m_iTwinEdge == -1)
			{
				boundaryEdge = m_pFace[curStartFace].m_piEdge[i];
				break;
			}
		}

		while (boundaryEdge == -1)
		{
			v0AdjFaces.push_front(v0AdjFaces.back());
			v0AdjFaces.pop_back();

			curStartFace = *v0AdjFaces.begin();
			boundaryEdge = -1;
			for (unsigned i = 0; i < 3; ++i)
			{
				if (m_pEdge[m_pFace[curStartFace].m_piEdge[i]].m_iVertex[1] == v0 &&
					m_pEdge[m_pFace[curStartFace].m_piEdge[i]].m_iTwinEdge == -1)
				{
					boundaryEdge = m_pFace[curStartFace].m_piEdge[i];
					break;
				}
			}
		}
	}

	// update vertex adjacent edges
	delete[] m_pVertex[v0].m_piEdge;
	delete[] m_pVertex[v1].m_piEdge;
	m_pVertex[v0].m_nValence = 0;
	m_pVertex[v0].m_piEdge = new UINT[v0AdjFaces.size()];
	m_pVertex[v1].m_nValence = 0;
	m_pVertex[v1].m_piEdge = NULL;
	if (m_pVertex[v1].m_bIsBoundary) m_pVertex[v0].m_bIsBoundary = true;

	for (list<unsigned>::iterator iter = v0AdjFaces.begin(); iter != v0AdjFaces.end(); ++iter)
	{
		for (unsigned j = 0; j < 3; ++j)
		{
			if (m_pEdge[m_pFace[*iter].m_piEdge[j]].m_iVertex[0] == v0)
			{
				m_pVertex[v0].m_piEdge[m_pVertex[v0].m_nValence++] = m_pFace[*iter].m_piEdge[j];
				break;
			}
		}

		for (unsigned j = 0; j < 3; ++j)
		{
			unsigned curEdge = m_pFace[*iter].m_piEdge[j];
			if (m_pEdge[curEdge].m_iVertex[0] == v0 || m_pEdge[curEdge].m_iVertex[1] == v0)
			{
				m_pEdge[curEdge].m_length = (m_pVertex[m_pEdge[curEdge].m_iVertex[0]].m_vPosition -
					m_pVertex[m_pEdge[curEdge].m_iVertex[1]].m_vPosition).length();
			}
		}

		calFaceNormal(*iter);
	}

	if (m_pVertex[v2].m_nValence > 1)
	{
		unsigned *v2piEdge = new UINT[m_pVertex[v2].m_nValence - 1];
		unsigned eIdx = 0;
		for (unsigned i = 0; i < m_pVertex[v2].m_nValence; ++i)
			if (m_pEdge[m_pVertex[v2].m_piEdge[i]].m_iFace != connectFace0)
				v2piEdge[eIdx++] = m_pVertex[v2].m_piEdge[i];
		assert(eIdx == m_pVertex[v2].m_nValence - 1);
		--m_pVertex[v2].m_nValence;
		delete[] m_pVertex[v2].m_piEdge;
		m_pVertex[v2].m_piEdge = v2piEdge;
	}
	else
	{
		m_pVertex[v2].m_bValid = false;
	}
	if (v3 != -1 && m_pVertex[v3].m_nValence > 1)
	{
		unsigned *v3piEdge = new UINT[m_pVertex[v3].m_nValence - 1];
		unsigned eIdx = 0;
		for (unsigned i = 0; i < m_pVertex[v3].m_nValence; ++i)
			if (m_pEdge[m_pVertex[v3].m_piEdge[i]].m_iFace != connectFace1)
				v3piEdge[eIdx++] = m_pVertex[v3].m_piEdge[i];
		assert(eIdx == m_pVertex[v3].m_nValence - 1);
		--m_pVertex[v3].m_nValence;
		delete[] m_pVertex[v3].m_piEdge;
		m_pVertex[v3].m_piEdge = v3piEdge;
	}
	else if (v3 != -1)
	{
		m_pVertex[v3].m_bValid = false;
	}

	// mark v1, commonFace0, commonFace1 and some edges unavailable
	m_pEdge[iEdge].m_bValid = false;
	if (twinEdge != -1) m_pEdge[twinEdge].m_bValid = false;
	m_pEdge[m_pEdge[iEdge].m_iNextEdge].m_bValid = false;
	m_pEdge[m_pEdge[m_pEdge[iEdge].m_iNextEdge].m_iNextEdge].m_bValid = false;
	if (twinEdge != -1) m_pEdge[m_pEdge[twinEdge].m_iNextEdge].m_bValid = false;
	if (twinEdge != -1) m_pEdge[m_pEdge[m_pEdge[twinEdge].m_iNextEdge].m_iNextEdge].m_bValid = false;

	m_pVertex[v1].m_bValid = false;
	m_pFace[connectFace0].m_bValid = false;
	if (connectFace1 != -1) m_pFace[connectFace1].m_bValid = false;
	if (m_pVertex[v1].m_bIsBoundary) m_pVertex[v0].m_bIsBoundary = true;

	// update actual vert, face, edge number
	--m_nValidVertNum;
	if (connectFace1 != -1) m_nValidFaceNum -= 2;
	else --m_nValidFaceNum;
	if (twinEdge != -1) m_nValidEdgeNum -= 6;
	else m_nValidEdgeNum -= 3;
}

void CMesh::calFaceNormal(UINT i)
{
	Vector3D v[2];

	//get the vector
	v[0] = m_pVertex[m_pFace[i].m_piVertex[2]].m_vPosition - m_pVertex[m_pFace[i].m_piVertex[0]].m_vPosition;

	if (m_pFace[i].m_nType == 3)
		v[1] = m_pVertex[m_pFace[i].m_piVertex[2]].m_vPosition - m_pVertex[m_pFace[i].m_piVertex[1]].m_vPosition;
	else
		v[1] = m_pVertex[m_pFace[i].m_piVertex[3]].m_vPosition - m_pVertex[m_pFace[i].m_piVertex[1]].m_vPosition;
	m_pFace[i].m_vNormal = v[0] ^ v[1];

	if (m_pFace[i].m_nType == 3)
		m_pFace[i].m_dArea = 0.5*m_pFace[i].m_vNormal.normalize();//λͬʱ˸
	else if (m_pFace[i].m_nType == 4)
		m_pFace[i].m_dArea = m_pFace[i].m_vNormal.normalize();//λͬʱ˸ı
	else
		m_pFace[i].m_vNormal.normalize();

	Vector3D vMassPoint;

	for (short j = 0; j < m_pFace[i].m_nType; j++)
		vMassPoint += m_pVertex[m_pFace[i].m_piVertex[j]].m_vPosition;
	vMassPoint /= (double)(m_pFace[i].m_nType);
	m_pFace[i].m_vMassPoint = vMassPoint;

}

void CMesh::calVertexNormal(UINT i)
{
	Vector3D v;
	UINT iFace;
	short valence = m_pVertex[i].m_nValence;
	if (valence<1)
		return;
	for (short j = 0; j<valence; j++)
	{
		iFace = m_pEdge[m_pVertex[i].m_piEdge[j]].m_iFace;
		v += m_pFace[iFace].m_vNormal;
	}
	v /= (double)valence;
	v.normalize();
	m_pVertex[i].m_vNormal = v;

}

