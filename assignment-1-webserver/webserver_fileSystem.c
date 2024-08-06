	/*Include headers*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "webserver_fileSystem.h"
#include "webserver_logger.h"

	/*Make the dynamic folder*/
file* makeDynamicFolder(){
	/*Allocate memory for dynamic folder*/
	file* dynamicFolder = malloc(MAX_AMMOUNT_OF_DATA * sizeof(file));

	if (dynamicFolder == NULL) {
		free(dynamicFolder);
		perror("dynamic folder malloc error");
		return NULL;
	}

	/*Allocate Memory for all files*/
	for (int i = 0; i < MAX_AMMOUNT_OF_DATA; ++i) {
		dynamicFolder[i].fileName = malloc(1024 * sizeof(char));
		dynamicFolder[i].fileData = malloc(MAX_BODY_SIZE * sizeof(char));
		// memory allocation error handling
		if (dynamicFolder[i].fileName == NULL || dynamicFolder[i].fileData == NULL) {
            // Handle memory allocation error for fileName or fileData
            free(dynamicFolder[i].fileName);
			free(dynamicFolder[i].fileData);
			perror("files malloc error");
            return NULL;
		}

		memset(dynamicFolder[i].fileName, 0 , 1024);
		memset(dynamicFolder[i].fileData, 0, MAX_BODY_SIZE);

	}
	/*return the dynamic folder*/
	return dynamicFolder;
}
	

	/*Delete the dynamicFolder*/
void deleteDynamicFolder(file* dynamicFolder){
	/*Delete all fileNames and fileDatas*/
	for (int i = 0; i < MAX_AMMOUNT_OF_DATA; ++i){
		free(dynamicFolder[i].fileName);
		free(dynamicFolder[i].fileData);
	}
	
	/*Delete the dynamic folder*/
	free(dynamicFolder);

	/*Leave the function*/
	return;
}


	/*Give file with given name from foler*/
file* giveFileFromFolder(file* folder, char* name){
	/*Check, if parameters are valid*/
	if(folder != NULL){
		
		/*Make returnable file*/
		file* returnFile = NULL;

		/*Go through folder and search for file*/
		for (int i = 0; i < MAX_AMMOUNT_OF_DATA; ++i){
			if (strcmp(folder[i].fileName, name) == 0){
				returnFile = &folder[i];
			}
		}

		/*Return the file*/
		return returnFile;
	}
	else{
		return NULL;
	}
}


	/*Reads the content of a file at given path*/
char* readFileContent(file* dynamicFolder, const char* path1){
	/*Check, if there is a path*/
	if (path1 != NULL){
		
	/*Due to technical reasons, the path will be casted to an array*/
		char path[strlen(path1) + 1];
		strcpy(path, path1);

	/*Divide the path into folder and file*/
		char* pathFolder = strtok(path, "/");
		char* pathFile = strtok(NULL, "/");


	/*Check, if path is valid*/		
		if (strtok(NULL, "/") == NULL && pathFolder != NULL && pathFile != NULL){
			
			//Static content*/
			if (strcmp(pathFolder, "static") == 0){
				if (strcmp(pathFile, "foo") == 0){
					return "Foo";
				}
				else if (strcmp(pathFile, "bar") == 0){
					return "Bar";
				}
				else if (strcmp(pathFile, "baz") == 0){
					return "Baz";
				}
				else{
					return NULL;
				}
			}

			//Dynamic content
			else if (strcmp(pathFolder, "dynamic") == 0){
				file* returnFile = giveFileFromFolder(dynamicFolder, pathFile);
				if (returnFile != NULL){
					return returnFile->fileData;
					// logMessage("Returning data to the request Handler: fileData: %s", returnFile->fileData);

				}
				else{
					return NULL;
				}
			}
			else{
				return NULL;
			}
		}
		else{
			return NULL;
		}
	}
	else{
		return NULL;
	}	
}


	/*puts a file into a folder*/
