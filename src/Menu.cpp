#include "menu.h"
#include "SdFat.h"




void Option::setName(char optionName[])
{
    if (strlen(optionName) < MAX_NAME_LEN)
	    strcpy(_name, optionName);

    _name[sizeof(_name) - 1] = '\0';
}




void Option::setName(const char optionName[])
{
    if (strlen(optionName) < MAX_NAME_LEN)
        strcpy(_name, optionName);

    _name[sizeof(_name) - 1] = '\0';
}




char* Option::name()
{
	return &_name[0];
}




void Option::setFileMenu(const bool& pref)
{
    _fileMenu = pref;
}




bool Option::isFileMenu()
{
    return _fileMenu;
}




void Option::setBackOpt(const bool& pref)
{
    _backOpt = pref;
}




bool Option::isBackOpt()
{
    return _backOpt;
}




void Option::setCallback(callbackFunction callbackFunc)
{
    callback = callbackFunc;
}




void Option::runCallback()
{
    if (hasCallback())
        callback();
}




bool Option::hasCallback()
{
    if (callback)
        return true;
    
    return false;
}




bool Option::addOption(Option* option)
{
    if (numOptions() < (int)sizeof(options))
    {
        options[numOptions()] = option;
        option->_parent = this;
        _numOptions++;

        return true;
    }

    return false;
}




char* Option::optionName(const int& index)
{
    if (index < numOptions())
        return option(index)->name();
    else
        return NULL;
}




Option* Option::option(const int& index)
{
    if (index < numOptions())
        return options[index];
    else
        return NULL;
}




bool Option::rmOption(const int& index)
{
    if (index < numOptions())
    {
        for (int i = index; i < (numOptions() - 2); i++)
            options[i] = options[i + 1];

        pop();
    }

    return false;
}




bool Option::pop()
{
    if (numOptions())
    {
        options[numOptions() - 1] = NULL;
        _numOptions--;

        if (numOptions())
            return true;
    }
    
    return false;
}




void Option::clear()
{
    while (pop());
}




int Option::numOptions()
{
    return _numOptions;
}




char* Option::parentName()
{
    return parent()->name();
}




Option* Option::parent()
{
    return _parent;
}




bool Menu::setSelection(const int& index)
{
    if (index < curNumOptions())
        _curOption = index;
    
    return false;
}




int Menu::curSelection()
{
    return _curOption;
}




Option* Menu::curMenu()
{
    return curMenuPtr;
}




char* Menu::curMenuName()
{
    return curMenu()->name();
}




int Menu::curNumOptions()
{
    return curMenu()->numOptions();
}




Option* Menu::curOption()
{
    return curMenu()->option(curSelection());
}




char* Menu::curOptName()
{
    return curOption()->name();
}





Option* Menu::curParent()
{
    return curMenu()->parent();
}




char* Menu::curParentName()
{
    return curMenu()->parentName();
}




void Menu::setSD(SdFs* sd)
{
    _sd = sd;
}




bool Menu::fileRead()
{
    return _fileRead;
}




char* Menu::fileContent(int& len)
{
    if (fileRead())
    {
        len = bytesRead;
        return fileContents;
    }
    else
        return NULL;
}




bool Menu::up()
{
    if (curSelection() > 0)
    {
        setSelection(curSelection() - 1);
        return true;
    }

    return false;
}




bool Menu::down()
{
    if (curSelection() < (curNumOptions() - 1))
    {
        setSelection(curSelection() + 1);
        return true;
    }

    return false;
}




bool Menu::enter(bool _runCallback)
{
    if (curSelection() < curNumOptions())
    {
        if (_runCallback)
            curOption()->runCallback();

        if (curOption()->isFileMenu())
        {
            if (curOption()->_isFile)
            {
                readSDFile();
                return false;
            }
            
            _fileRead = false;

            if (curOption()->isBackOpt())
                stepOut();
            else
            {
                stepIn();
            }
        }
        else
        {
            if (curOption()->isBackOpt())
                out(false); // callback shouldn't be called twice
            else
            {
                curMenuPtr = curOption();
                setSelection(0);
            }
        }

        if (curNumOptions())
            return true;
    }

    return false;
}




bool Menu::out(bool _runCallback)
{
    if (curParent())
    {
        if (curOption()->isFileMenu())
            stepOut();
        else
        {
            curMenuPtr = curMenu()->parent();
            setSelection(0);
        }

        if (_runCallback)
            curOption()->runCallback();

        if (curNumOptions())
            return true;
    }

    return false;
}




void Menu::recurse(Option* option, const int& numTabs)
{
    for (int i = 0; i < option->numOptions(); i++)
    {
        for (int k = 0; k < numTabs; k++)
            Serial.print("\t");

        if ((option == curMenu()) && (i == curSelection()))
            Serial.print("> ");
        
        Serial.println(option->optionName(i));
        recurse(option->option(i), numTabs + 1);
    }
}




