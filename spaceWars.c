#include <ncurses.h>
#include <stdlib.h>
 
// Models
// Green //

char spaceShipImage[165] = 
"@____                           \n"
"#\\   \\___________________       \n"
"#}\\   //   //   //  /////\\___|  \n"
"&##\\                          \\ \n"
"#}&#\\__________________________\\\n"; // 33 * 5  = 166(?) - последний это нулл

char stoneImage[60] =
"  _______  \n"
" /-------\\ \n"
"/---------\\\n"
"\\---------/\n"
" \\_______/ \n"; // 12 * 5 = 61(?) последний - это нулл

char bulletImage[5]= "---->"; // 5 * 1
  
// Models


void endGame(), movingStones(), movingPlayer(), draw(), 
collision(), main(), init(), finish(), newBullet();
#define nn 50 // how many 'enemy' we have
#define bb 20 // maximum of bullets
#define bulletMaxLife 150 //idk
short int loops = 0; // loops
long long int score = 0; // score
long long int speed = 0;
long long int bulletsHow = 0; // how many bullets we have on screen

struct spaceShip { int y; int x; };

struct stone { int y; int x; int speed; };

struct bullet { int y; int x; int life; };

struct sprite {int height; int width;};

struct rectangle {int x1, y1, x2, y2; }; //левыый нижний и правый верхний

struct sprite spaceShipSprite;

struct sprite stoneSprite;

struct sprite bulletSprite;

struct spaceShip ourShip;

struct stone stones[nn];

struct bullet bullets[bb];

short int map[1000][1000]; // y*x

void movingPlayer(){
    int k = getch();
    int sp = 3cd ;
    if ( k == 'w' || k == KEY_UP || k == '8' ){
        ourShip.y -= sp;
    }
    else if ( k == 's' || k == KEY_DOWN || k == '2'){
        ourShip.y += sp;
    }
    else if ( k == 'a' || k == KEY_LEFT || k == '4' ){
        ourShip.x -= sp;
    }
    else if ( k == 'd' || k == KEY_RIGHT || k == '6'){
        ourShip.x += sp;
    }
    else if (k == ' '){
        newBullet(ourShip.y+spaceShipSprite.height, ourShip.x+spaceShipSprite.width);
    }
}

void movingStones(){
    for (int i = 0; i<nn; i++){ 
        if (loops%stones[i].speed == 0){
            stones[i].x--;
        }
        if (stones[i].x + stoneSprite.width < 0 && rand()%25 == 0){
            stones[i].x = COLS;
            stones[i].y = rand()%(LINES-stoneSprite.height);
            stones[i].speed = 10 + rand() % 75 - speed;
            if (stones[i].speed < 1){
                stones[i].speed = 1;
            }
        }
    }
}

void movingBullets(){
    for (int i = 0; i<bb; i++){
        if (bullets[i].x != -44){
            bullets[i].x++;
            bullets[i].life++;
            if (bullets[i].life > bulletMaxLife){
                bullets[i].x = -44;
                bullets[i].y = -44;
                bullets[i].life = 0;
            }
        }
    }
}

void newBullet(int y, int x){
    for (int i = 0; i<bb; i++){
        if (bullets[i].x == -44){
            bullets[i].x = x;
            bullets[i].y = y;
            bullets[i].life = 0;
            return;
        }
    }
}

void mvprintwSprite(int y, int x, struct sprite k, char image[]){
    int s = 0;
    for (int i = 0; i<k.height; i++){
        for (int z = i*k.width; z < (i+1)*k.width; z++){
            if (image[z] !='\n' && image[z] !=' ' && image[z] != 0){
                mvaddch(y+s, x+(z%k.width), image[z]);
            }
        }
        s++;
    }
}

void draw(){
    mvprintwSprite(ourShip.y, ourShip.x, spaceShipSprite, spaceShipImage);
    for (int i = 0; i < nn; i++){
        if (stones[i].y < 0 || stones[i].x + stoneSprite.width < 0){
            continue;
        }
        mvprintwSprite(stones[i].y, stones[i].x, stoneSprite, stoneImage);
    }
    for (int i = 0; i < bb; i++){
        if (bullets[i].x == -44){
            continue;
        }
        mvprintwSprite(bullets[i].y, bullets[i].x, bulletSprite, bulletImage);
    }
    mvprintw(0, 0, "Score: %d && Loops: %d && Speed: %d", score, loops, speed);
}

int max(int x, int y){
    if (x>=y){
        return x;
    }
    return y;
}

int min(int x, int y){
    if (x<=y){
        return x;
    }
    return y;
}

struct rectangle rectanglesIntersection(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4){ 
    // x1, y1 - левый нижний первого
    // левый нижний и правый верхний
    struct rectangle answer; 
    answer.x1 = -44; answer.y1 = -44; answer.x2 = -44; answer.y2 = -44;
    int x5 = max(x1, x3);
    int y5 = max(y1, y3);
    int x6 = min(x2, x4);
    int y6 = min(y2, y4);
    if (x5 > x6 || y5 > y6){
        return answer;
    }
    answer.x1 = x5; answer.y1 = y5; answer.x2 = x6; answer.y2 = y6;
    return answer;
}

