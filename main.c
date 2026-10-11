#include<stdlib.h>
#include<math.h>
#include<ncurses.h>
#include<time.h>
#include<stdio.h>

int main()
{

	const float PI = 3.14;

	const int FRAME_RATE = 60;

	bool running=true;
	
	WINDOW* win = initscr();
	
	noecho();
	nodelay(win, TRUE);
	keypad(win, TRUE);

	int width, height;
	getmaxyx(win, height, width);
	
	char* framebuffer = calloc(height*(width+1), sizeof(char));
	float *zbuffer = calloc(height*width, sizeof(float));
	if(!framebuffer || ! zbuffer)
	{
		printf("Error: Failed to allocate memory for framebuffer");
		running=false;
	}

	const char *CRAMP="#+*";
	const int CRAMP_SIZE = 3;

	const float FOV_ANGLE = 3.14/2;

	const float MAX_RAY_DIST=30;
	const float RAY_STEP = 0.05;

	float player_x=3, player_y=3;
	float player_direction=0.0f;

	float movement_speed=0.1;
	float turning_speed=0.1;

	//ALL FUCKED PART< PLJ FIX
	//char *map = calloc(10, sizeof(char));
	const int map_rows = 14;
	const int map_cols = 14;
	const char map[]=
		"@@@@@@@@@@@@@@\0"
		"@            @\0"
		"@            @\0"
		"@            @\0"
		"@            @\0"
		"@            @\0"
		"@            @\0"
		"@            @\0"
		"@            @\0"
		"@            @\0"
		"@            @\0"
		"@            @\0"
		"@            @\0"
		"@@@@@@@@@@@@@@\0";

	double delta_time=0;
	
	struct timespec start_time, curr_time;
	clock_gettime(CLOCK_MONOTONIC, &start_time);

	bool up_pressed=false, down_pressed=false, left_pressed=false, right_pressed=false;
	
	while(running)
	{
		clock_gettime(CLOCK_MONOTONIC, &curr_time);
		delta_time+=(curr_time.tv_sec - start_time.tv_sec) * 1000.0 + (curr_time.tv_nsec - start_time.tv_nsec) / 1e6;


		//input handling
		int key_pressed = getch();
		while(key_pressed!=ERR)
		{
			if(key_pressed==27) running = false;
			else if(key_pressed==KEY_UP) up_pressed=true;
			else if(key_pressed==KEY_DOWN) down_pressed=true;
			else if(key_pressed==KEY_LEFT) left_pressed=true;
			else if(key_pressed==KEY_RIGHT) right_pressed=true;

			key_pressed = getch();
		}


		//handling resize
		if(is_term_resized)
                {
                        getmaxyx(win, height, width);
                        free(framebuffer);
                        free(zbuffer);
                        framebuffer = calloc(height*(width+1), sizeof(char));
                        zbuffer = calloc(height*width, sizeof(float));
                        if(framebuffer==NULL || zbuffer==NULL)
                        {
                                printf("Failed to resize framebuffer");
                                running=false;
                        }
                }



		if(delta_time>(1000.0/FRAME_RATE))
		{
			//for debigging
			//char str[50];
			//snprintf(str, sizeof(str), "%f", delta_time);
			//mvwaddstr(win, 10, 10, str); wrefresh(win);
			
				
			
				
                        //clear buffer
                        for(int i=0;i<height;i++)
                                for(int j=0;j<width;j++)
                                        {
                                                framebuffer[(i*width+1)+j]=' ';
                                                zbuffer[i*width+j]=INFINITY;
                                        }


			//actual rendering
			for(int i=0;i<width;i++)
			{
				
				float beta = FOV_ANGLE*((float)i/width-0.5);
				double cos_beta = cos(beta); 
				float ray_direction = player_direction + beta;
				
				float ray_dist = 0;
				

				float ray_dir_x=sin(ray_direction)*RAY_STEP, ray_dir_y=cos(ray_direction)*RAY_STEP;
				float ray_x=player_x, ray_y=player_y;
				
				while(ray_dist<MAX_RAY_DIST)
				{
					ray_dist+=RAY_STEP;
					ray_x+=ray_dir_x;
					ray_y+=ray_dir_y;

					int idx=(int)ray_y*(map_cols+1)+ray_x;
					if(idx>=(map_cols+1)*map_rows)
						break;

					if(map[idx]=='@')
					{
						double perp_dist = ray_dist*cos_beta;
						
						//int wh = height*(MAX_RAY_DIST-ray_dist)/MAX_RAY_DIST;
						//int wh = height*(MAX_RAY_DIST-(ray_dist*cos_beta))/MAX_RAY_DIST;
						int wh = height;
						//if(ray_dist>1) wh = height/ray_dist;
						if(perp_dist>1) wh = height/perp_dist;

						for(int j=0;j<wh;j++)
						{
							framebuffer[j*(width+1)+i]=CRAMP[(int)(CRAMP_SIZE*ray_dist/MAX_RAY_DIST)];
						}
						break;
					}
				}
				
			}




                        //present buffer
                        for(int i=0;i<height;i++)
                                mvwaddstr(win, i, 0, framebuffer+i*(width+1));

			wrefresh(win);


			//render the map
			for(int j=0;j<map_rows;j++)
			{
				mvwaddstr(win, j, width-map_cols, map+j*(map_cols+1));
			}

			mvwaddch(win, player_y, width-map_cols+player_x, 'P');
			
			wrefresh(win);
			
			
			//update
			if(up_pressed)
			{
				float delta_x=sin(player_direction)*movement_speed;
				if(map[(int)((int)player_y*(map_cols+1)+player_x+delta_x)]!='@')
					player_x+=delta_x;
				float delta_y=cos(player_direction)*movement_speed;
				if(map[(int)((int)(player_y+delta_y)*(map_cols+1)+player_x)]!='@')
					player_y+=delta_y;

				if(player_y<0) player_y=0;
				if(player_x<0) player_x=0;
				if(player_y>map_rows) player_y=map_rows;
				if(player_x>map_cols) player_x=map_cols;
				up_pressed=false;
			}
			if(down_pressed)
			{
				float delta_x=sin(player_direction)*movement_speed;
				if(map[(int)((int)player_y*(map_cols+1)+player_x-delta_x)]!='@')
					player_x-=delta_x;
				float delta_y=cos(player_direction)*movement_speed;
				if(map[(int)((int)(player_y-delta_y)*(map_cols+1)+player_x)]!='@')
					player_y-=delta_y;
				
				if(player_y<0) player_y=0;
				if(player_x<0) player_x=0;
				if(player_y>map_rows) player_y=map_rows;
				if(player_x>map_cols) player_x=map_cols;
				down_pressed=false;
			}
			if(left_pressed)
			{
				player_direction-=turning_speed;
				if(player_direction<(-1)*2*PI)player_direction+=2*PI;
				left_pressed=false;
			}
			if(right_pressed)
			{
				player_direction+=turning_speed;
				if(player_direction>2*PI)player_direction-=2*PI;
				right_pressed=false;
			}
			


			delta_time-=FRAME_RATE;
		}


		start_time=curr_time;

	}
	if(framebuffer) free(framebuffer);
	if(zbuffer) free(zbuffer);

	endwin();
	return 0;
}
