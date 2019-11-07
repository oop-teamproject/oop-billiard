#ifndef __CSTICK_H__
#define __CSTICK_H__

#include "d3dUtility.h"
#include "CSphere.h"
#include <cmath>
#define PI 3.14159265
class CStick {
private:
	float                   v_y; //당구채가 가리키는 방향. +x방향이 0, 위에서 볼 때 시계방향으로 회전한다.
	float					m_x, m_y, m_z; //당구채가 가리키는 중심.
	float					distance;	   //(당구채 머리쪽 끝이 현재 있는 위치)와 m 사이의 거리
public:
	CStick(void);
	~CStick(void);
public:
	bool create(IDirect3DDevice9* pDevice, D3DXCOLOR color);
	void destroy(void);
	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld);
	void setPosition(float x, float y, float z);
	void setDirection(float y);
	void viewAt(float x, float z);

	inline float getDistance() const { return distance; }
	inline float getX() const { return m_x; }
	inline float getY() const { return m_y; }
	inline float getZ() const { return m_z; }
	inline D3DXVECTOR3 getCenter(void) const { return D3DXVECTOR3(m_x, m_y, m_z); }
	inline float getDirection(void) const { return v_y; }
private:
	inline void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }

	D3DXMATRIX              m_mLocal;
	D3DXMATRIX				m_mRotate;
	D3DMATERIAL9            m_mtrl;
	ID3DXMesh* m_pBoundMesh;
};

#endif