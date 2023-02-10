// Name: Jade Soto
// Homework: Homework 3: shell
// Class: COP4600
// Professor: Boloni

// Importing Standard Libraries
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <string>
#include <math.h>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fstream>
#include <iostream>
using namespace std;

// Global Vectors
std::vector<string> cHistory;
std::vector<pid_t> pidhistory;

// Used for EXECV command since it needs char array
char ** tokentoCharArray(string input)
{
  vector<string> tok;
  tok.clear();
  stringstream stream(input);
  int i = 0;

  // Add items to tok vector
  while(stream >> input)
  {
    tok.push_back(input);
  }
  int toksize = tok.size();
  char ** chararray = new char*[toksize + 1];

  // Add items to char array
  while(i < toksize)
  {
    chararray[i] = new char[tok.at(i).size()];
    strcpy(chararray[i], tok.at(i).c_str());
    i++;
  }
  // Make sure that the char array ends in a null pointer
  chararray[toksize] = (char *) 0;
  return chararray;
}

// Saves command history to mysh.history text file
void savetoFile()
{
  int histsize = cHistory.size();
  int i = histsize - 1;
  ofstream tmpfile;
  int j = 0;

  // Opens file
  tmpfile.open("mysh.history");
  if(!tmpfile)
  {
    ofstream tmpfile("mysh.history");
  }
  cout << "\n";

  // Writes to file
  while (i >= 0)
  {
    tmpfile << j << ": " << cHistory.at(i) << "\n";
    i--;
    j++;
  }
  tmpfile.close();
}

// Terminate a PID
string terminatePID(string shell, string a)
{
  pid_t termPID;
  pid_t cPID;
  int killed;
  int index;

  // Makes sure that PID is given
  if(a == "")
  {
    cout << "Error: No PID given\n";
    return shell;
  }
  termPID = (pid_t)stoi(a);

  // Captures index of PID erased
  for(int i = 0; i < pidhistory.size(); i++)
  {
    if(termPID == pidhistory.at(i) )
    {
      pidhistory.erase(pidhistory.begin() + i);
    }
  }

  // Given in pdf
  killed = kill(termPID, SIGKILL);
  // Catches if kill does not work
  if(killed < 0)
  {
    cout << "Fail to Terminate PID:" << a << "\n";
    return shell;
  }
  cout << "PID: " << a << " Terminated\n";
  return shell;
}

// Prints history / clear history
string history(string shell, string a)
{
  int j = 0;

  // Clear history
  if(a.compare("-c") == 0)
  {
    cout << "Clearing command history\n";
    cHistory.clear();
  }

  // Prints History
  else if(a == "")
  {
    int histsize = cHistory.size();
    int i = histsize - 1;
    if(histsize == 0)
    {
      cout << "There were no previous commands\n";
      return shell;
    }
    cout << "Command History\n";

    // Prints the history in reverse order
    while( j < histsize)
    {
      cout << j << ": " << cHistory.at(i) << "\n";
      j++;
      i--;
    }
  }
  // Ensures there are no other arguments
  else
    {
      cout << "Invalid arguements\n";
    }
    return shell;
  }

// Terminate all PID
string terminateall(string shell)
{
  int i = 0;
  int pidsize = pidhistory.size();
  int killed;
  pid_t cPID;

  // Checks to see if there are any PID to eliminate
  if(pidhistory.size() == 0)
  {
    cout << "No processes to terminate\n";
    return shell;
  }
  // Terminates PID and ensures they are killed
  cout << "Terminating " << pidsize << " processes\n";
  while (i < pidsize)
  {
    cPID = pidhistory.at(i);
    killed = kill (cPID, SIGKILL);
    if(killed < 0)
    {
      cout << "Fail to Terminate PID:" << cPID << "\n";
    }
    cout << "Terminating PID: " << cPID << " \n";
    i++;
  }
  cout << "PID History Cleared\n";
  pidhistory.clear();
  return shell;
}

