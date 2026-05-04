#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include <conio.h>
#include <ctime>
#include <cstdlib>
#include <algorithm>

using namespace std;

// ===================== Console helpers =====================
void gotoXY(int x, int y)
{
    COORD pos;
    pos.X = static_cast<SHORT>(x);
    pos.Y = static_cast<SHORT>(y);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(hOut, pos);
}

void HideCursor()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hOut, &cursorInfo);
}

void SetConsoleWindow(int width, int height)
{
    char cmd[128];
    sprintf(cmd, "mode con cols=%d lines=%d", width, height);
    system(cmd);
}

// ===================== Game constants =====================
const int FIELD_WIDTH = 55;
const int FIELD_HEIGHT = 35;
const int INITIAL_SCORE = 5;
const int TOTAL_ENEMIES = 50;
const int FRAME_DELAY = 50;

// ===================== Basic structs =====================
struct Bullet
{
    int x;
    int y;
    int dy;       // -1: up, +1: down
    bool fromPlayer;
    bool alive;

    Bullet(int _x, int _y, int _dy, bool _fromPlayer)
        : x(_x), y(_y), dy(_dy), fromPlayer(_fromPlayer), alive(true)
    {
    }

    void move()
    {
        y += dy;
        if (y < 1 || y > FIELD_HEIGHT - 2)
        {
            alive = false;
        }
    }
};

class Plane
{
protected:
    int x;
    int y;
    int width;
    int height;
    vector<string> shape;
    bool alive;

public:
    Plane(int _x, int _y, const vector<string>& _shape)
        : x(_x), y(_y), shape(_shape), alive(true)
    {
        height = static_cast<int>(shape.size());
        width = 0;
        for (size_t i = 0; i < shape.size(); ++i)
        {
            width = max(width, static_cast<int>(shape[i].size()));
        }
    }

    virtual ~Plane() {}

    virtual void draw(vector<string>& canvas) const
    {
        if (!alive) return;

        for (int i = 0; i < height; ++i)
        {
            for (int j = 0; j < static_cast<int>(shape[i].size()); ++j)
            {
                char ch = shape[i][j];
                int px = x + j;
                int py = y + i;
                if (ch != ' ' && px >= 1 && px <= FIELD_WIDTH - 2 && py >= 1 && py <= FIELD_HEIGHT - 2)
                {
                    canvas[py][px] = ch;
                }
            }
        }
    }

    bool contains(int px, int py) const
    {
        if (!alive) return false;

        for (int i = 0; i < height; ++i)
        {
            for (int j = 0; j < static_cast<int>(shape[i].size()); ++j)
            {
                if (shape[i][j] == ' ') continue;
                if (x + j == px && y + i == py)
                {
                    return true;
                }
            }
        }
        return false;
    }

    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    bool isAlive() const { return alive; }
    void setAlive(bool value) { alive = value; }
};

class PlayerPlane : public Plane
{
public:
    PlayerPlane()
        : Plane(FIELD_WIDTH / 2 - 2, FIELD_HEIGHT - 3, vector<string>{
            " /=\\ ",
            "<<*>>",
            " * * "
        })
    {
    }

    void moveLeft()
    {
        if (x > 1) x--;
    }

    void moveRight()
    {
        if (x + width < FIELD_WIDTH - 1) x++;
    }

    void moveUp()
    {
        if (y > 1) y--;
    }

    void moveDown()
    {
        if (y + height < FIELD_HEIGHT - 1) y++;
    }

    Bullet shoot() const
    {
        return Bullet(x + width / 2, y - 1, -1, true);
    }
};

class EnemyPlane : public Plane
{
    int moveTick;

public:
    EnemyPlane(int _x, int _y)
        : Plane(_x, _y, vector<string>{
            " \\+/ ",
            "  |",
            "  "
        }), moveTick(0)
    {
    }

    void update()
    {
        if (!alive) return;

        moveTick++;
        if (moveTick % 4 == 0)
        {
            y++;
        }

        if (y > FIELD_HEIGHT - 4)
        {
            alive = false;
        }
    }

    bool canShoot() const
    {
        if (!alive) return false;
        return (rand() % 100) < 2;   // ÉÔÎ¢½µµÍ·¢µ¯¸ÅÂÊ£¬»­Ãæ¸üÎÈ
    }

    Bullet shoot() const
    {
        return Bullet(x + width / 2, y + height, 1, false);
    }
};

class Game
{
private:
    PlayerPlane player;
    vector<EnemyPlane> enemies;
    vector<Bullet> bullets;
    int score;
    int enemiesSpawned;
    int enemiesDestroyed;
    bool running;
    int frameCounter;
    int playerShootCooldown;   // ÐÂÔö£ºÍæ¼ÒÉä»÷ÀäÈ´

public:
    Game()
        : score(INITIAL_SCORE),
          enemiesSpawned(0),
          enemiesDestroyed(0),
          running(true),
          frameCounter(0),
          playerShootCooldown(0)
    {
        srand(static_cast<unsigned int>(time(0)));
        HideCursor();
        SetConsoleWindow(80, 45);
    }

    void spawnEnemy()
    {
        if (enemiesSpawned >= TOTAL_ENEMIES) return;

        if (frameCounter % 12 == 0)
        {
            int enemyWidth = 5;
            int ex = 1 + rand() % (FIELD_WIDTH - 2 - enemyWidth + 1);
            enemies.push_back(EnemyPlane(ex, 1));
            enemiesSpawned++;
        }
    }

