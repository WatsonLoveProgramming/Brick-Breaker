#include "HelloWorldScene.h"
HelloWorld::HelloWorld():isFlying(false),score(0), lifeNum(3){}

Scene* HelloWorld::createScene()
{
    auto scene = Scene::createWithPhysics();
    scene->getPhysicsWorld()->setGravity(Vec2(0,0));
    scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
    
    auto layer = HelloWorld::create();
    scene->addChild(layer);
    return scene;
}

bool HelloWorld::init()
{
    if ( !Layer::init() )
    {
        return false;
    }
    loadTileMap();
    loadPhysicsBody();
    loadProp();
    return true;
}

void HelloWorld::loadTileMap()
{
    map = TMXTiledMap::create("textmap.tmx");
    addChild(map);
    TMXLayer* layer = map->getLayer("bricks");
    for(int x = 0;x < 15;x++)
    {
        for(int y = 0;y < 25;y++)
        {
            int gid = layer->getTileGIDAt(Vec2(x,y));
            if(gid != 12)
            {
                Sprite* block = layer->getTileAt(Vec2(x,y));
                if(!block)
                    continue;
                PhysicsBody* body = PhysicsBody::createEdgeBox(block->getContentSize(),PhysicsMaterial(1.0f,1.0f,0.0f));
                body->setContactTestBitmask(0xFFFFFFFF);
                block->setName("block");
                allBlocks.pushBack(block);
                block->setPhysicsBody(body);
            }
        }
    }
}

void HelloWorld::loadPhysicsBody()
{
    visibleSize = Director::getInstance()->getVisibleSize();
    origin = Director::getInstance()->getVisibleOrigin();

    //边框
    edgeSp = Sprite::create();
    auto boundBody = PhysicsBody::createEdgeBox(visibleSize,PhysicsMaterial(0.0f,1.0f,0.0f),3);
    edgeSp->setPosition(visibleSize.width/2, visibleSize.height/2);
    edgeSp->setPhysicsBody(boundBody);
    addChild(edgeSp);

    //弹板
    batSprite = Sprite::create("bar.png");
    batBody = PhysicsBody::createEdgeBox(batSprite->getContentSize(),PhysicsMaterial(0.0f,1.0f,0.0f));
    batBody->setContactTestBitmask(0xFFFFFFFF);
    batSprite->setPhysicsBody(batBody);
    batSprite->setPosition(Director::getInstance()->getWinSize().width/2,50);
    addChild(batSprite);
    
    //球
    ball = Sprite::create("ball.png");
    ball->setPosition(Vec2(batSprite->getPositionX(),batSprite->getPositionY()+20));
    ballBody = PhysicsBody::createCircle(ball->getContentSize().width/2,PhysicsMaterial(0.0f,1.0f,0.0f));
    ballBody->setContactTestBitmask(0xFFFFFFFF);
    ball->setPhysicsBody(ballBody);
    ball->setName("ball");
    addChild(ball);
    
    //分数
    auto score = Label::createWithTTF("Score: 0", "fonts/Marker Felt.ttf", 24);
    score->setPositionX(visibleSize.width - score->getContentSize().width/2 - 20);
    score->setPositionY(visibleSize.height - score->getContentSize().height/2 - 5);
    score->setColor(Color3B::YELLOW);
    score->setName("score");
    addChild(score);
    
    //生命
    for (int i = 0; i < lifeNum; i++){
        allLives.pushBack(Sprite::create("life.png"));
        allLives.at(i)->setPosition(i * 35 + 16, visibleSize.height - 16);
        addChild(allLives.at(i));
    }
    
    touch();
    contact();
    scheduleUpdate();
    
}

void HelloWorld::loadProp()
{
    TMXObjectGroup* objects = map->getObjectGroup("prop");
    auto spawnPoint = objects->getObject("pop");
    int x = spawnPoint["x"].asInt();
    int y = spawnPoint["y"].asInt();
    prop = Sprite::create("item.png");
    prop->setPosition(x,y);
    prop->setZOrder(10);
    prop->setName("item");
    
    auto propBody = PhysicsBody::createBox(prop->getContentSize(),PhysicsMaterial(0.0f,1.0f,0.0f));
    propBody->setContactTestBitmask(0xFFFFFFFF);
    prop->setPhysicsBody(propBody);
    
    map->addChild(prop);
}

