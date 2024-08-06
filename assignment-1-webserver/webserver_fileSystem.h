#ifndef WEBSERVER_FILESYSTEM_H
#define WEBSERVER_FILESYSTEM_H	

	/*Include headers*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

	/*Define maximum ammount of data*/
#define MAX_AMMOUNT_OF_DATA 100
#define MAX_BODY_SIZE 8192

	/*Define file*/
typedef struct file_{
	char* fileName;
	char* fileData;
} file;


		//Important functions


	/*Make the dynamic folder*/
file* makeDynamicFolder();

	/*Delete the dynamicFolder*/
void deleteDynamicFolder(file* dynamicFolder);

	/*Reads the content of a file at given path*/
char* readFileContent(file* dynamicFolder, const char* path1);

	/*Put file into foler under given path*/
int putFile(file* dynamicFolder, const char* path, const char* data);

	/*Delete file under given path*/
int deleteFile(file* dynamicFolder, const char* path);


		//Backend functions


	/*Give file with given name from foler*/
file* giveFileFromFolder(file* folder, char* name);

	/*puts a file into a folder*/
int putFileIntoFolder(file* folder, char* name, const char* data);

	/*Delete a file with given name from given folder*/
int deleteFileFromFolder(file* folder, char* name);

#endif