
#include "cocos2d.h"
USING_NS_CC;

class HelloWorld:public Layer
{
public:
    Size visibleSize;
    Vec2 origin;
    Sprite* ball;
    Sprite* batSprite;
    Sprite* edgeSp;
    Sprite* prop;
    PhysicsBody* ballBody;
    PhysicsBody* batBody;
    TMXTiledMap* map;
    bool isFlying;
    Vector<Sprite*> allBlocks;
    Vector<Sprite*> allLives;
    int score;
    int lifeNum;
    
    static Scene* createScene();
    CREATE_FUNC(HelloWorld);
    bool init();
    HelloWorld();
    
    void loadPhysicsBody();
    void loadTileMap();
    void loadProp();
    void contact();
    void touch();
    void update(float dt);
    void isDead(float dt);
};