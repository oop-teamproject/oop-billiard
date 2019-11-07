#include "CStick.h"

CStick::CStick(void) {
	D3DXMatrixIdentity(&m_mLocal);
	ZeroMemory(&m_mtrl, sizeof(m_mtrl));
	v_y = 0;
	m_x = 0;
	m_y = 0;
	m_z = 0;
	distance = 0;
	m_pBoundMesh = NULL;
}
CStick::~CStick(void) {}
bool CStick::create(IDirect3DDevice9* pDevice, D3DXCOLOR color)
{
	if (NULL == pDevice)
		return false;

	m_mtrl.Ambient = color;
	m_mtrl.Diffuse = color;
	m_mtrl.Specular = color;
	m_mtrl.Emissive = d3d::BLACK;
	m_mtrl.Power = 5.0f;

	if (FAILED(D3DXCreateCylinder(pDevice, 0.16f, 0.04f, 4.0f, 20, 20, &m_pBoundMesh, NULL)))
		return false;
	return true;
}
void CStick::destroy(void) {
	if (m_pBoundMesh != NULL) {
		m_pBoundMesh->Release();
		m_pBoundMesh = NULL;
	}
}
void CStick::draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld) {
	if (NULL == pDevice)
		return;
	pDevice->SetTransform(D3DTS_WORLD, &mWorld);
	pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
	pDevice->MultiplyTransform(D3DTS_WORLD, &m_mRotate);
	pDevice->SetMaterial(&m_mtrl);
	m_pBoundMesh->DrawSubset(0);
}
void CStick::setPosition(float x, float y, float z) {
	D3DXMATRIX m;
	this->m_x = x;
	this->m_y = y;
	this->m_z = z;

	D3DXMatrixTranslation(&m, x, y, z);
	setLocalTransform(m);
}
void CStick::setDirection(float y) {
	v_y = y;
	D3DXMatrixRotationY(&m_mRotate, y);
}
void CStick::viewAt(float x, float z) {
	x -= m_x;
	z -= m_z;
	if (x == 0 && z == 0) return;
	float phi = std::atan2f(x, z); //xz평면에 내렸을 때 y축 방향으로 회전한 각도
	D3DXMATRIX vY;
	D3DXMatrixRotationY(&vY, phi);
	m_mRotate = vY;
}