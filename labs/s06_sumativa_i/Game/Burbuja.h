#pragma once

#include <SDL.h>

#include "Engine/Matrix3x3.h"
#include "Engine/Vector2D.h"

// Burbuja indicadora de aterrizaje: hija de la botella en el suelo y fija en el destino al saltar.
void dibujarBurbuja(SDL_Renderer* r, const Matrix3x3& padreBotella, const Vector2D& destino,
                    bool enAire, int margenX, int camY);
