/*Christopher Will
CS 485g Program 4 */
#include "nsh.h"
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

//make sure the variable name the user gave is valid
bool checkVarName(string varName){
	if(!(isalpha(varName[0]))){
		cerr << "Variable names must begin with a letter " << endl;
		return false;//variable name didn't begin with a letter, so return false
	}
	for(unsigned int i = 1; i < varName.length(); i+=1){
		if(!(isalnum(varName[i]))){
			cerr << "Variables names may contain only numbers and letters " << endl;
			return false;//the remaining characters must be alphanumeric, else return false
		}
	}
	return true;/*the variable started with a letter and then contained only
			alphanumeric characters, so return true for valid var name*/
}

//add the users variable and its value to the hash table
void setUserVariable(vector<string> eachToken){
	if(eachToken.size() != 3){
		cerr << "set expects 3 tokens but " << eachToken.size() << " provided " << endl;
		return;//incorrect number of tokens given, so throw error and return
	}
	string varName = eachToken[1];//the variable name is the 2nd token entered
	bool isValidVarName = checkVarName(varName);//make sure the variable name is of the correct form
	if(isValidVarName){
		string varValue = eachToken[2];//the value of the variable is the 3rd token
		userVariables[varName] = varValue;//make the varName map to varValue
	}
	if(varName == "ShowTokens" ){
		if(userVariables[varName] == "1"){
			showTokensIsSet = true;
		}else{
			showTokensIsSet = false;//user entered ShowTokens and some non-1 value
		}
	}
}

void changeDirectory(vector<string> eachToken){
	if(eachToken.size() != 2){
		cerr << "dir expects 2 tokens but " << eachToken.size() << " provided " << endl;
		return;// user gave incorrect number of tokens, so throw error and return
	}
	string directory = eachToken[1];//directory to change to will be the 2nd token given
	int validChdir = chdir(directory.c_str());//change directories, only return of directory was invalid
	if(validChdir < 0){
		perror("Error changing directories");
	}
}

//make sure the file the user wishes to call exec() on is in the PATH
bool fileExists(string userFile, string &filePath){
	if(userFile[0] == '/' || userFile[0] == '.'){
		ifstream file(userFile.c_str());//user gave absolute or relative path for the file
		if(file){
			filePath = userFile;
			return true;//no errors in opening the file so return true
		}else{
			cerr << userFile << " cannot be found in the current path " << endl;
			return false;//file does not exist
		}
	}		
	string currentPath;
	userVariables["PATH"]+=':';/*make the last char in PATH be :, this helps for 
					using : as the delimiter*/

	for(unsigned int i = 0; i < userVariables["PATH"].length(); i+=1){
		if(userVariables["PATH"][i] != ':'  ){
			currentPath+=userVariables["PATH"][i];
			//split the PATH by : and check each path given
		}else{
			string tryPath = currentPath + '/' + userFile;//try the current path with the users file
			ifstream file(tryPath.c_str());
			if(file){
				filePath = tryPath;
        			userVariables["PATH"] = userVariables["PATH"].substr(0, userVariables["PATH"].length() - 1);
				return true;//file exists so return true and remove the last : from PATH
			}
			else{
				currentPath = "";// the file was not found in that path, so reset currentPath
			}
		}
	}
	userVariables["PATH"] = userVariables["PATH"].substr(0, userVariables["PATH"].length() - 1);
	//Remove the last : which was added before the start of the above for loop
	cerr << userFile << " cannot be found in the current path" << endl;
	return false;//never returned that the file was in the path, so it must then not be in the current PATH
}


void executeDo(vector<char *> args, string filePath){
	pid_t pid = fork();
	int status;//status is used in waitpid() to wait for the child to terminate before the parent returns		
	if(pid == 0){//in the child process
		if((execv((char *)filePath.c_str(), &args.front())) < 0){
			perror("Error calling exec");//execv() returned so throw an error
		}
	}else{//in the parent process
		waitpid(pid, &status, 0);// wait for the child to finish executing before returning
	}
}

//Pre-processing to be done before calling do, back, or tovar
bool processCommand(vector<string> eachToken, unsigned int minNumTokens, vector<char *> &args, string &filePath){
	if(eachToken.size() < minNumTokens){
		cerr << "expected at least " << minNumTokens << " tokens, but got " << eachToken.size() << " tokens " << endl;
		return false;//minNumTokens is either 2 for do and back, or 3 for tovar
	}
	if(eachToken[0] == "tovar"){//tovar is special. Need to make sure the variable name given is valid
		string varName = eachToken[1];//the variable name is the 2nd token
		bool validVarName = checkVarName(varName);//make sure the vairable name was of correct form
		if(!validVarName){
			return false;// variable name was not in correct form, so return
		}
	}

	string command = eachToken[minNumTokens - 1];/*the file to execute will be either the 2nd for back and do, or
							the 3rd token for tovar	*/
	bool validCommand = fileExists(command, filePath);//make sure the file exists in the current PATH
	if(!validCommand){
		return false;//file does not exist in current PATH, so return false
	}
	for(unsigned int i = minNumTokens - 1; i < eachToken.size(); i+=1){
		args.push_back((char *)eachToken[i].c_str());//add the file and any options it's to be executed with to args
	}
	args.push_back(0);//add the NULL terminator to args
	return true;
}

