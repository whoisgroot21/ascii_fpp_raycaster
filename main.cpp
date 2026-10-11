#include<iostream>
#include<cmath>
#include<chrono>
#include<array>
#include<curses.h>

const int screenWidth = 100;
const int screenHeight = 25;

const float FOVAngle=3.14/2;

bool running;


const int mapHeight = 6;
const int mapWidth = 6;
char map[mapHeight][mapWidth]=
{
	{'@', '@', '@', '@', '@', '@'},
	{'@', ' ', ' ', ' ', ' ', '@'},
	{'@', ' ', ' ', ' ', ' ', '@'},
	{'@', ' ', ' ', ' ', ' ', '@'},
	{'@', ' ', ' ', ' ', ' ', '@'},
	{'@', '@', '@', '@', '@', '@'}
};

/*{
	{'@', '@', '@', '@', '@', '@'},
	{'@', ' ', ' ', ' ', '@', '@'},
	{'@', '@', ' ', ' ', ' ', '@'},
	{'@', ' ', ' ', '@', ' ', '@'},
	{'@', ' ', ' ', '@', ' ', '@'},
	{'@', '@', '@', '@', '@', '@'}
};*/

struct vec2
{
	float x, y;
};

void keyEventHandler(int key, vec2& player_pos, float &player_angle, float delta_time);
void render(char framebuffer[screenHeight][screenWidth], WINDOW* win);
void update(char framebuffer[screenHeight][screenWidth], vec2 player_pos, float player_angle);
void clear_buffer(char framebuffer[screenHeight][screenWidth]);

using namespace std::chrono_literals;

float time_step = 0.016f;
constexpr std::chrono::nanoseconds timestep(16ms); //60FPS

int main()
{

	using clock = std::chrono::steady_clock;
	//using clock = std::chrono::high_resolution_clock;

	//std::chrono::time_point time_start;
	auto time_start = clock::now();
	std::chrono::nanoseconds lag(0ns);

	int winx, winy;
	WINDOW* mainWin, *gameWin;
	mainWin = initscr();
	getmaxyx(mainWin, winy, winx);

	vec2 player_pos = {2.0f, 4.0f};
	
	float player_angle = 0.0f;
	//vec2 player_direction{0.0f, -1.0f};

	char framebuffer[screenHeight][screenWidth];

	if(winy>screenHeight && winx>screenWidth)
	{
		gameWin = subwin(mainWin, screenHeight, screenWidth, (winy-screenHeight)/2, (winx-screenWidth)/2);
		//box(gameWin, 0, 0);
		keypad(initscr(), TRUE);
		nodelay(initscr(), TRUE);
		noecho();
		running = true;
		for (int i = 0;i<screenHeight;i++)
		{
			for (int j = 0;j<screenWidth;j++)
				framebuffer[i][j] = ' ';
		}

		time_start = clock::now();
	}
	else
	{
		running = false;
		endwin();
		std::cerr << "console window too small";
	}
	while(running)
	{

		auto time_now = clock::now();
		auto delta_time = time_now - time_start;
		time_start = time_now;
		lag+=delta_time;
		//if(delta_time<0.1) continue;
		
		//while(lag>= timestep)
		if(lag>= timestep)
		{
			lag-=timestep;

			int keyPressed = getch();
			if(keyPressed!=ERR) 
				keyEventHandler(keyPressed, player_pos, player_angle, std::chrono::duration<float>(timestep).count());
				//or std::chrono::duration<float>(delta_time).count());

			clear_buffer(framebuffer);
			update(framebuffer, player_pos, player_angle);
			render(framebuffer, gameWin);
		}
		//render(framebuffer, gameWin);

		//if resize just exit(no risize handling)
	}

	endwin();
	return 0;
}

void render(char framebuffer[screenHeight][screenWidth], WINDOW* win)
{
	//place the map on top
	for (int i=0;i<mapHeight;i++)
	{
		for (int j=0;j<mapWidth;j++)
		{
			framebuffer[i][screenWidth+j-6]=map[i][j];
		}
	}

	//render the framebuffer
	for (int i=0;i<screenHeight;i++)
	{
		for (int j=0;j<screenWidth;j++)
			mvwaddch(win, i, j, framebuffer[i][j]);
	}

	wrefresh(win);
}

