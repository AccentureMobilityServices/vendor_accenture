#include <stdio.h>
#include <string>
#include "gles2_emulator_constants.h"

using namespace std;


void writeToFile(string fileName, string content) {
	FILE*p = fopen(fileName.c_str(),"w");
	fwrite(content.c_str(), 1, content.length(), p);
	fclose(p);
}

string preprocessor(string input, int shaderNum) 
{
    char line[500];
	char shaderInput[100];
	char shaderOutput[100];

	sprintf(shaderInput,"/tmp/shaderinput%d",shaderNum);
	sprintf(shaderOutput,"/tmp/shaderoutput%d",shaderNum);
	shaderNum++;

    int i = 0;
	string outStr;

	writeToFile(shaderInput, input);

	FILE* p = popen((string(PREPROCESSOR_NAME)+string(" ")+string(shaderInput)).c_str(), "r");
    
	if (p != NULL )
    {
		while ( fgets ( line, sizeof line, p ) != NULL ) {/* read a line */
			if (line[0]!='#' && line[0]!='\n') 
            	outStr += line;
        }
        pclose(p);
		writeToFile(shaderOutput, outStr);
        return outStr;
    }
    else
    {
        return "\n";
    }
}
/*
int main(int argc, char* argv) {
	string test= "one\ntwo\nthree    \n four";
	string out = preprocessor(test);
	printf("%s\n",out.c_str());

}
*/
