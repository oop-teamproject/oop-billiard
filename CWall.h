// -----------------------------------------------------------------------------
// CWall class definition
// -----------------------------------------------------------------------------
#ifndef __CWALL_H__
#define __CWALL_H__

#include "d3dUtility.h"
#include "CSphere.h"

class CWall {
private:
	float					m_x, m_y, m_z;
	float                   m_width, m_height, m_depth;
public:
	CWall(void);
	~CWall(void);
public:
	bool create(IDirect3DDevice9* pDevice, float ix, float iz, float iwidth, float iheight, float idepth, D3DXCOLOR color);
	void destroy(void);
	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld);
private:
	float square(float X) { return X * X; }
public:
	bool hasIntersected(CSphere& ball) const;
	D3DXVECTOR3 closestPoint(CSphere& ball) const;
	void hitBy(CSphere& ball);
	void setPosition(float x, float y, float z);
	float getHeight(void) const { return this->m_height; }
	float getWidth(void)  const { return this->m_width; }
	float getDepth(void)  const { return this->m_depth; }
	D3DXVECTOR3 getCenter(void) const;
private:
	void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }

	D3DXMATRIX              m_mLocal;
	D3DMATERIAL9            m_mtrl;
	ID3DXMesh* m_pBoundMesh;
};

#endif