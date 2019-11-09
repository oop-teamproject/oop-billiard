#ifndef __CSCOREBOARD_H__
#define __CSCOREBOARD_H__

#include <string>
#include "d3dUtility.h"

class CScoreBoard {
private:
	int score[2];
public:
	CScoreBoard(void);
	~CScoreBoard(void) {}
public:
	bool create(IDirect3DDevice9* pDevice, int fontsize, char* fontname = "Arial", D3DXCOLOR color = d3d::BLACK);
	void destroy(void) {}
	void draw(IDirect3DDevice9* pDevice);
	void setFontArea(int left, int top, int right, int bottom);
	void setScore(int a, int b) { score[0] = a, score[1] = b; }
	int getScoreA() const { return score[0]; }
	int getScoreB() const { return score[1]; }
	void addScoreA(int diff = 1);
	void addScoreB(int diff = 1);
	void resetScore() { score[0] = score[1] = 0; }
private:
	LPD3DXFONT Font;
	RECT fontArea;
	D3DXCOLOR color;
};

#endif