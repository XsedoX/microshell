#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>

#define BUFFER_SIZE 2048

int amount=1;
int i=0;
char previous_dir[2048]={};

void red()
{
	printf("\033[1;31m");
}
void green()
{
	printf("\033[1;32m");
}
void yellow()
{
	printf("\033[1;33m");
}
void blue()
{
	printf("\033[1;34m");
}
void purple()
{
	printf("\033[1;35m");
}
void cyan()
{
	printf("\033[1;36m");
}
void reset()
{
	printf("\033[0m");
}

void help()
{
	printf("\n\033[1;32m///////////\033[1;31mMICROSHELL\033[1;32m////////////\n");
	printf("\033[1;32m//////\033[1;36m*\033[1;34mJuliusz Sadowski\033[1;36m*\033[1;32m/////////\n");
	reset();
	printf("\nOBSLUGIWANE KOMEDY:\n");
	printf("\033[1;35mcd\033[0m - wlacznie z ~, -.\n");
	printf("\033[1;33mcp\033[0m - 3 flagi\n");
	printf("	-u, -r, -i\n");
	printf("	brak obslugi flag laczonych\n");
	printf("\033[1;36mls\033[0m - 3 flagi\n");
	printf("	-a, -l, -m\n");
	printf("	-l niestety nie pokazuje dowiazan\n");
	printf("	flagi laczone -al, -am\n");
	printf("Wpisanie innej komendy niz powyzsze powoduje uruchomienie wpisanej komendy poprzez exec.\n");
	reset();
}

int run_exec(char **komenda)
{
	int ERR = execvp(komenda[0], komenda);
	
	return ERR;
}

int cd(char **komenda)
{
	char *home_dir=getenv("HOME"), komenda_bez_tyldy[2048]={}, previous_previous_dir[2048]={};
	int ERR=0;


	
	if(home_dir!=NULL)
	{
		if((komenda[1]==NULL) || (strcmp(komenda[1], "~")==0))
		{
			getcwd(previous_dir, sizeof(previous_dir));	
			ERR = chdir(home_dir);
		}
		else if(strcmp(komenda[1], "-")==0)
		{
			getcwd(previous_previous_dir, sizeof(previous_previous_dir));
			ERR = chdir(previous_dir);
			strcpy(previous_dir, previous_previous_dir);
		}
			
		else if(komenda[1][0]==126)/*~*/
		{
			getcwd(previous_dir, sizeof(previous_dir));	
			chdir(home_dir);

			for(i=2; komenda[1][i]>0; i++)
				komenda_bez_tyldy[i-2]=komenda[1][i];

			ERR = chdir(komenda_bez_tyldy);
		}
		else
		{
			getcwd(previous_dir, sizeof(previous_dir));	
			ERR = chdir(komenda[1]);
		}
	}

	return ERR;
}

char **read_command()
{
	char text[2048]={}, c=0;
	int letter=0, command_letter=0;
	char **command=NULL;
	amount = 1;
	
	fgets(text, sizeof(text), stdin);
	
	
	while((c=text[letter])>0)
	{
		if(c==32)
			amount++;
			
		letter++;
	}
	
	command = calloc(amount+1, sizeof(char*));
	
	for(i=0; i<=amount; i++)
		command[i]=calloc(BUFFER_SIZE, sizeof(char));
		
	letter=0;
		
	for(i=0; i<amount; i++)
	{
		command_letter=0;
		while((c=text[letter])>0 && c!=32 && c!=10)
		{		
			command[i][command_letter]=c;
			letter++;
			command_letter++;
		}
		letter++;
	}
		
	command[amount]=NULL;
	
	return command;
}

