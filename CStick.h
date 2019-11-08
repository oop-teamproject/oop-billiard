#ifndef __CSTICK_H__
#define __CSTICK_H__

#include "d3dUtility.h"
#include "CSphere.h"
#include <cmath>
#define PI 3.14159265
class CStick {
private:
	float                   v_y;			//당구채가 가리키는 방향. +x방향이 0, 위에서 볼 때 시계방향으로 회전한다. 단위 rad
	float					m_x, m_y, m_z;	//당구채의 중심.
	float					length;			//당구채의 길이.
	float					visible;		//현재 당구채를 그려야 하는지 판별
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

	inline float getX() const { return m_x; }
	inline float getY() const { return m_y; }
	inline float getZ() const { return m_z; }
	inline float getDirection() const { return v_y; }
	inline D3DXVECTOR3 getCenter(void) const { return D3DXVECTOR3(m_x, m_y, m_z); }
	void setVisible(bool isTrue) { visible = isTrue; }
	void setPosToward(float x, float y, float z, float distance, float direction); //set position toward (x, m_y, z). distance between cue and ball are distance, and rotated toward +y from +x axis by direction rad.
private:
	inline void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }

	D3DXMATRIX              m_mLocal;
	D3DXMATRIX				m_mRotate;
	D3DMATERIAL9            m_mtrl;
	ID3DXMesh* m_pBoundMesh;
};

#endif