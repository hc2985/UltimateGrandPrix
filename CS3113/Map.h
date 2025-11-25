#include "cs3113.h"
#include <unordered_map>


struct MultiTileObject {
    Texture2D texture;
    int widthTiles;
    int heightTiles;
    Vector2 offset;
    float scale;
    float rotation;    
};

class Map
{
private:
    int mMapColumns; // number of columns in map
    int mMapRows;    // number of rows in map

    unsigned int *mLevelData; // array of tile indices
    Texture2D mTextureAtlas;  // texture atlas

    float mTileSize; // size of each tile in pixels

    int mTextureColumns; // number of columns in texture atlas
    int mTextureRows;    // number of rows in texture atlas

    std::vector<Rectangle> mTextureAreas; // texture areas for each tile
    Vector2 mOrigin; // center of the map in world coordinates

    float mLeftBoundary;  // left boundary of the map in world coordinates
    float mRightBoundary; // right boundary of the map in world coordinates
    float mTopBoundary;   // top boundary of the map in world coordinates
    float mBottomBoundary;// bottom boundary of the map in world coordinates

    std::unordered_map<int, MultiTileObject> mMultiTileObjects;

public:
    Map(int mapColumns, int mapRows, unsigned int *levelData,
        const char *textureFilePath, float tileSize, int textureColumns,
        int textureRows, Vector2 origin);
    ~Map();

    void build();
    void render();
    bool isSolidTileAt(Vector2 position, float *xOverlap, float *yOverlap);

    int           getMapColumns()     const { return mMapColumns;     };
    int           getMapRows()        const { return mMapRows;        };
    float         getTileSize()       const { return mTileSize;       };
    unsigned int* getLevelData()      const { return mLevelData;      };
    Texture2D     getTextureAtlas()   const { return mTextureAtlas;   };
    int           getTextureColumns() const { return mTextureColumns; };
    int           getTextureRows()    const { return mTextureRows;    };
    float         getLeftBoundary()   const { return mLeftBoundary;   };
    float         getRightBoundary()  const { return mRightBoundary;  };
    float         getTopBoundary()    const { return mTopBoundary;    };
    float         getBottomBoundary() const { return mBottomBoundary; };
    int           getTileAtWorldPos(Vector2 pos) const;

    Vector2 findTile(int tileID) const;

    void setTileType(int index, int tileType) 
        { mLevelData[index] = tileType; }

    void registerMultiTileObject(
        int tileID,
        const char* texturePath,
        int widthTiles,
        int heightTiles,
        Vector2 offset = {0,0},
        float scale = 1.0f,
        float rotation = 0.0f       
    );
};