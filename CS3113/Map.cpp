#include "Map.h"

Map::Map(int mapColumns, int mapRows, unsigned int *levelData,
         const char *textureFilePath, float tileSize, int textureColumns,
         int textureRows, Vector2 origin) : 
         mMapColumns {mapColumns}, mMapRows {mapRows}, 
         mTextureAtlas { LoadTexture(textureFilePath) },
         mLevelData {levelData }, mTileSize {tileSize}, 
         mTextureColumns {textureColumns}, mTextureRows {textureRows},
         mOrigin {origin} { build(); }

Map::~Map() { UnloadTexture(mTextureAtlas); }

void Map::build()
{
    // Calculate map boundaries in world coordinates
    mLeftBoundary   = mOrigin.x - (mMapColumns * mTileSize) / 2.0f;
    mRightBoundary  = mOrigin.x + (mMapColumns * mTileSize) / 2.0f;
    mTopBoundary    = mOrigin.y - (mMapRows * mTileSize) / 2.0f;
    mBottomBoundary = mOrigin.y + (mMapRows * mTileSize) / 2.0f;

    // Precompute texture areas for each tile
    for (int row = 0; row < mTextureRows; row++)
    {
        for (int col = 0; col < mTextureColumns; col++)
        {
            Rectangle textureArea = {
                (float) col * (mTextureAtlas.width / mTextureColumns),
                (float) row * (mTextureAtlas.height / mTextureRows),
                (float) mTextureAtlas.width / mTextureColumns,
                (float) mTextureAtlas.height / mTextureRows
            };

            mTextureAreas.push_back(textureArea);
        }
    }
}

void Map::render()
{
    // Draw each tile in the map
    for (int row = 0; row < mMapRows; row++)
    {
        // Draw each column in the row
        for (int col = 0; col < mMapColumns; col++)
        {
            // Get the tile index at the current row and column
            int tile = mLevelData[row * mMapColumns + col];

            // If the tile index is 0, we do not draw anything
            if (tile == 0) continue;
            if (mMultiTileObjects.count(tile)) continue;

            Rectangle destinationArea = {
                mLeftBoundary + col * mTileSize,
                mTopBoundary  + row * mTileSize, // y-axis is inverted
                mTileSize,
                mTileSize
            };

            // Draw the tile
            DrawTexturePro(
                mTextureAtlas,
                mTextureAreas[tile - 1], // -1 because tile indices start at 1
                destinationArea,
                {0.0f, 0.0f}, // origin
                0.0f,         // rotation
                WHITE         // tint
            );
        }
    }

    for (int row = 0; row < mMapRows; row++)
    {
        for (int col = 0; col < mMapColumns; col++)
        {
            int tile = mLevelData[row * mMapColumns + col];

            if (!mMultiTileObjects.count(tile))
                continue;

            MultiTileObject &obj = mMultiTileObjects[tile];

            // Position of TOP-LEFT in world space
            float px = mLeftBoundary + col * mTileSize + obj.offset.x;
            float py = mTopBoundary  + row * mTileSize + obj.offset.y;

            // World size of the object (what matters)
            float worldW = obj.widthTiles  * mTileSize;
            float worldH = obj.heightTiles * mTileSize;

            Rectangle src = {
                0,
                0,
                (float)obj.texture.width,
                (float)obj.texture.height
            };

            // Calculate origin point based on rotation so that top-left
            // of rotated sprite ends up at (px, py)
            Vector2 origin;

            if (fabs(obj.rotation) < 0.1f) {
                // 0 degrees: top-left stays top-left
                origin = { 0.0f, 0.0f };
            }
            else if (fabs(obj.rotation - 90.0f) < 0.1f) {
                // 90 degrees: top-left becomes bottom-left
                origin = { 0.0f, worldH };
            }
            else if (fabs(obj.rotation - 180.0f) < 0.1f) {
                // 180 degrees: top-left becomes bottom-right
                origin = { worldW, worldH };
            }
            else if (fabs(obj.rotation - 270.0f) < 0.1f || fabs(obj.rotation + 90.0f) < 0.1f) {
                // 270 degrees: top-left becomes top-right
                origin = { worldW, 0.0f };
            }
            else {
                // General case for arbitrary rotations
                origin = { 0.0f, 0.0f };
            }

            Rectangle dst = {
                px,
                py,
                worldW,
                worldH
            };

            DrawTexturePro(
                obj.texture,
                src,
                dst,
                origin,
                obj.rotation,
                WHITE
            );
        }
    }
}

