
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

void clearScreenArea(int startY, int lines)
{
    for (int i = 0; i < lines; ++i)
    {
        gotoXY(0, startY + i);
        cout << string(120, ' ');
    }
}

// ===================== Game constants =====================
const int FIELD_WIDTH = 55;
const int FIELD_HEIGHT = 35;
const int INITIAL_SCORE = 5;
const int TOTAL_ENEMIES = 50;
const int BASE_FRAME_DELAY = 50;

// ===================== Basic structs =====================
struct Bullet
{
    int x;
    int y;
    int dy;
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
                if (x + j == px && y + i == py) return true;
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
        : Plane(FIELD_WIDTH / 2 - 2, FIELD_HEIGHT - 4,
            vector<string>{" /=\\ ", "<<*>>", " * * "})
    {
    }

    void reset()
    {
        x = FIELD_WIDTH / 2 - 2;
        y = FIELD_HEIGHT - 4;
        alive = true;
    }

    void moveLeft()  { if (x > 1) x--; }
    void moveRight() { if (x + width < FIELD_WIDTH - 1) x++; }
    void moveUp()    { if (y > 1) y--; }
    void moveDown()  { if (y + height < FIELD_HEIGHT - 1) y++; }

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
        : Plane(_x, _y, vector<string>{" \\+/ ", "  |", "   "}),
          moveTick(0)
    {
    }

    void update(int speedLevel)
    {
        if (!alive) return;
        moveTick++;
        int moveInterval = max(1, 5 - speedLevel);
        if (moveTick % moveInterval == 0)
        {
            y++;
        }
        if (y > FIELD_HEIGHT - 4)
        {
            alive = false;
        }
    }

    bool canShoot(int speedLevel) const
    {
        if (!alive) return false;
        int probability = 2 + speedLevel; // score rises => harder
        return (rand() % 100) < probability;
    }

    Bullet shoot() const
    {
        return Bullet(x + width / 2, y + height, 1, false);
    }
};

enum GameState
{
    STATE_MENU,
    STATE_HELP,
    STATE_RUNNING,
    STATE_PAUSED,
    STATE_GAME_OVER,
    STATE_EXIT
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
    int playerShootCooldown;
    GameState state;
    bool playerWin;
    bool askRestart;

public:
    Game()
    {
        srand(static_cast<unsigned int>(time(0)));
        HideCursor();
        SetConsoleWindow(86, 48);
        state = STATE_MENU;
        playerWin = false;
        askRestart = false;
        resetRound();
    }

    void resetRound()
    {
        player.reset();
        enemies.clear();
        bullets.clear();
        score = INITIAL_SCORE;
        enemiesSpawned = 0;
        enemiesDestroyed = 0;
        running = true;
        frameCounter = 0;
        playerShootCooldown = 0;
        playerWin = false;
        askRestart = false;
    }

    int getDifficultyLevel() const
    {
        if (score >= 20) return 4;
        if (score >= 15) return 3;
        if (score >= 10) return 2;
        if (score >= 7)  return 1;
        return 0;
    }

    int getSpawnInterval() const
    {
        int level = getDifficultyLevel();
        int interval = 12 - level * 2;
        return max(4, interval);
    }

    void drawMenu()
    {
        system("cls");
        gotoXY(20, 4);  cout << "========================================";
        gotoXY(20, 5);  cout << "         Plane War - Enhanced";
        gotoXY(20, 6);  cout << "========================================";
        gotoXY(20, 9);  cout << "1. Start Game";
        gotoXY(20, 11); cout << "2. Help / Instructions";
        gotoXY(20, 13); cout << "3. Exit";
        gotoXY(20, 17); cout << "Features:";
        gotoXY(23, 19); cout << "- Multi-interface: menu, help, pause, result";
        gotoXY(23, 20); cout << "- Difficulty increases with score";
        gotoXY(23, 21); cout << "- Boundary protection and collision detection";
        gotoXY(23, 22); cout << "- ESC pause / resume";
        gotoXY(20, 26); cout << "Please press 1 / 2 / 3";
    }

    void drawHelp()
    {
        system("cls");
        gotoXY(10, 3);  cout << "================ Game Help ================";
        gotoXY(10, 6);  cout << "Move       : W A S D";
        gotoXY(10, 8);  cout << "Shoot      : Space";
        gotoXY(10, 10); cout << "Pause      : ESC";
        gotoXY(10, 12); cout << "Quit Round : Q";
        gotoXY(10, 15); cout << "Rules:";
        gotoXY(13, 17); cout << "1. Destroy enemy planes to gain score.";
        gotoXY(13, 18); cout << "2. When the player is hit, score -1.";
        gotoXY(13, 19); cout << "3. When score reaches 0, game ends.";
        gotoXY(13, 20); cout << "4. When score rises, enemies fall faster";
        gotoXY(13, 21); cout << "   and appear more frequently.";
        gotoXY(13, 22); cout << "5. Player aircraft cannot fly out of bounds.";
        gotoXY(10, 26); cout << "Press B to go back to menu";
    }

    void drawPause()
    {
        gotoXY(18, FIELD_HEIGHT / 2);
        cout << "********** GAME PAUSED **********";
        gotoXY(18, FIELD_HEIGHT / 2 + 1);
        cout << "Press ESC to continue, Q to end";
    }

    void clearPause()
    {
        gotoXY(18, FIELD_HEIGHT / 2);
        cout << string(40, ' ');
        gotoXY(18, FIELD_HEIGHT / 2 + 1);
        cout << string(40, ' ');
    }

