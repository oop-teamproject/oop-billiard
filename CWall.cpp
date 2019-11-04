// -----------------------------------------------------------------------------
// CWall class definition
// -----------------------------------------------------------------------------
#include "CWall.h"
#include "CSphere.h"

CWall::CWall(void)
{
	D3DXMatrixIdentity(&m_mLocal);
	ZeroMemory(&m_mtrl, sizeof(m_mtrl));
	m_width = 0;
	m_depth = 0;
	m_height = 0;
	m_pBoundMesh = NULL;
}
CWall::~CWall(void) {}
bool CWall::create(IDirect3DDevice9* pDevice, float ix, float iz, float iwidth, float iheight, float idepth, D3DXCOLOR color = d3d::WHITE)
{
	if (NULL == pDevice)
		return false;

	m_mtrl.Ambient = color;
	m_mtrl.Diffuse = color;
	m_mtrl.Specular = color;
	m_mtrl.Emissive = d3d::BLACK;
	m_mtrl.Power = 5.0f;

	m_width = iwidth;
	m_depth = idepth;
	m_height = iheight;

	if (FAILED(D3DXCreateBox(pDevice, iwidth, iheight, idepth, &m_pBoundMesh, NULL)))
		return false;
	return true;
}

void CWall::destroy(void)
{
	if (m_pBoundMesh != NULL) {
		m_pBoundMesh->Release();
		m_pBoundMesh = NULL;
	}
}

void CWall::draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
{
	if (NULL == pDevice)
		return;
	pDevice->SetTransform(D3DTS_WORLD, &mWorld);
	pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
	pDevice->SetMaterial(&m_mtrl);
	m_pBoundMesh->DrawSubset(0);
}
bool CWall::hasIntersected(CSphere& ball) const
{	
	D3DXVECTOR3 ballCenter = ball.getCenter();
	D3DXVECTOR3 wallCenter = this->getCenter();
	float ballRadius = ball.getRadius();
	float width = getWidth() / 2;
	float height = getHeight() / 2;
	float depth = getDepth() / 2;
	//�浹���� �ʴ� ��찡 �ξ� �����Ƿ�, ����� ���� �ٿ���ڽ� �ȿ� ������ �ʴ� ��츦 ���� �����Ѵ�.
	if (ballCenter.x - ballRadius > wallCenter.x + width || ballCenter.x + ballRadius < wallCenter.x - width)
		return false;
	if (ballCenter.y - ballRadius > wallCenter.y + height || ballCenter.y + ballRadius < wallCenter.y - height)
		return false;
	if (ballCenter.z - ballRadius > wallCenter.z + depth || ballCenter.z + ballRadius < wallCenter.z - depth)
		return false;
	D3DXVECTOR3 closest = closestPoint(ball);
	D3DXVECTOR3 diff = closest - ballCenter;
	//���� �߽ɰ�, ���� ���� ����� ������ü ���� �� ������ �Ÿ��� �缭 �Ǵ��Ѵ�.
	return D3DXVec3Length(&diff) <= ballRadius;
}

//�Ķ���ͷ� �־��� ���� ���� ����� ������ü ���� ���� ���Ѵ�.
D3DXVECTOR3 CWall::closestPoint(CSphere& ball) const {
	D3DXVECTOR3 result;
	D3DXVECTOR3 ballCenter = ball.getCenter();
	D3DXVECTOR3 wallCenter = this->getCenter();
	float width = getWidth() / 2;
	float height = getHeight() / 2;
	float depth = getDepth() / 2;
	if (ballCenter.x < wallCenter.x - width)
		result.x = wallCenter.x - width;
	else if (ballCenter.x > wallCenter.x + width)
		result.x = wallCenter.x + width;
	else result.x = ballCenter.x;

	if (ballCenter.y < wallCenter.y - height)
		result.y = wallCenter.y - height;
	else if (ballCenter.y > wallCenter.y + height)
		result.y = wallCenter.y + height;
	else result.y = ballCenter.y;

	if (ballCenter.z < wallCenter.z - depth)
		result.z = wallCenter.z - depth;
	else if (ballCenter.z > wallCenter.z + depth)
		result.z = wallCenter.z + depth;
	else result.z = ballCenter.z;

	return result;
}

void CWall::hitBy(CSphere& ball)
{
	/*test code*/
	if (!hasIntersected(ball))
	{
		return;
	}
	else
	{
		//���� ���� ��ĥ ��� �ڷ� ���� ������.
		//���� ���� ���� �������� �ʰ� �ִ� ��� ����� ����Ƿ� �ӵ��� ���Ƿ� �����Ѵ�.
		if (ball.getVelocity() == D3DXVECTOR3(0, 0, 0))
			ball.setPower(1, 0, 0);
		D3DXVECTOR3 prevPos = ball.getCenter() - ball.getVelocity();
		D3DXVECTOR3 currentPos = ball.getCenter();
		D3DXVECTOR3 closest = closestPoint(ball);
		float prevDist;
		float currDist;
		prevDist = D3DXVec3Length(&(closest - prevPos));
		currDist = D3DXVec3Length(&(closest - currentPos));
		float multiplier = (ball.getRadius() - prevDist) / (currDist - prevDist);
		ball.setCenter(prevPos.x + multiplier * ball.getVelocity_X(), prevPos.y + multiplier * ball.getVelocity_Y(), prevPos.z + multiplier * ball.getVelocity_Z());
		//���� �ӵ��� ����Ѵ�.
		D3DXVECTOR3 ballToWall = closest - ball.getCenter();
		if (ballToWall == D3DXVECTOR3(0, 0, 0)) //closest�� ���� �߽��� ������ ���� Normalize�� �� ���� ���� ������ ���͸� ����.
			ballToWall.x = 1.0f;
		D3DXVec3Normalize(&ballToWall, &ballToWall);
		D3DXVECTOR3 vTowardWall;
		vTowardWall = ballToWall * D3DXVec3Dot(&ballToWall, &ball.getVelocity());
		D3DXVECTOR3 vOrthogonal = ball.getVelocity() - vTowardWall;
		vTowardWall = -vTowardWall;
		D3DXVECTOR3 newVelocity = vTowardWall + vOrthogonal;
		if (newVelocity.y <= 0.1)
			ball.setPower(newVelocity.x, 0, newVelocity.z);
		else
			ball.setPower(newVelocity.x, newVelocity.y, newVelocity.z);
		//�ڷ� ���ȴ� ��ŭ �ٽ� ������ �̵���Ų��.
		currentPos = ball.getCenter();
		currentPos += (1 - multiplier) * ball.getVelocity();
		ball.setCenter(currentPos.x, currentPos.y, currentPos.z);
	}
}

void CWall::setPosition(float x, float y, float z)
{
	D3DXMATRIX m;
	this->m_x = x;
	this->m_y = y;
	this->m_z = z;

	D3DXMatrixTranslation(&m, x, y, z);
	setLocalTransform(m);
}

D3DXVECTOR3 CWall::getCenter(void) const
{
	D3DXVECTOR3 org(m_x, m_y, m_z);
	return org;
}