    void processInput()
    {
        if (_kbhit())
        {
            char ch = _getch();
            if (ch == 'a' || ch == 'A')
            {
                player.moveLeft();
            }
            else if (ch == 'd' || ch == 'D')
            {
                player.moveRight();
            }
            else if (ch == 'w' || ch == 'W')
            {
                player.moveUp();
            }
            else if (ch == 's' || ch == 'S')
            {
                player.moveDown();
            }
            else if (ch == ' ')
            {
                if (playerShootCooldown == 0)
                {
                    bullets.push_back(player.shoot());
                    playerShootCooldown = 4; // 4Ö¡ÀäÈ´
                }
            }
            else if (ch == 'q' || ch == 'Q')
            {
                running = false;
            }
        }

        if (playerShootCooldown > 0)
        {
            playerShootCooldown--;
        }
    }

    void updateEnemies()
    {
        for (size_t i = 0; i < enemies.size(); ++i)
        {
            enemies[i].update();

            if (enemies[i].canShoot())
            {
                bullets.push_back(enemies[i].shoot());
            }

            if (enemies[i].isAlive())
            {
                for (int iy = enemies[i].getY(); iy < enemies[i].getY() + enemies[i].getHeight(); ++iy)
                {
                    for (int ix = enemies[i].getX(); ix < enemies[i].getX() + enemies[i].getWidth(); ++ix)
                    {
                        if (player.contains(ix, iy))
                        {
                            enemies[i].setAlive(false);
                            score--;
                            if (score <= 0)
                            {
                                running = false;
                            }
                        }
                    }
                }
            }
        }
    }

    void updateBullets()
    {
        for (size_t i = 0; i < bullets.size(); ++i)
        {
            if (!bullets[i].alive) continue;

            bullets[i].move();
            if (!bullets[i].alive) continue;

            if (bullets[i].fromPlayer)
            {
                for (size_t j = 0; j < enemies.size(); ++j)
                {
                    if (enemies[j].isAlive() && enemies[j].contains(bullets[i].x, bullets[i].y))
                    {
                        enemies[j].setAlive(false);
                        bullets[i].alive = false;
                        score++;
                        enemiesDestroyed++;
                        break;
                    }
                }
            }
            else
            {
                if (player.contains(bullets[i].x, bullets[i].y))
                {
                    bullets[i].alive = false;
                    score--;
                    if (score <= 0)
                    {
                        running = false;
                    }
                }
            }
        }

        bullets.erase(remove_if(bullets.begin(), bullets.end(),
            [](const Bullet& b) { return !b.alive; }),
            bullets.end());

        enemies.erase(remove_if(enemies.begin(), enemies.end(),
            [](const EnemyPlane& e) { return !e.isAlive(); }),
            enemies.end());
    }

    bool win() const
    {
        return enemiesSpawned >= TOTAL_ENEMIES && enemies.empty();
    }

    void drawFrame()
    {
        vector<string> canvas(FIELD_HEIGHT, string(FIELD_WIDTH, ' '));

        for (int x = 0; x < FIELD_WIDTH; ++x)
        {
            canvas[0][x] = '=';
            canvas[FIELD_HEIGHT - 1][x] = '=';
        }
        for (int y = 0; y < FIELD_HEIGHT; ++y)
        {
            canvas[y][0] = '|';
            canvas[y][FIELD_WIDTH - 1] = '|';
        }

        player.draw(canvas);

        for (size_t i = 0; i < enemies.size(); ++i)
        {
            enemies[i].draw(canvas);
        }

        for (size_t i = 0; i < bullets.size(); ++i)
        {
            if (!bullets[i].alive) continue;
            if (bullets[i].x >= 1 && bullets[i].x <= FIELD_WIDTH - 2 &&
                bullets[i].y >= 1 && bullets[i].y <= FIELD_HEIGHT - 2)
            {
                canvas[bullets[i].y][bullets[i].x] = bullets[i].fromPlayer ? '|' : '!';
            }
        }

        gotoXY(0, 0);
        cout << "============== Plane War (Console Version) ==============\n";
        cout << "Move: W A S D   Shoot: Space   Quit: Q\n";
        cout << "Score: " << score
             << "   Spawned: " << enemiesSpawned << "/" << TOTAL_ENEMIES
             << "   Destroyed: " << enemiesDestroyed
             << "   Active: " << enemies.size() << "      \n";

        for (int y = 0; y < FIELD_HEIGHT; ++y)
        {
            cout << canvas[y] << "\n";
        }
    }

    void showResult()
    {
        gotoXY(0, FIELD_HEIGHT + 5);
        if (score <= 0)
        {
            cout << "Game Over! Your score reached 0.                \n";
        }
        else if (win())
        {
            cout << "Congratulations! You cleared all enemies.       \n";
        }
        else
        {
            cout << "Game exited by user.                            \n";
        }
        cout << "Final Score: " << score << "                    \n";
    }

    void run()
    {
        while (running)
        {
            frameCounter++;
            processInput();
            spawnEnemy();
            updateEnemies();
            updateBullets();
            drawFrame();

            if (score <= 0 || win())
            {
                running = false;
            }

            Sleep(FRAME_DELAY);
        }

        showResult();
    }
};

int main()
{
    Game game;
    game.run();
    system("pause");
    return 0;
}