void cp_r(char file_in[2048], char file_out[2048])
{
	DIR *folder_from;
	char file_data[BUFFER_SIZE];
	struct dirent *entry;
	char file_from_path[2048]={}, file_to_path[2048]={};
	struct stat file_from_info;
	mode_t spec_file_info;
	int file_from=0, file_to=0, bytes=0;

	mkdir(file_out, 0777);
	if((folder_from = opendir(file_in))!=NULL)
	{
		while((entry = readdir(folder_from)) != NULL)
		{
			strcpy(file_from_path, file_in);
			strcat(file_from_path, entry->d_name);
			strcpy(file_to_path, file_out);
			strcat(file_to_path, entry->d_name);

			if(stat(file_from_path, &file_from_info)!=-1)
			{

				spec_file_info=file_from_info.st_mode;

				if(S_ISREG(spec_file_info) && (strcmp(entry->d_name, ".")!=0) && (strcmp(entry->d_name, "..")!=0))/*file*/
				{
					file_from =  open(file_from_path, O_RDONLY);
					file_to =  open(file_to_path, O_WRONLY|O_CREAT|O_TRUNC, 0666);

					while((bytes = read(file_from, &file_data, BUFFER_SIZE))>0)
						write(file_to, &file_data, bytes);
					
					close(file_to);
					close(file_from);
				}
				else if(S_ISDIR(spec_file_info) && (strcmp(entry->d_name, ".")!=0) && (strcmp(entry->d_name, "..")!=0))/*DIR*/
				{
					strcat(file_from_path, "/");
					strcat(file_to_path, "/");

					cp_r(file_from_path, file_to_path);
				}
			}
		}
	}
	else
	{
		reset();
		perror("Komunikat bledu\033[1;31m");
		reset();
		printf("Numer bledu: \033[1;31m%d\n", errno);
		reset();
	}
	closedir(folder_from);
}

bool does_exist(char path[2048])
{
	struct stat file_to_check;
	return (stat(path, &file_to_check) == 0);
}