void collision(){
    if (ourShip.y < 0 || ourShip.x < 0 || ourShip.y + spaceShipSprite.height - 1 >= LINES 
    || ourShip.x + spaceShipSprite.width - 2 >= COLS){ // коллизия игрока с краями экрана
        endGame();
        return;
    }
    for (int i = 0; i < nn; i++){ // коллизия КАМней друг с другом
        for (int k = 0; k < nn; k++){
            if (stones[i].x + stoneSprite.width < 0 || stones[i].y < 0){
                break;
            }
            if (stones[i].x + stoneSprite.width < 0 || stones[k].y < 0 || k==i){
                continue;
            }
            int x1, x2, x3, x4, y1, y2, y3, y4;
            x1 = stones[k].x; y1 = stones[k].y;
            x2 = stones[k].x + stoneSprite.width - 1; y2 = stones[k].y + stoneSprite.height - 1;
            x3 = stones[i].x; y3 = stones[i].y;
            x4 = stones[i].x + stoneSprite.width - 1; y4 = stones[i].y + stoneSprite.height - 1;
            struct rectangle ans = rectanglesIntersection(x1, y1, x2, y2, x3, y3, x4, y4);
            if (ans.x1 != -44){
                if (stones[k].speed <= stones[i].speed){
                    stones[i].x = -10;
                    stones[i].y = -10;
                    break;
                }
                stones[k].x = -10; stones[k].y = -10;
            }
        }
    }

    for(int i = 0; i < bb; i++){ // коллизия пулей с кем-либо. пока только камни
        if (bullets[i].x == -44){
            continue;
        }
        for (int b = 0; b<nn; b++){
            if (stones[b].x + stoneSprite.width < 0){
                continue;
            }
            int x1, x2, x3, x4, y1, y2, y3, y4;
            x1 = stones[b].x; y1 = stones[b].y;
            x2 = stones[b].x + stoneSprite.width - 1; y2 = stones[b].y + stoneSprite.height - 1;
            x3 = bullets[i].x; y3 = bullets[i].y;
            x4 = bullets[i].x + bulletSprite.width - 1; y4 = bullets[i].y + bulletSprite.height - 1;
            struct rectangle ans = rectanglesIntersection(x1, y1, x2, y2, x3, y3, x4, y4);
            if (ans.x1 != -44){
                bullets[i].x = -44;
                stones[b].x = -10;
                break;
            }
        }
    }

    for (int i = 0; i < nn; i++){ // коллизия игрока с CUMнями
        if (stones[i].y < 0 || stones[i].x + stoneSprite.width < 0){
            continue;
        }
        int x1, x2, x3, x4, y1, y2, y3, y4;
        x1 = ourShip.x; y1 = ourShip.y;
        x2 = ourShip.x + spaceShipSprite.width - 1 ; y2 = ourShip.y + spaceShipSprite.height - 1;
        x3 = stones[i].x; y3 = stones[i].y;
        x4 = stones[i].x + stoneSprite.width - 1; y4 = stones[i].y + stoneSprite.height - 1;
        struct rectangle ans = rectanglesIntersection(x1, y1, x2, y2, x3, y3, x4, y4);
        
        if (ans.x1 != -44){
            for( int p = ans.y1; p<=ans.y2; p++){
                for (int k = ans.x1; k<=ans.x2; k++){
                    // i, k - y,x
                    int ss = (p - ourShip.y) * spaceShipSprite.width + k - ourShip.x;
                    int st = (p - stones[i].y) * stoneSprite.width + k - stones[i].x;
                    if ( spaceShipImage[ss] !='\n' && spaceShipImage[ss] != ' ' &&
                    stoneImage[st] != '\n' && stoneImage[st] != ' '){
                        endGame();
                        return;
                    }
                }
            }    
        }
    }
}

void endGame(){
    mvaddch(ourShip.y, ourShip.x, '&');
    mvprintw(LINES/2, COLS/2 - 20, "You're lose. What a pathetic person 0/");
    refresh();
    napms(7500);
    finish();
}

void init(){
    initscr();
    savetty();
    noecho(); // idk
    curs_set(0); // no cursor
    keypad(stdscr, 1); // can use arrows etc
    timeout(0); // getch - no waiting
}

void finish(){
    curs_set(1);
    clear();
    refresh();
    resetty();
    endwin();
    exit(0);
}

void main(){
    init();
    //short int map[LINES][COLS]; // массив объектов
    // for (int i = 0; i<LINES; i++)
    //     for (int k = 0; k < COLS; k++)
    //         map[i][k] = NULL;
    
    for (int i = 0; i<nn; i++){
        stones[i].y = -10;
        stones[i].x = -10;
        stones[i].speed = 1;
    } // stones initialisation.

    for (int i = 0; i<bb; i++){
        bullets[i].y = -44;
        bullets[i].x = -44;
        bullets[i].life = 0;
    } // bullets initialisation.
    
    ourShip.y = LINES/2; ourShip.x = 1;
    spaceShipSprite.height = 5; spaceShipSprite.width = 33; 
    stoneSprite.height = 5; stoneSprite.width = 12;
    bulletSprite.height = 1; bulletSprite.width = 5;

    while(true){ 
        movingStones();
        movingPlayer();
        if (loops%1 == 0){
            movingBullets();
        }
        
        draw();
        refresh();
        
        collision();
        napms(10);
        loops += 1;
        if (loops%500 == 0){
            score++;
            if (score%1 == 0){
                speed++;
            }
        }
        
        erase();
    }
}
