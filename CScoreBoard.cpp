#include "CScoreboard.h"

CScoreBoard::CScoreBoard(void) {
	resetScore();
	fontArea.left = 0;
	fontArea.right = 0;
	fontArea.top = 0;
	fontArea.bottom = 0;
	color = d3d::BLACK;
}

bool CScoreBoard::create(IDirect3DDevice9* pDevice, int fontsize, char* fontname, D3DXCOLOR color) {
	D3DXCreateFont(pDevice, 50, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, fontname, &Font);
	this->color = color;
	return true;
}

void CScoreBoard::draw(IDirect3DDevice9* pDevice) {
	char str[100];
	sprintf(str, "%d : %d", score[0], score[1]);
	Font->DrawText(NULL, str, -1, &fontArea, DT_CENTER | DT_NOCLIP, color);
}
void CScoreBoard::addScoreA(int diff) {
	score[0] += diff;
	if (score[0] < 0)
		score[0] = 0;
}
void CScoreBoard::addScoreB(int diff) {
	score[1] += diff;
	if (score[1] < 0)
		score[1] = 0;
}
void CScoreBoard::setFontArea(int left, int top, int right, int bottom) {
	fontArea.left = left;
	fontArea.top = top;
	fontArea.right = right;
	fontArea.bottom = bottom;
}