void Menu::tree()
{
    Serial.print("Top menu name: "); Serial.println(name());
    
    if (parent())
    {
        Serial.print("Parent name: ");
        Serial.println(parentName());
    }

    Serial.println("Options:");
    recurse(this, 1);
    Serial.println();
}




void Menu::stepIn()
{
    if (pwd[strlen(pwd) - 1] != '/')
        strcat(pwd, "/");

    if (curMenu()->isFileMenu())
        strcat(pwd, curOptName());
    else
    {
        curMenuPtr = curOption();

        memset(pwd, 0, sizeof(pwd));
        pwd[0] = '/';
    }

    ls(pwd);
    curMenu()->clear();

    memset(fileOptions, 0, sizeof(fileOptions));
    memset(dirOptions,  0, sizeof(dirOptions));

    for (int i = 0; i < numFiles; i++)
    {
        fileOptions[i].setName(fileNames[i]);
        fileOptions[i].setFileMenu(true);
        fileOptions[i]._isFile = true;
        curMenu()->addOption(&fileOptions[i]);
    }

    for (int i = 0; i < numDirs; i++)
    {
        dirOptions[i].setName(dirNames[i]);
        dirOptions[i].setFileMenu(true);
        curMenu()->addOption(&dirOptions[i]);
    }

    back.setName("back");
    back.setBackOpt(true);
    back.setFileMenu(true);
    curMenu()->addOption(&back);

    setSelection(0);
}




void Menu::stepOut()
{
    if (strcmp(pwd, "/")) // exit file manager if trying to step out of root
    {
        for (int i = (strlen(pwd) - 1); i > 0; i--) // remove everything up to last (and including) last '/'
        {
            if (pwd[i] == '/')
            {
                pwd[i] = '\0';
                break;
            }

            pwd[i] = '\0';
        }

        if (ls(pwd))
        {
            curMenu()->clear();

            memset(fileOptions, 0, sizeof(fileOptions));
            memset(dirOptions,  0, sizeof(dirOptions));

            for (int i = 0; i < numFiles; i++)
            {
                fileOptions[i].setName(fileNames[i]);
                fileOptions[i].setFileMenu(true);
                fileOptions[i]._isFile = true;
                curMenu()->addOption(&fileOptions[i]);
            }

            for (int i = 0; i < numDirs; i++)
            {
                dirOptions[i].setName(dirNames[i]);
                dirOptions[i].setFileMenu(true);
                curMenu()->addOption(&dirOptions[i]);
            }

            back.setName("back");
            back.setBackOpt(true);
            back.setFileMenu(true);
            curMenu()->addOption(&back);
        }
    }
    else
        curMenuPtr = curMenu()->parent();
    
    setSelection(0);
}




bool Menu::ls(const char path[])
{
    if (_sd)
    {
        char nameArr[100];

        memset(dirNames,  0, sizeof(dirNames));
        memset(fileNames, 0, sizeof(fileNames));

        numDirs  = 0;
        numFiles = 0;

        if (_sd->exists(path))
        {
            auto dir = _sd->open(path);

            if (dir.isDirectory())
            {
                while (true)
                {
                    auto entry = dir.openNextFile();

                    if (!entry)
                        break;

                    entry.getName(nameArr, sizeof(nameArr));

                    if (!strstr(nameArr, "System Volume Information"))
                    {
                        if (entry.isDirectory())
                        {
                            strcpy(dirNames[numDirs], nameArr);
                            numDirs += 1;
                        }
                        else
                        {
                            strcpy(fileNames[numFiles], nameArr);
                            numFiles += 1;
                        }
                    }
                }
            }
        }
        else
            return false;
    }
    
    return true;
}




bool Menu::readFile(char filepath[])
{
    if (_sd)
    {
        if (_sd->exists(filepath) && !strstr(filepath, "System Volume Information"))
        {
            auto file = _sd->open(filepath, FILE_READ);
            memset(fileContents, 0, sizeof(fileContents));

            bytesRead = 0;
            size_t n;
            uint8_t buf[64];

            while ((n = file.read(buf, sizeof(buf))) > 0)
            {
                memcpy(fileContents + bytesRead, buf, n);
                bytesRead += n;
            }

            file.close();

            return true;
        }
        else
        {
            Serial.print(filepath);
            Serial.println(F(" doesn't exist"));
        }
    }
    else
        Serial.print(F("Card not connected"));

    return false;
}




void Menu::readSDFile()
{
    char nameBuff[50];

    strcpy(nameBuff, pwd);

    if (nameBuff[strlen(nameBuff) - 1] != '/')
        strcat(nameBuff, "/");

    strncat(nameBuff, curOptName(), strlen(curOptName()));
    readFile(nameBuff);
    _fileRead = true;
}