void update(char framebuffer[screenHeight][screenWidth], vec2 player_pos, float player_angle)
{
	static vec2 previous_pos(player_pos);

	//cast a ray for every pixel(here character) in window
	for (int x=0;x<screenWidth;x++)
	{
		vec2 direction;
		
		//method 1
		
		float angle = (player_angle-(FOVAngle/2)) + (FOVAngle/screenWidth)*x;
		direction.x = sinf(angle);
		direction.y = cosf(angle);
		
		
		//method 2 
		//In this method we dont use angles hence no need for angle or player_angle
		//Instead we have tofocal which is a direction vector that goes straight to scree
		//to rotate the player we change tofocal
		/*
		float NDC = (2*x)/screenWidth -1;
		float magnitude = sqrt((player_direction.x+NDC)*(player_direction.x+NDC)+(player_direction.y)*(player_direction.y));
		direction.x = (player_direction.x+NDC)/magnitude;
		direction.y = (player_direction.y)/magnitude;
		*/

		vec2 ray = {0.0f, 0.0f};
		
		bool hitWall = false;
		while(!hitWall && ray.x<10 && ray.y<10 )
		{
			ray.x += direction.x*0.1f;
			ray.y += direction.y*0.1f;

			int testx = player_pos.x+ray.x;
			int testy = player_pos.y-ray.y;
			if(map[(int)testy][(int)testx]=='@')
				hitWall=true;
		}
		if (hitWall)
		{
			//calculate the distance in x and y by
			//euclidean distance formula 
			//actual distance is gonna give fish eye effect
			//float distance = sqrt((ray.x)*(ray.x) + (ray.y)*(ray.y));
			
			//instead directly compare with y
			float distance = sqrt(ray.y*ray.y);

			char shade = ' ';

			/*if((int)distance<2) shade = '#';
			else if((int)distance<3) shade = '*';
			else shade = '.';*/

			if((int)distance<3) shade = '#';
			else shade='*';

			int wallSize = (float)screenHeight/(float)distance;

			if (wallSize>screenHeight)
				wallSize=screenHeight;	
			
			for (int p=0;p<wallSize;p++)
				framebuffer[p][x]=shade;
		}
		else
			framebuffer[10][x]=' ';
	}
	map[(int)previous_pos.y][(int)previous_pos.x] = ' ';
	map[(int)player_pos.y][(int)player_pos.x] = 'P';
	previous_pos = player_pos;
	framebuffer[screenHeight/2][screenWidth/2]='+';
}

void keyEventHandler(int key, vec2& player_pos, float &player_angle, float delta_time)
{
	if(key==27)
		running=false;

	if(key==KEY_UP)
	{
		vec2 dir ={sinf(player_angle), cosf(player_angle)};
		if(map[(int)player_pos.y][(int)(player_pos.x+1)]!=' ' && dir.x>0) dir.x=0;
		if(map[(int)player_pos.y][(int)(player_pos.x-1)]!=' ' && dir.x<0) dir.x=0;
		if(map[(int)(player_pos.y+1)][(int)player_pos.x]!=' ' && dir.y<0) dir.y=0;
		if(map[(int)(player_pos.y-1)][(int)player_pos.x]!=' ' && dir.y>0) dir.y=0;
		
		player_pos.y-=dir.y*10*delta_time;
		player_pos.x+=dir.x*10*delta_time;
		//player_pos.y-=dir.y*0.1;
		//player_pos.x+=dir.x*0.1;
	}
	if(key==KEY_DOWN)
	{
		vec2 dir ={sinf(player_angle), cosf(player_angle)};
		if(map[(int)player_pos.y][(int)(player_pos.x-1)]!=' ' && dir.x>0) dir.x=0;
		if(map[(int)player_pos.y][(int)(player_pos.x+1)]!=' ' && dir.x<0) dir.x=0;
		if(map[(int)(player_pos.y-1)][(int)player_pos.x]!=' ' && dir.y<0) dir.y=0;
		if(map[(int)(player_pos.y+1)][(int)player_pos.x]!=' ' && dir.y>0) dir.y=0;
		player_pos.y +=dir.y*10*delta_time;
		player_pos.x -+dir.x*10*delta_time;
		//player_pos.y+=dir.y*0.1;
		//player_pos.x-=dir.x*0.1;
	}
	if(key==KEY_LEFT)
		player_angle-=0.1 ;//1000*delta_time;
	if(key==KEY_RIGHT)
		player_angle+=0.1; //1000*delta_time;
}

void clear_buffer(char framebuffer[screenHeight][screenWidth])
{
	for (int i=0; i<screenHeight;i++)
	{
		for (int j=0; j<screenWidth;j++)
			framebuffer[i][j]=' ';
	}
}