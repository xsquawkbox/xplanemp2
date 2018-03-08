//
// Created by kuroneko on 3/03/2018.
//

#ifndef RENDERER_H
#define RENDERER_H

#include <string>
#include <vector>

extern XPLMDataRef		gVisDataRef;		// Current air visiblity for culling.
extern XPLMProbeRef		gTerrainProbe;

extern double	Render_LabelDistance;
extern double	Render_FullPlaneDistance;
extern GLfloat	Render_LabelViewport[4];

struct Label {
	int			x;
	int			y;
	float 		distSqr;
	std::string	labelText;
};

extern std::vector<Label>	gLabelList;

void	Renderer_Init();
void	Renderer_Attach_Callbacks();
void	Renderer_Detach_Callbacks();


#endif //RENDERER_H