int putFileIntoFolder(file* folder, char* name, const char* data){
	/*Checks, if arguments are valid*/
	if(folder != NULL && name != NULL){

	/*Go through folder and search for the first empty file*/
		int fileIndex = 0;
		bool emptyFileFound = false;
		while(fileIndex < MAX_AMMOUNT_OF_DATA && emptyFileFound == false){
			if (strcmp(folder[fileIndex].fileName, "\0") == 0){
				emptyFileFound = true;
			}
			else{
				fileIndex++;
			}
		}

	/*Checks, if empty file was found*/
		if (emptyFileFound == true){

	/*Write the file*/
			strcpy(folder[fileIndex].fileName, name);
			strcpy(folder[fileIndex].fileData, data);

			// logMessage("Write data to the file is successful: fileName: %s, fileData: %s", folder[fileIndex].fileName, folder[fileIndex].fileData);
	/*Leave the function*/
			return 0;
		}
		else{
			return -1;
		}
	}
	else{
		return -1;
	}
}


	/*Put file into foler under given path*/
int putFile(file* dynamicFolder, const char* path, const char* data){
	/*Check, if arguments are valid*/
	if(dynamicFolder != NULL && path != NULL){

	/*Extract the foldeName and the fileName*/
		char path_[strlen(path) + 1];
		strcpy(path_, path);

		char* folderName = strtok(path_, "/");
		char* fileName = strtok(NULL, "/");

	/*Check, if folderName, fileName and path are valid*/
		if(folderName != NULL && fileName != NULL && strtok(NULL, "/") == NULL){

	/*Check, if folderName is dynamic*/
			if(strcmp(folderName, "dynamic") == 0){

	/*Check, if file already exists*/
				file* newFile = giveFileFromFolder(dynamicFolder, fileName);
				if (newFile != NULL){
					// Free the existing memory if newFile->fileData is already allocated
					free(newFile->fileData);

					// Allocate new memory and copy the data
					newFile->fileData = malloc(strlen(data) + 1); // +1 for the null terminator
					if (newFile->fileData == NULL) {
						// Handle memory allocation error
						perror("fileData");
						return -1; 
					}
					strncpy(newFile->fileData, data, strlen(data) + 1);
					return 0;
				}
				else{
					return putFileIntoFolder(dynamicFolder, fileName, data);
				}
			}
			else{
				return -1;
			}
		}
		else{
			return -1;
		}
	}	
	else{
		return -1;
	}
}


	/*Delete a file with given name from given folder*/
int deleteFileFromFolder(file* folder, char* name){
	/*Checks, if arguments are valid*/
	if (folder != NULL && name != NULL){
		
	/*Go through folder and search for the given file*/
		int fileIndex = 0;
		bool fileFound = false;
		while(fileIndex < MAX_AMMOUNT_OF_DATA && fileFound == false){
			if (strcmp(folder[fileIndex].fileName, name) == 0){
				fileFound = true;
			}
			else{
				fileIndex++;
			}
		}

	/*Delete the file, if it was found*/
		if (fileFound == true){
			strcpy(folder[fileIndex].fileName, "\0");
			strcpy(folder[fileIndex].fileData, "\0");

			return 0;
		}
		else{
			return -1;
		}
	}
	else{
		return -1;
	}
}


	/*Delete file under given path*/
int deleteFile(file* dynamicFolder, const char* path){
	/*Check, if arguments are valid*/
	if(dynamicFolder != NULL && path != NULL){

	/*Extract the foldeName and the fileName*/
		char path_[strlen(path) + 1];
		strcpy(path_, path);

		char* folderName = strtok(path_, "/");
		char* fileName = strtok(NULL, "/");

	/*Check, if folderName, fileName and path are valid*/
		if(folderName != NULL && fileName != NULL && strtok(NULL, "/") == NULL){

	/*Check, if folderName is dynamic*/
			if(strcmp(folderName, "dynamic") == 0){

	/*Delete the file*/
				// strcpy(fileName, fileName);
				return deleteFileFromFolder(dynamicFolder, fileName);
			}
			else{
				return -1;
			}
		}
		else{
			return -1;
		}
	}	
	else{
		return -1;
	}
}