int cp(char **komenda)
{
	char file_data[BUFFER_SIZE], overwrite[32]={}, file_from_month[4]={}, file_to_month[4]={};
	char *token=NULL, file_from_date_time_string[32]={}, file_to_date_time_string[32]={}, file_from_time_string[32]={}, file_to_time_string[32]={};
	int file_from=0, file_to=0, bytes=0, file_to_time[3], file_from_time[3], file_to_date[2], file_from_date[2], file_to_month_n=0, file_from_month_n=0, length=0;
	struct stat file_from_info;
	struct stat file_to_info;
	bool is_newer=true;


	if(komenda[1][0]==45)/*czy myslnik*/
	{

		if(komenda[1][1]==114 && komenda[1][2]==0)/*r*/
		{
			length = strlen(komenda[2]);
			if(komenda[2][length-1]!=47)/* / */
				komenda[2][length]=47;

			length=strlen(komenda[3]);
			if(komenda[3][length-1]!=47)
				komenda[3][length]=47;

			cp_r(komenda[2], komenda[3]);
		}
		else if(komenda[1][1]==117 && komenda[1][2]==0)/*u*/
		{
			if(!does_exist(komenda[3]))
				is_newer=true;
			else
			{
				if(stat(komenda[2], &file_from_info)==-1 || stat(komenda[3], &file_to_info)==-1)
					return -1;
				else
				{
					strcpy(file_from_date_time_string, ctime(&file_from_info.st_mtime));
					strcpy(file_to_date_time_string, ctime(&file_to_info.st_mtime));

					token=strtok(file_from_date_time_string, " ");
					token=strtok(NULL, " ");

					strcpy(file_from_month, token);

					token=strtok(NULL, " ");

					file_from_date[0]=atoi(token);

					token=strtok(NULL, " ");

					strcpy(file_from_time_string, token);
					token=strtok(NULL, " ");

					file_from_date[1]=atoi(token);

					token=strtok(file_from_time_string, ":");

					for(i=0; token!=NULL; i++)
					{
						file_from_time[i]=atoi(token);
						token = strtok(NULL, ":");
					}

					token=strtok(file_to_date_time_string, " ");
					token=strtok(NULL, " ");

					strcpy(file_to_month, token);

					token=strtok(NULL, " ");

					file_to_date[0]=atoi(token);

					token=strtok(NULL, " ");

					strcpy(file_to_time_string, token);
					token=strtok(NULL, " ");

					file_to_date[1]=atoi(token);

					token=strtok(file_to_time_string, ":");

					for(i=0; token!=NULL; i++)
					{
						file_to_time[i]=atoi(token);
						token = strtok(NULL, ":");
					}

					free(token);

					if(strcmp(file_from_month, "Jan")==0)
						file_from_month_n=1;
					else if(strcmp(file_from_month, "Feb")==0)
						file_from_month_n=2;
					else if(strcmp(file_from_month, "Mar")==0)
						file_from_month_n=3;
					else if(strcmp(file_from_month, "Apr")==0)
						file_from_month_n=4;
					else if(strcmp(file_from_month, "May")==0)
						file_from_month_n=5;
					else if(strcmp(file_from_month, "Jun")==0)
						file_from_month_n=6;
					else if(strcmp(file_from_month, "Jul")==0)
						file_from_month_n=7;
					else if(strcmp(file_from_month, "Aug")==0)
						file_from_month_n=8;
					else if(strcmp(file_from_month, "Sep")==0)
						file_from_month_n=9;
					else if(strcmp(file_from_month, "Oct")==0)
						file_from_month_n=10;
					else if(strcmp(file_from_month, "Nov")==0)
						file_from_month_n=11;
					else if(strcmp(file_from_month, "Dec")==0)
						file_from_month_n=12;

					if(strcmp(file_to_month, "Jan")==0)
						file_to_month_n=1;
					else if(strcmp(file_to_month, "Feb")==0)
						file_to_month_n=2;
					else if(strcmp(file_to_month, "Mar")==0)
						file_to_month_n=3;
					else if(strcmp(file_to_month, "Apr")==0)
						file_to_month_n=4;
					else if(strcmp(file_to_month, "May")==0)
						file_to_month_n=5;
					else if(strcmp(file_to_month, "Jun")==0)
						file_to_month_n=6;
					else if(strcmp(file_to_month, "Jul")==0)
						file_to_month_n=7;
					else if(strcmp(file_to_month, "Aug")==0)
						file_to_month_n=8;
					else if(strcmp(file_to_month, "Sep")==0)
						file_to_month_n=9;
					else if(strcmp(file_to_month, "Oct")==0)
						file_to_month_n=10;
					else if(strcmp(file_to_month, "Nov")==0)
						file_to_month_n=11;
					else if(strcmp(file_to_month, "Dec")==0)
						file_to_month_n=12;


					if(file_from_date[1]<file_to_date[1])
						is_newer=false;
					else if(file_from_date[1]==file_to_date[1])
					{
						if(file_from_month_n<file_to_month_n)
							is_newer=false;
						else if(file_from_month_n==file_to_month_n)
						{
							if(file_from_date[0]<file_to_date[0])
								is_newer=false;
							else if(file_from_date[0]==file_to_date[0])
							{
								if(file_from_time[0]<file_to_time[0])
									is_newer=false;
								else if(file_from_time[0]==file_to_time[0])
								{
									if(file_from_time[1]<file_to_time[1])
										is_newer=false;
									else if(file_from_time[1]==file_to_time[1])
									{
										if(file_from_time[2]<=file_to_time[2])
											is_newer=false;
									}
								}
							}
						}
					}
				}
			}

			if(is_newer)
			{
				file_from = open(komenda[2], O_RDONLY);

				file_to = open(komenda[3], O_WRONLY|O_CREAT|O_TRUNC, 0666);

				if(file_from==-1 || file_to==-1)
					return -1;

				while((bytes = read(file_from, &file_data, BUFFER_SIZE))>0)
					write(file_to, &file_data, bytes);
				

				close(file_to);
				close(file_from);

				return 0;
				
			}
		}
		else if(komenda[1][1]==105 && komenda[1][2]==0)/*i*/
		{


			file_from = open(komenda[2], O_RDONLY);


			if(access(komenda[3], F_OK)==0)
			{
				printf("cp: overwrite \"%s\"? [\033[1;34my\033[0m(\033[1;34myes\033[0m):\033[1;32mn\033[0m(\033[1;32mno\033[0m)]\n", komenda[3]);
				fgets(overwrite, sizeof(overwrite), stdin);
				if(strchr(overwrite, 121)!=NULL)
				{
					file_to = open(komenda[3], O_WRONLY|O_CREAT|O_TRUNC, 0666);

					if(file_from==-1 || file_to==-1)
						return -1;

					while((bytes = read(file_from, &file_data, BUFFER_SIZE))>0)
						write(file_to, &file_data, bytes);

					close(file_to);
					close(file_from);

					
					return 0;
				}
				else
					return 0;
			}
			else
			{

				file_to = open(komenda[3], O_WRONLY|O_CREAT|O_TRUNC, 0666);

				if(file_from==-1 || file_to==-1)
					return -1;

				while((bytes = read(file_from, &file_data, BUFFER_SIZE))>0)
					write(file_to, &file_data, bytes);
				
				close(file_to);
				close(file_from);

				return 0;
			}
		}
		else 
		{
			red();
			printf("Microshell obsluguje tylko 3 flagi: -r, -u, -i. Nie obsluguje laczenia tych flag.\n");
			reset();
		}
	}
	else
	{
		file_from = open(komenda[1], O_RDONLY);

		file_to = open(komenda[2], O_WRONLY|O_CREAT|O_TRUNC, 0666);

		if(file_from==-1 || file_to==-1)
			return -1;

		while((bytes = read(file_from, &file_data, BUFFER_SIZE))>0)
			write(file_to, &file_data, bytes);
		
		close(file_from);
		close(file_to);

		
		return 0;
	}

	return 0;
}

