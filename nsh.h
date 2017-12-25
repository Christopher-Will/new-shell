#ifndef NSH_H
#define NSH_H
#include <string>
#include <map>
#include <iostream>
#include <vector>
using namespace std;

//Define the globals needed in nsh.cpp
map<string, string> userVariables; //hash table to hold the users variable and the value it maps to
map<int, string> backgroundProcs; // hash table for the background processes
string userPrompt = "nsh > "; //default user prompt
bool showTokensIsSet = false; //by default, ShowTokens is turned off
bool mismatchQuotes; //determine whether user gave an opening quote without a matching closing quote

//Signatures for all the functions used by nsh.cpp
void setUserVariable(vector<string> eachToken); //assign the variable given to its associated value
void removeComments(vector<char> &eachChar); //strip comments from the users line
bool fileExists(string userFile); //make sure the file the user wishes to execute is in the path
void callCommand(string userCommand, vector<string> eachToken); //handle all the possible commands the user may give
bool checkVarName(string varName);//make sure the var name given is valid
void changePrompt(vector<string> eachToken);// change the prompt
void showTokens(vector<string> eachToken); //display each token
vector<string> scanLine(vector<char>  eachChar);//tokenize the users input
void expandVariables(vector<string> &eachToken);//substitute any variables given with their value
void showProcs(vector<string> eachToken); //show any background processes
void executeBack(vector<char*> args, string filePath); //perform the back command
void executeDo(vector<char*> args, string filePath); //perform the do command
void executeToVar(vector<char *> args, string filePath, string varName); //perform the tovar command
bool processCommand(vector<string> eachToken, unsigned int minNumTokens, vector<char *> &args, string &filePath);
//do pre-processing for the back, do, or tovar command
#endif