    void spawnEnemy()
    {
        if (enemiesSpawned >= TOTAL_ENEMIES) return;
        int spawnInterval = getSpawnInterval();
        if (frameCounter % spawnInterval == 0)
        {
            int enemyWidth = 5;
            int ex = 1 + rand() % (FIELD_WIDTH - 2 - enemyWidth + 1);
            enemies.push_back(EnemyPlane(ex, 1));
            enemiesSpawned++;
        }
    }

    void processInput()
    {
        if (!_kbhit()) return;

        int ch = _getch();

        if (ch == 27) // ESC
        {
            state = STATE_PAUSED;
            return;
        }

        if (ch == 'a' || ch == 'A') player.moveLeft();
        else if (ch == 'd' || ch == 'D') player.moveRight();
        else if (ch == 'w' || ch == 'W') player.moveUp();
        else if (ch == 's' || ch == 'S') player.moveDown();
        else if (ch == ' ')
        {
            if (playerShootCooldown == 0)
            {
                bullets.push_back(player.shoot());
                playerShootCooldown = 4;
            }
        }
        else if (ch == 'q' || ch == 'Q')
        {
            running = false;
            state = STATE_GAME_OVER;
        }
    }

    void processPauseInput()
    {
        if (!_kbhit()) return;
        int ch = _getch();
        if (ch == 27)
        {
            clearPause();
            state = STATE_RUNNING;
        }
        else if (ch == 'q' || ch == 'Q')
        {
            running = false;
            state = STATE_GAME_OVER;
        }
    }

    void updateEnemies()
    {
        int level = getDifficultyLevel();

        for (size_t i = 0; i < enemies.size(); ++i)
        {
            enemies[i].update(level);

            if (enemies[i].canShoot(level))
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
                                state = STATE_GAME_OVER;
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
                        state = STATE_GAME_OVER;
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
        for (size_t i = 0; i < enemies.size(); ++i) enemies[i].draw(canvas);

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
        cout << "============== Plane War (Enhanced) ==============\n";
        cout << "Move: W A S D   Shoot: Space   Pause: ESC   Quit: Q\n";
        cout << "Score: " << score
             << "   Spawned: " << enemiesSpawned << "/" << TOTAL_ENEMIES
             << "   Destroyed: " << enemiesDestroyed
             << "   Active: " << enemies.size()
             << "   Level: " << getDifficultyLevel() + 1 << "     \n";

        for (int y = 0; y < FIELD_HEIGHT; ++y)
        {
            cout << canvas[y] << "\n";
        }
    }

    void showResult()
    {
        system("cls");
        gotoXY(18, 6);  cout << "=============== Round Result ===============";
        gotoXY(18, 10); cout << "Final Score      : " << score;
        gotoXY(18, 12); cout << "Enemies Spawned  : " << enemiesSpawned;
        gotoXY(18, 14); cout << "Enemies Destroyed: " << enemiesDestroyed;

        if (score <= 0)
            gotoXY(18, 17), cout << "Result: Game Over";
        else if (playerWin)
            gotoXY(18, 17), cout << "Result: You Win";
        else
            gotoXY(18, 17), cout << "Result: Round Ended";

        gotoXY(18, 21); cout << "Press Y to start a new game";
        gotoXY(18, 23); cout << "Press N to return to the main menu";
    }

    void handleResultInput()
    {
        if (!_kbhit()) return;
        int ch = _getch();
        if (ch == 'y' || ch == 'Y')
        {
            resetRound();
            system("cls");
            state = STATE_RUNNING;
        }
        else if (ch == 'n' || ch == 'N')
        {
            state = STATE_MENU;
        }
    }

    void updateRunning()
    {
        frameCounter++;
        processInput();

        if (state != STATE_RUNNING) return;

        spawnEnemy();
        updateEnemies();
        updateBullets();
        drawFrame();

        if (playerShootCooldown > 0) playerShootCooldown--;

        if (score <= 0)
        {
            running = false;
            playerWin = false;
            state = STATE_GAME_OVER;
        }
        else if (win())
        {
            running = false;
            playerWin = true;
            state = STATE_GAME_OVER;
        }
    }

    void run()
    {
        while (state != STATE_EXIT)
        {
            if (state == STATE_MENU)
            {
                drawMenu();
                while (state == STATE_MENU)
                {
                    if (_kbhit())
                    {
                        int ch = _getch();
                        if (ch == '1')
                        {
                            resetRound();
                            system("cls");
                            state = STATE_RUNNING;
                        }
                        else if (ch == '2')
                        {
                            state = STATE_HELP;
                        }
                        else if (ch == '3')
                        {
                            state = STATE_EXIT;
                        }
                    }
                    Sleep(30);
                }
            }
            else if (state == STATE_HELP)
            {
                drawHelp();
                while (state == STATE_HELP)
                {
                    if (_kbhit())
                    {
                        int ch = _getch();
                        if (ch == 'b' || ch == 'B')
                        {
                            state = STATE_MENU;
                        }
                    }
                    Sleep(30);
                }
            }
            else if (state == STATE_RUNNING)
            {
                updateRunning();
                Sleep(BASE_FRAME_DELAY);
            }
            else if (state == STATE_PAUSED)
            {
                drawPause();
                processPauseInput();
                Sleep(60);
            }
            else if (state == STATE_GAME_OVER)
            {
                showResult();
                while (state == STATE_GAME_OVER)
                {
                    handleResultInput();
                    Sleep(30);
                }
            }
        }

        system("cls");
        gotoXY(20, 10);
        cout << "Thanks for playing Plane War!";
        gotoXY(20, 12);
    }
};

int main()
{
    Game game;
    game.run();
    return 0;
}
