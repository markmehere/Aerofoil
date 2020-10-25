#pragma once

struct Point;
struct Window;

void StartMainMenuUI();
void TickMainMenuUI();
void DismissMainMenuUI();
void HandleMainMenuUIResolutionChange();
bool HandleMainMenuUIClick(Window *window, const Point &pt);
