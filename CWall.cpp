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
	//충돌하지 않는 경우가 훨씬 많으므로, 충분히 넓은 바운딩박스 안에 들어오지 않는 경우를 먼저 배제한다.
	if (ballCenter.x - ballRadius > wallCenter.x + width || ballCenter.x + ballRadius < wallCenter.x - width)
		return false;
	if (ballCenter.y - ballRadius > wallCenter.y + height || ballCenter.y + ballRadius < wallCenter.y - height)
		return false;
	if (ballCenter.z - ballRadius > wallCenter.z + depth || ballCenter.z + ballRadius < wallCenter.z - depth)
		return false;
	D3DXVECTOR3 closest = closestPoint(ball);
	D3DXVECTOR3 diff = closest - ballCenter;
	//구의 중심과, 구에 가장 가까운 정육면체 위의 점 사이의 거리를 재서 판단한다.
	return D3DXVec3Length(&diff) <= ballRadius;
}

//파라미터로 주어진 구에 가장 가까운 직육면체 위의 점을 구한다.
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
		//공과 벽이 겹칠 경우 뒤로 조금 돌린다.
		//만약 공이 전혀 움직이지 않고 있던 경우 모순이 생기므로 속도를 임의로 설정한다.
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
		//새로 속도를 계산한다.
		D3DXVECTOR3 ballToWall = closest - ball.getCenter();
		if (ballToWall == D3DXVECTOR3(0, 0, 0)) //closest와 구의 중심이 완전히 겹쳐 Normalize할 수 없는 경우는 임의의 벡터를 쓴다.
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
		//뒤로 돌렸던 만큼 다시 앞으로 이동시킨다.
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
