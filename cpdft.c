#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<dirent.h>
#include<errno.h>

/*
*	Leonardo Javier Malcotti.
*      ___          ___                    ___             
*     /\__\        /\  \     _____        /\__\            
*    /:/  /       /::\  \   /::\  \      /:/ _/_   ___     
*   /:/  /       /:/\:\__\ /:/\:\  \    /:/ /\__\ /\__\    
*  /:/  /  ___  /:/ /:/  //:/  \:\__\  /:/ /:/  //:/  /    
* /:/__/  /\__\/:/_/:/  //:/__/ \:|__|/:/_/:/  //:/__/     
* \:\  \ /:/  /\:\/:/  / \:\  \ /:/  /\:\/:/  //::\  \     
*  \:\  /:/  /  \::/__/   \:\  /:/  /  \::/__//:/\:\  \    
*   \:\/:/  /    \:\  \    \:\/:/  /    \:\  \\/__\:\  \   
*    \::/  /      \:\__\    \::/  /      \:\__\    \:\__\  
*     \/__/        \/__/     \/__/        \/__/     \/__/ 
*
*
*	cpdt - c pdf tool
*	A small and under development tool to modify pdf written in c.
*/

#define PROG_NAME argv[0]
#define FIRST_ARG argv[1]
#define SECON_ARG argv[2]

/* 
*	Check if the path passed is of a directory.
*/
int is_dir(const char* path){
	struct stat path_stat;
	stat(path, &path_stat);
	return S_ISDIR(path_stat.st_mode);
}

/* 
*	Execute gs with the appropiate arguments to combine the two pdf passed as argument.
*	
*	The function shouldn't return anything as the last operation is a call to execlp().
*	In case it fails it will print an error without doing anything else.
*	The file created will be called as the first argument passed with an added "m_" at the start.
*/
void exec_gs(const char* first_file, const char* second_file){

	//create the name of the new file, adding m_ as prefix
	char* new_file = (char*)calloc(strlen("-sOutputFile=m_") + strlen(first_file), sizeof(char));
	strcpy(new_file,"-sOutputFile=m_");
	strcat(new_file,first_file);

	//exec ghostscript
	execlp("gs", "gs", "-q", "-sDEVICE=pdfwrite", "-dNOPAUSE", "-dBATCH", "-dSAFER", new_file, first_file, second_file, NULL);

	fprintf(stderr, "%s\n", strerror( errno ));
}

/* 
*	This is a comodity version of exec_gs which combines the name of the first file with the name of the dir in which it is in.
*/
void exec_gs_dir(const char* dir, const char* first_file, const char* second_file){

	char* df = (char*)calloc(strlen(dir) + strlen(first_file) + 2,sizeof(char));
	strcpy(df,dir);
	strcat(df,"/");
	strcat(df,first_file);

	char* new_dir = (char*)calloc(strlen("m_") + strlen(dir) + 2,sizeof(char));
	strcpy(new_dir,"m_");
	strcat(new_dir,dir);

	exec_gs(df,second_file);
}

/*
*	Main function of the program in case the first argument is a directory.
*
*	It will generate a new directory called as the one passed plus a prefix "m_".
*	The generated file will be placed in the new directory.
*	The new files will be generated appending to each file in the directory passed the file passed as second argument.
*	The procedure will generate a fork for every file it has to make.
*/
void procedure_input_dir(int argc, char * argv[]){

	printf("%s: dir detected.\n",PROG_NAME);

	char* new_dir = (char*)calloc(strlen("m_") + strlen(FIRST_ARG) + 2,sizeof(char));
	strcpy(new_dir,"m_");
	strcat(new_dir,FIRST_ARG);

	//make a new dir where to store the generated files.
	if( mkdir(new_dir,S_IRWXU) == -1 ){
		fprintf(stderr, "failed to create a directory\n%s\n", strerror( errno ));
		exit(-1);
	}

	struct dirent *dp;
	DIR* dir_stream = opendir(FIRST_ARG);

	//read the content of the directory
	while(	dir_stream	){

		if ( (dp = readdir(dir_stream)) == NULL ) {
			closedir(dir_stream);
			break;
		}

		char * fileName = (char*)calloc(strlen(dp->d_name),sizeof(char));
		strcpy(fileName,dp->d_name);

		//check if it isn't an hidden file
		if(	fileName[0] != '.'	){

			//create a child for every file
			if( fork() == 0 ) {

				printf("creating: %s\n",dp->d_name);
				exec_gs_dir(FIRST_ARG, dp->d_name, SECON_ARG);
			}
		}
	}

	//wait for all the children to finish
	while(wait(NULL) > 0);

}

int main(int argc, char* argv[]){

	if( argc != 3 ){
		printf("%s: needs 2 arguments.\n",PROG_NAME);
		exit(-1);
	}

	if( is_dir(FIRST_ARG) ){
		procedure_input_dir(argc,argv);
	} else {

		printf("%s: single file detected.\n",argv[0]);
		exec_gs(FIRST_ARG,SECON_ARG);
	}

	return 0;
}