bool Map::isSolidTileAt(Vector2 position, float *xOverlap, float *yOverlap)
{
    *xOverlap = 0.0f;
    *yOverlap = 0.0f;

    if (position.x < mLeftBoundary || position.x > mRightBoundary ||
        position.y < mTopBoundary  || position.y > mBottomBoundary)
        return false;

    int tileXIndex = floor((position.x - mLeftBoundary) / mTileSize);
    int tileYIndex = floor((position.y - mTopBoundary) / mTileSize);

    if (tileXIndex < 0 || tileXIndex >= mMapColumns ||
        tileYIndex < 0 || tileYIndex >= mMapRows)
        return false;

    // Check tiles for multi-tile objects
    for (int dy = -2; dy <= 2; dy++)
    {
        for (int dx = -2; dx <= 2; dx++)
        {
            int checkCol = tileXIndex + dx;
            int checkRow = tileYIndex + dy;

            if (checkCol < 0 || checkCol >= mMapColumns ||
                checkRow < 0 || checkRow >= mMapRows)
                continue;

            int tile = mLevelData[checkRow * mMapColumns + checkCol];

            if (!mMultiTileObjects.count(tile))
                continue;

            MultiTileObject &obj = mMultiTileObjects[tile];

            // get dimensions
            float baseWidth = obj.widthTiles * mTileSize;
            float baseHeight = obj.heightTiles * mTileSize;

            // swap dimensions for rotations
            float objWidth = baseWidth;
            float objHeight = baseHeight;

            if (fabs(obj.rotation - 90.0f) < 0.1f || fabs(obj.rotation - 270.0f) < 0.1f || fabs(obj.rotation + 90.0f) < 0.1f)
            {
                objWidth = baseHeight;
                objHeight = baseWidth;
            }

            // Calculate collision box position (top-left of rotated object)
            float objLeft = mLeftBoundary + checkCol * mTileSize + obj.offset.x;
            float objTop = mTopBoundary + checkRow * mTileSize + obj.offset.y;

            float objCenterX = objLeft + objWidth / 2.0f;
            float objCenterY = objTop + objHeight / 2.0f;

            // Check if position is inside this object's bounding box
            float halfWidth = objWidth / 2.0f;
            float halfHeight = objHeight / 2.0f;

            float distX = fabs(position.x - objCenterX);
            float distY = fabs(position.y - objCenterY);

            if (distX < halfWidth && distY < halfHeight)
            {
                *xOverlap = halfWidth - distX;
                *yOverlap = halfHeight - distY;
                return true;
            }
        }
    }
    // no collision found
    return false;
}

void Map::registerMultiTileObject(
    int tileID,
    const char* texturePath,
    int widthTiles,
    int heightTiles,
    Vector2 offset,
    float scale,
    float rotation
)
{
    Texture2D tex = LoadTexture(texturePath);

    MultiTileObject obj;
    obj.texture    = tex;
    obj.widthTiles = widthTiles;
    obj.heightTiles = heightTiles;
    obj.offset     = offset;
    obj.scale      = scale;
    obj.rotation   = rotation;

    mMultiTileObjects[tileID] = obj;
}

Vector2 Map::findTile(int tileID) const {
    for(int row = 0; row < mMapRows; row++) {
        for(int col = 0; col < mMapColumns; col++) {

            int idx = row * mMapColumns + col;
            if (mLevelData[idx] == tileID) {
                float x = mLeftBoundary + col * mTileSize + mTileSize * 0.5f;
                float y = mTopBoundary  + row * mTileSize + mTileSize * 0.5f;
                return {x, y};
            }
        }
    }

    return {0,0}; //default
}

int Map::getTileAtWorldPos(Vector2 pos) const
{
    int col = (int)((pos.x - mLeftBoundary) / mTileSize);
    int row = (int)((pos.y - mTopBoundary)  / mTileSize);

    if (col < 0 || row < 0 || col >= mMapColumns || row >= mMapRows)
        return 0;

    return mLevelData[row * mMapColumns + col];
}