string parse(string shell, string input)
{
  string command;
  string a = "";
  stringstream stream(input);

  // Records command history
  stream >> command;
  cHistory.push_back(input);
  // Incase byebye somehow gets passed
  if(command.compare("byebye") == 0)
  {
    return shell;
  }
  if(command.compare("history") == 0)
  {
    stream >> a;
    shell = history(shell, a);
    return shell;
  }
  // Was weird with it being seperate function so its in parse
  else if(command.compare("replay") == 0)
  {
    stream >> a;
    if(a == "")
    {
      cout << "Error: No number given to replay\n";
      return shell;
    }
    // Replaying a replay leads to weird stuff so I omit it from history
    cHistory.erase(cHistory.begin() + cHistory.size() -1);
    int histsize = cHistory.size();
    int argue = stoi(a);
    // Catches invalid input
    if(histsize == 0)
    {
      cout << "There is no previous commands stored this session to replay\n";
      return shell;
    }
    if(argue > histsize -1)
    {
      cout << "Invalid arguement\n";
      return shell;
    }
    if(argue < 0)
    {
      cout << "Invalid arguement\n";
      return shell;
    }
    int arguem =  histsize - argue - 1;
    shell = parse(shell, cHistory.at(arguem));
  }
  // Needed stringstream but hard to put in seperate function so I put it in the parse
  else if(command.compare("start") == 0)
  {
    stream >> a;
   if(a == "")
   {
     cout << "Error: No parameters given\n";
     return shell;
   }
    cout << "Starting program\n";
    int escape;
    string execute = "";
    string parameters;
    // Absolute Path
    if(a[0] == '/')
    {
      execute = a;
    }
    // Relative Path
    else
    {
      execute = shell + '/' + a;
    }
    const char * charexecute = execute.c_str();

    // Ensures that file exists
    if(access(charexecute, X_OK) != 0)
    {
      cout << "Error: file does not exist\n";
      return shell;
    }
      pid_t PID = fork();
      // Catch if fork causes error
      if(PID == -1)
      {
        cout << "Failed fork\n";
        return shell;
      }
      // Child stuff
      else if(PID == 0)
      {
        while(stream >> parameters)
        {
          a.append(" ");
          a.append(parameters);
        }
        char **executecommand = tokentoCharArray(a);

        // Executes a file
        execv(charexecute, executecommand);
      }
      else
      {
        // Ensures that current PID is finished
        waitpid(PID, &escape, WUNTRACED);
      }
        return shell;
  }
  // Needed stringstream but hard to put in seperate function
  // so I put it in the parse
  else if(command.compare("background") == 0)
  {
    stream >> a;
    int escape;
    string execute = "";
    string parameters;
    if(a == "")
    {
      cout << "Error: No parameters given\n";
      return shell;
    }
    if(a[0] == '/')
    {
      execute = a;
    }
    else
    {
      execute = shell + '/' + a;
    }
    const char *charexecute = execute.c_str();
    if(access(charexecute, X_OK) != 0)
    {
      cout << "Error: File does not exist\n";
      return shell;
    }
      pid_t PID = fork();
      // Catch if fork causes error
      if(PID == -1)
      {
        cout << "Fail fork\n";
        return shell;
      }

      // Child stuff
      else if(PID == 0)
      {
        while(stream >> parameters)
        {
          a.append(" ");
          a.append(parameters);
        }
          char **execvCommand = tokentoCharArray(a);

          // Executes a file
          execv(charexecute, execvCommand);
        }
        else
        {
          pidhistory.push_back(PID);
          cout<< "PID Process:" << PID << "\n";

          // Ensures that PID process is finished
          waitpid(PID, &escape, WNOHANG);
        }
        // Tried everything to get the # on the new line but will not work :(
          return shell;
  }
  else if(command.compare("terminate") == 0)
  {
    stream >> a;
    bool onlydigits = (a.find_first_not_of( "0123456789" ) == std::string::npos);
    if(onlydigits == false)
    {
      cout << "Error: Arguement contains a non-digit\n";
      return shell;
    }
    shell = terminatePID(shell, a);
    return shell;
  }
  else if(command.compare("terminateall") == 0)
  {
    shell = terminateall(shell);
    return shell;
  }
  else if(command.compare("repeat") == 0)
  {
    stream >> a;
    bool onlydigits = (a.find_first_not_of( "0123456789" ) == std::string::npos);
    if(onlydigits == false)
    {
      cout << "Error: No number parameter given";
      return shell;
    }
    int repeat = stoi(a);
    int i;
    stream >> a;
    for (i = 0; i < repeat; i++)
    {
      int escape;
      string execute = "";
      string parameters;
      if(a == "")
      {
        cout << "Error: No parameters given\n";
        return shell;
      }
      if(a[0] == '/')
      {
        execute = a;
      }
      else
      {
        execute = shell + '/' + a;
      }
      const char *charexecute = execute.c_str();
      if(access(charexecute, X_OK) != 0)
      {
        cout << "Error: File does not exist\n";
        return shell;
      }
        pid_t PID = fork();
        // Catch if fork causes error
        if(PID == -1)
        {
          cout << "Fail fork\n";
          return shell;
        }

        // Child stuff
        else if(PID == 0)
        {
          while(stream >> parameters)
          {
            a.append(" ");
            a.append(parameters);
          }
            char **execvCommand = tokentoCharArray(a);

            // Executes a file
            execv(charexecute, execvCommand);
          }
          else
          {
            pidhistory.push_back(PID);
            cout<< "PID Process:" << PID << "\n";
            // Ensures that PID process is finished
            waitpid(PID, &escape, WNOHANG);
          }
          // Tried everything to get the # on the new line but will not work :(
    }
    return shell;
  }
  else if(command.compare("dwelt") == 0)
  {
      stream >> a;
      const char *file;
      struct stat direct;
      file = a.c_str();

      // Checks to see if file is directory
      if(stat(file, &direct) == 0 && S_ISDIR(direct.st_mode))
      {
        printf("Abode is.\n");
        return shell;
      }

      // Checks to see if file is file
      if(stat(file, &direct) == 0 && S_ISREG(direct.st_mode))
      {
        printf("Dwelt indeed\n");
        return shell;
      }

      else
      {
        printf("Dwelt not\n");
      }
      return shell;
  }
  else if(command.compare("maik") == 0)
  {
    const char *file;
    FILE *test;
    struct stat direct;
    stream >> a;
    file = a.c_str();

    // Checks to see if file already exists
    if(test = fopen(file,"r"))
    {
      printf("Error: File already exists\n");
      fclose(test);
      return shell;
    }

      // Opens new file
      ofstream tmpfile;
      tmpfile.open(file);
      if(!tmpfile)
      {
        ofstream tmpfile(file);
      }

      // Writes in new file
      tmpfile << "Draft\n";
      tmpfile.close();
      return shell;
  }
  else if(command.compare("coppy") == 0)
  {
    // Obtains source file name
    stream.ignore(256,'-');
    stream >> a;
    const char *file1;
    const char *file2;
    ifstream source;
    file1 = a.c_str();
    source.open(file1);
    FILE *test;

    // Chekcs to see if source file exists
    if(!source)
    {
      cout << "ERROR: Source file does not exist\n";
      return shell;
    }

    // Obtains destination file name
    stream.ignore(256,'-');
    stream >> a;
    file2 = a.c_str();

    // Checks to see if destination file exists already
    if(test = fopen(file2,"r"))
    {
      printf("Error: Destination File already exists\n");
      fclose(test);
      source.close();
      return shell;
    }

    ofstream destination;
    destination.open(file2);
    if(!destination)
    {
      ofstream destination(file2);
    }
    string line;

    // Copies content from source file to destination file
    if(source && destination)
    {
      while(getline(source, line))
      {
        destination << line << "\n";
      }
    }
    source.close();
    destination.close();
  }
  else
  {
    // History does not store invalid commands
    cHistory.erase(cHistory.begin() + cHistory.size() -1);
    cout << "Command invalid\n";
  }
  return shell;
}


int main()
{
  char shelldirectory[1000];
  std::string input;
  string shell = getcwd(shelldirectory, sizeof(shelldirectory));
  cout << "Please enter commands in lowercase\n";
  cout << "If you are doing the coppy function do it like this: #coppy from-filename to-file2name\n";
  cout << "Enter command \n #";
  getline(cin, input);
  while(input.compare("byebye") != 0)
  {
    shell = parse(shell, input);
    cout <<"\n #";
    getline(cin, input);
  }
  // Saves byebye to history
  cHistory.push_back(input);
  // Saves history to text file
  savetoFile();
  return 0;
}
