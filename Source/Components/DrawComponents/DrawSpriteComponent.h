//
// Created by Lucas N. Ferreira on 28/09/23.
//

#pragma once

#include "DrawComponent.h"
#include <string>

class DrawSpriteComponent : public DrawComponent {
public:
    // (Lower draw order corresponds with further back)
    DrawSpriteComponent(Actor* owner, const std::string& texturePath, int width = 0, int height = 0,
                        int drawOrder = 100);

    ~DrawSpriteComponent() override;

    void Draw(SDL_Renderer* renderer) override;

    void SetWidth(float width) { mWidth = static_cast<int>(width); }
    void SetHeight(float height) { mHeight = static_cast<int>(height); }

protected:
    SDL_Texture* mSpriteSheetSurface;

    int mWidth;
    int mHeight;
};