int ls(char **komenda)
{
	char path[2048], name[256], type, usr_r, usr_w, usr_x, grp_r, grp_w, grp_x, oth_r, oth_w, oth_x, owner[1024]={}, owner_group[1024]={}, date_time[32]={};
	DIR *folder_from;
	off_t size;
	struct dirent *entry;
	mode_t spec_file_info;
	struct stat file_info;
	struct group *grp_info;
	struct passwd *usr_info;
	uid_t usr_id;
	gid_t grp_id;
	char *token=NULL;

	getcwd(path, sizeof(path));

	if(komenda[1]!=NULL && komenda[1][0]==45)/*czy myslnik*/
	{
		
		if(strchr(komenda[1], 97)!=NULL && strchr(komenda[1], 108)!=NULL && komenda[1][3]==0)/*al*/
		{
			if((folder_from = opendir(path))==NULL)
				return -1;
			else
			{
				printf("\033[1;33mTYPE/PERM  \033[1;34mOWNER      \033[1;35mGROUP       \033[0mSIZE      \033[1;36mMODIFICATION DATE        \033[1;32mNAME\n");
				reset();
				while((entry = readdir(folder_from))!=NULL)
				{
					strcpy(name, entry->d_name);
					if(stat(name, &file_info)==-1)
						continue;
					else
					{
						spec_file_info=file_info.st_mode;

						if(S_ISLNK(spec_file_info))/*link*/
							type=108;/*l*/
						else if(S_ISREG(spec_file_info))/*file*/
							type=45;/*-*/
						else if(S_ISDIR(spec_file_info))/*dir*/
							type=100;/*d*/

						if(spec_file_info & S_IRUSR)/*usr_r*/
							usr_r=114;/*r*/
						else
							usr_r=45;/*-*/
						if(spec_file_info & S_IWUSR)/*usr_w*/
							usr_w=119;/*w*/
						else
							usr_w=45;/*-*/
						if(spec_file_info & S_IXUSR)/*usr_x*/
							usr_x=120;/*x*/
						else
							usr_x=45;/*-*/

						if(spec_file_info & S_IRGRP)/*grp_r*/
							grp_r=114;/*r*/
						else
							grp_r=45;/*-*/
						if(spec_file_info &&S_IWGRP)/*grp_w*/
							grp_w=119;/*w*/
						else
							grp_w=45;/*-*/
						if(spec_file_info &&S_IXGRP)/*grp_x*/
							grp_x=120;/*x*/
						else
							grp_x=45;/*-*/

						if(spec_file_info & S_IROTH)/*oth_r*/
							oth_r=114;/*r*/
						else
							oth_r=45;/*-*/
						if(spec_file_info & S_IWOTH)/*oth_w*/
							oth_w=119;/*w*/
						else
							oth_w=45;/*-*/
						if(spec_file_info & S_IXOTH)/*oth_x*/
							oth_x=120;/*x*/
						else
							oth_x=45;/*-*/

						usr_id = file_info.st_uid;
						grp_id = file_info.st_gid;


						if((usr_info=getpwuid(usr_id))==NULL)
							continue;
						else
							strcpy(owner, usr_info->pw_name);

						if((grp_info=getgrgid(grp_id))==NULL)
							continue;
						else
							strcpy(owner_group, grp_info->gr_name);

						size = file_info.st_size;

						strcpy(date_time, ctime(&file_info.st_mtime));

						token = strtok(date_time, "\n");
						token = strtok(NULL, "\n");
						free(token);

						printf("\033[1;33m%c%c%c%c%c%c%c%c%c%c \033[1;34m%10s \033[1;35m%10s \033[0m%10li \033[1;36m%s \033[1;32m%s\n", type, usr_r, usr_w, usr_x, grp_r, grp_w, grp_x, oth_r, oth_w, oth_x, owner, owner_group, size, date_time, name);
						reset();

					}
				}
				closedir(folder_from);
				return 0;
			}
		}
		else if(strchr(komenda[1], 97)!=NULL && strchr(komenda[1], 109)!=NULL && komenda[1][3]==0)/*am*/
		{
			if((folder_from = opendir(path))==NULL)
				return -1;
			else
			{
				entry = readdir(folder_from);
				strcpy(name, entry->d_name);
				printf("\033[1;32m%s", name);

				while((entry = readdir(folder_from))!=NULL)
				{
					strcpy(name, entry->d_name);
					printf("\033[0m, \033[1;32m%s", name);
					
				}
				reset();
				printf("\n");

			}
			closedir(folder_from);
			return 0;
		}
		else if(komenda[1][1]==109 && komenda[1][2]==0)/*m*/
		{
			if((folder_from = opendir(path))==NULL)
				return -1;
			else
			{
				entry = readdir(folder_from);
				strcpy(name, entry->d_name);
				if(name[0]!=46)
						printf("\033[1;32m%s", name);

				while((entry = readdir(folder_from))!=NULL)
				{
					strcpy(name, entry->d_name);
					if(name[0]!=46)
						printf("\033[0m, \033[1;32m%s", name);
					
				}
				reset();
				printf("\n");

			}
			closedir(folder_from);
			return 0;
		}
		else if(komenda[1][1]==108 && komenda[1][2]==0)/*l*/
		{
			if((folder_from = opendir(path))==NULL)
				return -1;
			else
			{
				printf("\033[1;33mTYPE/PERM  \033[1;34mOWNER      \033[1;35mGROUP       \033[0mSIZE      \033[1;36mMODIFICATION DATE        \033[1;32mNAME\n");
				reset();
				while((entry = readdir(folder_from))!=NULL)
				{
					strcpy(name, entry->d_name);

					if(name[0]==46)/*.*/
						continue;
					if(stat(name, &file_info)==-1)
						continue;
					else
					{
						spec_file_info=file_info.st_mode;

						if(S_ISLNK(spec_file_info))/*link*/
							type=108;/*l*/
						else if(S_ISREG(spec_file_info))/*file*/
							type=45;/*-*/
						else if(S_ISDIR(spec_file_info))/*dir*/
							type=100;/*d*/

						if(spec_file_info & S_IRUSR)/*usr_r*/
							usr_r=114;/*r*/
						else
							usr_r=45;/*-*/
						if(spec_file_info & S_IWUSR)/*usr_w*/
							usr_w=119;/*w*/
						else
							usr_w=45;/*-*/
						if(spec_file_info & S_IXUSR)/*usr_x*/
							usr_x=120;/*x*/
						else
							usr_x=45;/*-*/

						if(spec_file_info & S_IRGRP)/*grp_r*/
							grp_r=114;/*r*/
						else
							grp_r=45;/*-*/
						if(spec_file_info & S_IWGRP)/*grp_w*/
							grp_w=119;/*w*/
						else
							grp_w=45;/*-*/
						if(spec_file_info & S_IXGRP)/*grp_x*/
							grp_x=120;/*x*/
						else
							grp_x=45;/*-*/

						if(spec_file_info & S_IROTH)/*oth_r*/
							oth_r=114;/*r*/
						else
							oth_r=45;/*-*/
						if(spec_file_info & S_IWOTH)/*oth_w*/
							oth_w=119;/*w*/
						else
							oth_w=45;/*-*/
						if(spec_file_info & S_IXOTH)/*oth_x*/
							oth_x=120;/*x*/
						else
							oth_x=45;/*-*/

						usr_id = file_info.st_uid;
						grp_id = file_info.st_gid;


						if((usr_info=getpwuid(usr_id))==NULL)
							continue;
						else
							strcpy(owner, usr_info->pw_name);

						if((grp_info=getgrgid(grp_id))==NULL)
							continue;
						else
							strcpy(owner_group, grp_info->gr_name);

						size = file_info.st_size;

						strcpy(date_time, ctime(&file_info.st_mtime));

						token = strtok(date_time, "\n");
						token = strtok(NULL, "\n");
						free(token);
						printf("\033[1;33m%c%c%c%c%c%c%c%c%c%c \033[1;34m%10s \033[1;35m%10s \033[0m%10li \033[1;36m%s \033[1;32m%s\n", type, usr_r, usr_w, usr_x, grp_r, grp_w, grp_x, oth_r, oth_w, oth_x, owner, owner_group, size, date_time, name);
						reset();

					}
				}
				closedir(folder_from);
				return 0;
			}
		}
		else if(komenda[1][1]==97 && komenda[1][2]==0)/*a*/
		{
			if((folder_from = opendir(path))==NULL)
				return -1;
			else
			{
				while((entry = readdir(folder_from))!=NULL)
				{
					strcpy(name, entry->d_name);
					printf("\033[1;32m%s\n", name);
					reset();
				}

			}
			closedir(folder_from);
			return 0;
		}
		else
		{
			red();
			printf("Microshell obsluguje tylko 3 flagi: -a, -l, -m.\n");
			reset();
		}
	}
	else
	{
		if((folder_from = opendir(path))==NULL)
			return -1;
		else
		{
			while((entry = readdir(folder_from))!=NULL)
			{
				strcpy(name, entry->d_name);
				if(name[0]!=46)
					printf("\033[1;32m%s\n", name);
				reset();
			}
		}
		closedir(folder_from);
		return 0;
	}
	return 0;
}