//call execv(), but don't wait for the child process to finish, just return back to the prompt
void executeBack(vector<char*> args, string filePath){
	pid_t pid = fork();
	if(pid == 0){//in the child process
		if((execv((char *)filePath.c_str(), &args.front())) < 0){
			perror("Error calling exec"); //execv() returned, so there must have been an error
		}
	}
	backgroundProcs[pid] = filePath;
	/*to keep track of the background processes we have the pid of the parent map to the file the user
	executed. We only really need the pid, but having the file path makes it more clear as to which pid goes
	with which file*/
	
}
void executeToVar(vector<char *> args, string filePath, string varName){
	pid_t pid = fork();
	int status;
	string tempFile = "/tmp/out.txt";
	if(pid == 0){
		int outFile = open(tempFile.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
		dup2(outFile, 1);
		close(outFile);
		if((execv((char *)filePath.c_str(), &args.front())) < 0){
			perror("error calling exec");
		}
	}else{
		waitpid(pid, &status, 0);
	}
	ifstream readFile;
	readFile.open(tempFile.c_str());
	
	string line;
	string fileOutput;
	while(getline(readFile, line)){
		fileOutput+=line; 
		fileOutput+="\n";
	}
	userVariables[varName] = fileOutput;
	remove(tempFile.c_str()); //clear the contents of /tmp/out.txt so that the next tovar command can start from a blank file	
}

//give the status of any processes running in the background
void showProcs(vector<string> eachToken){
	if(eachToken.size() != 1){
		cerr << "procs expects 1 token, but " << eachToken.size() << " were provided" << endl;
		return;//give error if more than 1 token given
	}
	if(backgroundProcs.size() == 0){
		cerr << "No background processes have been executed " << endl;
		return;//give error if no background processes have been executed yet
	}

	for(map<int, string>::iterator it = backgroundProcs.begin(); it != backgroundProcs.end(); it++){
		pid_t pid = it->first; // the pid of the process
		string file = it->second; // the file file of the process being executed
		int status;
		pid_t result = waitpid(pid, &status, WNOHANG); // get the status of the process without having to wait for it to return
		if(result != 0){ 
			cout << pid << " (" << file << ") has finished running " << endl;
		}else{
			cout << pid << " (" << file << ") is currently running" << endl;
		}
	}
}

//call the relevant command, based on the 1st token given
void callCommand(string userCommand, vector<string> eachToken){
	vector<char *> args;//used to store the tokens for the do, back, and tovar command
	string filePath; //the file path to the file, if it exists. for the do, back and tovar commands

	//Call relevant functions based on the value of userCommand
	if(userCommand == "set"){
		setUserVariable(eachToken);
	}else if(userCommand == "prompt"){
		changePrompt(eachToken);
	}else if(userCommand == "dir"){
		changeDirectory(eachToken);	
	}else if(userCommand == "procs"){
		showProcs(eachToken);
	}else if(userCommand == "do"){
		bool validCommand = processCommand(eachToken, 2, args, filePath);
		if(validCommand){ //make sure the file to execute is in the PATH
			executeDo(args, filePath);
		}
	}else if(userCommand == "back"){
		bool validCommand = processCommand(eachToken, 2, args, filePath);
		if(validCommand){//make sure the file to execute is in the PATH
			executeBack(args, filePath);
		}
	}else if(userCommand == "tovar"){
		bool validCommand = processCommand(eachToken, 3, args, filePath);
		if(validCommand){//make sure the file to execute is in the PATH
			executeToVar(args, filePath, eachToken[1]);
		}
	}else{
		cerr << userCommand << " is not a recognized command " << endl;
	}
}

//update the user's prompt
void changePrompt(vector<string> eachToken){
	if(eachToken.size() != 2){
		cerr << "prompt expects 2 tokens, but " << eachToken.size() << " provided" << endl;
		return;//prompt only takes 2 tokens, so throw error
	}
	userPrompt = eachToken[1];//the new prompt will be the 2nd token given
}

//strip comments from the user's line
void removeComments(vector<char> &eachChar){
	for(unsigned int i = 0; i < eachChar.size(); i+=1){//loop over each char the user entered
		if(eachChar[i] == '%'){//found a comment, so delete the comment and every char after it
			vector<char> tempCharVector;
			for(unsigned int j = 0; j < i; j+=1){
				tempCharVector.push_back(eachChar[j]);
			}
			eachChar = tempCharVector;
		}
	}
}

//look for any variables, and replace them with their value if they're found
void expandVariables(vector<string> &eachToken){
	bool foundVariable = false; //used to denote whether a variable has been found or not
	for(unsigned int i = 0; i < eachToken.size(); i+=1){//loop over each char in each string in the eachToken vector
		for(unsigned int j = 0; j < eachToken[i].length(); j+=1){
			if(eachToken[i][j] == '$'){//found a variable
				foundVariable = true;
				string userVar;
				int startIndex = j;//get the index where the variable was found
				j+=1;//move j to look at the next char
				while(j < eachToken[i].length() && eachToken[i][j] != ' ' && eachToken[i][j] != '"'){
					userVar+=eachToken[i][j];
					j+=1;//get each char after the $, so long as we don't find a space, quote, or are at the end of their line
				}
				string varToSub;//used to see if the variable given actually exists
				if(userVariables.count(userVar) > 0){//see if userVar is a key in userVariables
					varToSub = userVariables[userVar];
				}else{
					cerr << userVar << " has not yet been defined " << endl;
					eachToken[i] = ""; //replace the incorrect variable with the empty string
					//variable is not in userVariables, so throw error
				}
				int lastIndex = j;//this is where the variable stopped
				string newToken;//make a new token where the var has been expanded
				for(int k = 0; k < startIndex; k+=1){
					newToken+=eachToken[i][k];
					//add the char's before the variable was found to newToken
				}
				for(unsigned int k = 0; k < varToSub.length(); k+=1){
					newToken+=varToSub[k];
					//add the value of the $variable to newToken
				}
				for(unsigned int k = lastIndex; k < eachToken[i].length(); k+=1){
					newToken+=eachToken[i][k];
					//add the char's found after the $variable to newToken
				}
				//replace the value at the ith index to the token with the expanded variable 
				eachToken[i] = newToken;
				j-=1;
			}
		}
	}
	if(showTokensIsSet && foundVariable){
		cout << "After expansion: " << endl;
		showTokens(eachToken);
	}//found at least 1 variable, so show the tokens now after the expansion(if ShowTokens is 1).
}

void showTokens(vector<string> eachToken){
	for(unsigned int i = 0; i < eachToken.size(); i+=1){
		cout << "token: " << eachToken[i] << endl;
	}
}//output each token after comments have been removed from the user's line

//tokenize the user's input
vector<string> scanLine(vector<char>  eachChar){
	vector<string> eachToken;//vector to hold each token
	for(unsigned int i = 0; i < eachChar.size(); i+=1){
		string token;//this will hold the current token
		if(eachChar[i] == ' ' || eachChar[i] == '\t'){
			continue;//current char was a space, so don't do anything
		}
		if(eachChar[i] == '"'){//user entered a quote, so get all char's until a matching quote, or the end of the line is reached
			i+=1;//move to the next char
			if(i == eachChar.size()){
				mismatchQuotes = true;
				return eachToken;//user gave only 1 quote with nothing after it, so return
			}
			while(eachChar[i] != '"'){//keep getting char's until the end of the line, or matching quote is found
				token+=eachChar[i];
				i+=1;
				if(i == eachChar.size()){//reached the end of the line before finding a matching quote
					mismatchQuotes = true;
					return eachToken; 
					//I treat a mismatched quote as an error
				}	
			}
			//add the valid token to the token vector
			eachToken.push_back(token);
		}else{//current char is not a space or a quote, so it must just be a bare token
			while(i < eachChar.size() && eachChar[i] != ' ' && eachChar[i] != '\t'){
				token+=eachChar[i];
				i+=1;//add in each letter from this bare token until the end of the line is reached, or we find a space
			}
			eachToken.push_back(token);//add this bare token to the token vector
		}
	}
	mismatchQuotes = false;
	return eachToken;//return the token vector
}

int main(){
	userVariables["PATH"] = "/bin:/usr/bin"; //the default PATH
	string line;
	cout << userPrompt;
	while(getline(cin, line)){//Read the users input, line by line
	        vector<char> eachChar;
	        for(unsigned int i = 0; i < line.length(); i+=1){
        	        eachChar.push_back(line[i]);
        	}//store each character from the user's line in this vector

		removeComments(eachChar);//strip the vector of any characters following %
		string userLine;
		for(unsigned int i = 0; i < eachChar.size(); i+=1){
			userLine+=eachChar[i];
		}//make a string from the comment-less user line
		vector<string> eachToken;
		eachToken = scanLine(eachChar);// tokenize the user's input
		if(eachToken.size() == 0){
			cout << userPrompt;
			continue; //user gave no command, so prompt for next instruction
		}
		if(mismatchQuotes){
			cerr << "Mismatch quotes error " << endl;
			cout << userPrompt;
			continue;
		}
		if(showTokensIsSet){
			showTokens(eachToken);
		}//user wants to see each token, so output them

		expandVariables(eachToken);//show the tokens after substituting the variables for their values
		string firstCommand = eachToken[0];// get the command the user wishes to execute in nsh
		if(firstCommand == "done"){
			break;// exit the loop if the user typed done
		}
		callCommand(firstCommand, eachToken);//call the appropriate function based on firstCommand
		cout << userPrompt;
	}
	return 0;
}
