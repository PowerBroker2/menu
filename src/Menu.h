#pragma once
#include "Arduino.h"
#include "SdFat.h"




extern "C"
{
	typedef void (*callbackFunction)(void);
}




#define MAX_NAME_LEN       50
#define MAX_NUM_SUBOPTIONS 15
#define MAX_DIRS           10
#define MAX_DIRNAME_LEN    30
#define MAX_FILES          50
#define MAX_FILENAME_LEN   30
#define MAX_FILE_CONTENTS  1000




class Option
{
public:
    void setName(char optionName[]);
    void setName(const char optionName[]);
    char* name();

    void setFileMenu(const bool& pref);
    bool isFileMenu();
    void setBackOpt(const bool& pref);
    bool isBackOpt();

    void setCallback(callbackFunction callbackFunc);
    void runCallback();
    bool hasCallback();

    bool addOption(Option* option);
    char* optionName(const int& index);
    Option* option(const int& index);
    bool rmOption(const int& index);
    bool pop();
    void clear();
    int numOptions();

    char* parentName();
    Option* parent();

    friend class Menu;




private:
    bool _fileMenu;
    bool _isFile;
    bool _backOpt;
    int _numOptions;
    char _name[MAX_NAME_LEN];
    Option* options[MAX_NUM_SUBOPTIONS];
    Option* _parent;
    callbackFunction callback;
};




class Menu : public Option
{
public:
    bool setSelection(const int& index);
    int curSelection();
    Option* curMenu();
    char* curMenuName();
    int curNumOptions();
    Option* curOption();
    char* curOptName();
    Option* curParent();
    char* curParentName();

    void setSD(SdFs* sd);
    bool fileRead();
    char* fileContent(int& len);

    void recurse(Option* option, const int& numTabs);
    void tree();

    bool up();
    bool down();
    bool enter(bool _runCallback = true);
    bool out(bool _runCallback = true);




private:
    int _curOption;
    Option* curMenuPtr = this;

    SdFs* _sd;
    char pwd[50] = "/";
    int numDirs;
    int numFiles;
    char dirNames[MAX_DIRS][MAX_DIRNAME_LEN];
    char fileNames[MAX_FILES][MAX_FILENAME_LEN];
    Option dirOptions[MAX_DIRS];
    Option fileOptions[MAX_FILES];
    char fileContents[MAX_FILE_CONTENTS];
    int bytesRead;
    bool _fileRead;

    Option back;




    void stepIn();
    void stepOut();
    bool ls(const char path[]);
    bool readFile(char filepath[]);
    void readSDFile();
};
