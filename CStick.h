#ifndef __CSTICK_H__
#define __CSTICK_H__

#include "d3dUtility.h"
#include "CSphere.h"

class CStick {
private:
	float                   v_x, v_y, v_z; //�籸ä�� ����Ű�� ����.
	float					m_x, m_y, m_z; //�籸ä�� ����Ű�� ��ġ.
	float					distance;	   //(�籸ä �Ӹ��� ���� ���� �ִ� ��ġ)�� m ������ �Ÿ�
public:
	CStick(void);
	~CStick(void);
public:
	bool create(IDirect3DDevice9* pDevice, float ix, float iz, D3DXCOLOR color);
	void destroy(void);
	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld);
	void setPosition(float x, float y, float z);
	void setDirection(float x, float y, float z);

	float getDistance() const { return distance; }
	float getX() const { return m_x; }
	float getY() const { return m_y; }
	float getZ() const { return m_z; }
	D3DXVECTOR3 getCenter(void) const;
private:
	void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }

	D3DXMATRIX              m_mLocal;
	D3DMATERIAL9            m_mtrl;
	ID3DXMesh* m_pBoundMesh;
};

#endif