int main()
{
	char **command;
	char path[2048];
	char *username = getlogin();
	bool print_path=false, exit_microshell=false;
	
	do
	{
		reset();
		getcwd(path, sizeof(path));


		if(!print_path)
		{
			print_path=true;
			username=getlogin();
			printf("\033[1;34m{%s}\033[1;32m[%s] \033[1;33m$ ", username, path);
			reset();
			command=read_command();
		}

		if((strcmp(command[0], "cd"))==0)
		{
			print_path=false;
			if(cd(command)!=0)
			{
				perror("Komunikat bledu\033[1;31m");
				reset();
				printf("Numer bledu: \033[1;31m%d\n", errno);
				reset();
			}
		}
		else if(strcmp(command[0], "help")==0)
		{
			print_path=false;
			help();
		}
		else if(strcmp(command[0], "ls")==0)
		{
			print_path=false;
			if(ls(command)!=0)
			{
				perror("Komunikat bledu\033[1;31m");
				reset();
				printf("Numer bledu: \033[1;31m%d\n", errno);
				reset();
			}
		}
		else if(strcmp(command[0], "cp")==0 && command[1]!=NULL)
		{
			print_path=false;

			if(cp(command)!=0)
			{
				perror("Komunikat bledu\033[1;31m");
				reset();
				printf("Numer bledu: \033[1;31m%d\n", errno);
				reset();
			}
		}
		else if(strcmp(command[0], "exit")==0)
			exit_microshell=true;
		else if(strcmp(command[0], "cp")!=0 && strcmp(command[0], "exit")!=0 && strcmp(command[0], "ls")!=0 && strcmp(command[0], "help")!=0 && command!=NULL)
		{

			print_path=false;
			pid_t child_id = fork();

			if(child_id==0)
			{
				if(run_exec(command)!=-1)
					exit(EXIT_SUCCESS);
				else
				{
					perror("Komunikat bledu\033[1;31m");
					reset();
					printf("Numer bledu: \033[1;31m%d\n", errno);
					reset();
					exit(EXIT_FAILURE);
				}
			}
			else
			{
				wait(NULL);
			}
		}

		for(i=0; i<=amount; i++)
			free(command[i]);

		command=NULL;
	}
	while(!exit_microshell);

	return 0;
}