void HelloWorld::touch()
{
    EventListenerTouchOneByOne* ev1 = EventListenerTouchOneByOne::create();
    ev1->onTouchBegan = [](Touch* touch,Event* ev){return true;};
    ev1->onTouchMoved = [=](Touch* touch,Event* ev){
        float x = touch->getDelta().x;
        batSprite->setPositionX(batSprite->getPositionX() + x);
        
        //弹板碰左右边
        if (batSprite->getPositionX() + batSprite->getContentSize().width / 2 >= visibleSize.width)
        {
            batSprite->setPositionX(visibleSize.width - batSprite->getContentSize().width / 2);
        }
        if (batSprite->getPositionX() - batSprite->getContentSize().width /2 <= 0)
        {
            batSprite->setPositionX(batSprite->getContentSize().width / 2);
        }
    };
    ev1->onTouchEnded = [=](Touch* touch,Event* ev){isFlying=true;};
    _eventDispatcher->addEventListenerWithSceneGraphPriority(ev1, this);
}

void HelloWorld::contact()
{
    auto newScore = (Label*)getChildByName("score");
    
    EventListenerPhysicsContact* evContact = EventListenerPhysicsContact::create();
    evContact->onContactBegin = [](PhysicsContact& contact){return true;};
    evContact->onContactSeparate = [=](PhysicsContact& contact)
    {
        auto bodyA = (Sprite*)(contact.getShapeA()->getBody()->getNode());
        auto bodyB = (Sprite*)(contact.getShapeB()->getBody()->getNode());
        if(!bodyA||!bodyB)
            return;
        if(bodyA->getName() == "ball" || bodyB->getName() == "ball")
        {
            if(bodyB->getName() == "ball")  //若球是B, 把球变成bodyA，另一碰撞物设为B
            {
                auto bodyC = bodyA;
                bodyA = bodyB;
                bodyB = bodyC;
            }
            if(bodyB->getName() == "block")
            {
                allBlocks.eraseObject(bodyB);
                bodyB->removeFromParentAndCleanup(true);
                score++;
                newScore->setString(__String::createWithFormat("Score: %d",score)->_string);
            }
            else if (bodyB->getName() == "item")
            {
                bodyB->removeFromParentAndCleanup(true);
                for (int i = 0; i < allBlocks.size(); i++) {
                    Sprite* block = allBlocks.at(i);
                    PhysicsBody* body = block->getPhysicsBody();
                    body->setCollisionBitmask(0x00000000);
                }
            }
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(evContact,this);
}

void HelloWorld::update(float dt)
{
    if (isFlying) {
        float x = ballBody->getVelocity().x;
        float y = ballBody->getVelocity().y;
        if(x!=300 && x!=-300)
        {
            if(x < 0)
                x = -300;
            else
                x = 300;
        }
        if(y!=300 && y!=-300)
        {
            if(y < 0)
                y = -300;
            else
                y = 300;
        }
        ballBody->setVelocity(Vec2(x,y));
    }else
        ball->setPositionX(batSprite->getPositionX());
    
    if (ball->getPositionY() < batSprite->getPositionY() - 20) {
        lifeNum--;
        if (lifeNum >= 0) {
            allLives.at(lifeNum)->removeFromParentAndCleanup(true);
            allLives.erase(lifeNum);
        }
        pause();
        ballBody->setVelocity(Vec2(0,0));
        this->scheduleOnce(schedule_selector(HelloWorld::isDead), 2.0);
    }
}

void HelloWorld::isDead(float t)
{
    
    if (lifeNum != 0) {
        ball->setPosition(Vec2(batSprite->getPositionX(),batSprite->getPositionY()+20));
        isFlying=false;
        resume();
    }else{
        auto gameover = Label::createWithTTF("Game Over", "fonts/Marker Felt.ttf", 64);
        gameover->setPosition(visibleSize.width / 2, visibleSize.height /2);
        addChild(gameover);
    }
   
}