#include <iostream>
#include "Tokenizer.h"

using namespace std;

Tokenizer::Tokenizer (const string _input)
    : error(false), is_expansion(false)
{
    input = trim(_input);
    split("|");
    bg_proc = (_input.back() == char('&'));
}

Tokenizer::Tokenizer (const string _input, vector<string>& __inner_strings)
    : error(false), is_expansion(true)
{
    inner_strings = __inner_strings;
    input = trim(_input);
    split("|");
    bg_proc = (_input.back() == char('&'));
}

Tokenizer::~Tokenizer () {
    for (auto cmd : commands) {
        delete cmd;
    }
    commands.clear();
}

bool Tokenizer::hasError () {
    return error;
}

string Tokenizer::trim (const string in) {
    int i = in.find_first_not_of(" \n\r\t");
    int j = in.find_last_not_of(" \n\r\t");

    if (i >= 0 && j >= i) {
        return in.substr(i, j-i+1);
    }
    return in;
}

/* Remove the sign expansions and place them into
    an external vector for later processing     */
void Tokenizer::remove_se ( std::string& line )
{
    size_t index = 0;
    while (line.find("$(") != string::npos) 
    {
        size_t start = 0;
        size_t end = 0;
        if (line.find(")") != string::npos) {
            start = line.rfind("$(");
            end = line.find(")", start + 1);
            if (end == string::npos) {
                error = true;
                cerr << "Invalid command - No closing parenthesis on `$(`" << endl;
                return;
            }
        }

        inner_sign_expansions.push_back(line.substr(start + 2, end - start - 2));

        string str_beg = line.substr(0, start);
        string str_mid = "--tkn" + to_string(index) + "nkt--";
        string str_end = line.substr(end + 1);
        line = str_beg + str_mid + str_end;

        ++index;
    }
}

void Tokenizer::split (const string delim) {
    string temp = input;
    
    int index = 0;
    while (temp.find("\"") != string::npos || temp.find("\'") != string::npos) {
        int start = 0;
        int end = 0;
        if (temp.find("\"") != string::npos
            && (temp.find("\'") == string::npos || temp.find("\"") < temp.find("\'"))) {
            start = temp.find("\"");
            end = temp.find("\"", start+1);
            if ((size_t) end == string::npos) {
                error = true;
                cerr << "Invalid command - Non-matching quotation mark on \"" << endl;
                return;
            }
        }
        else if (temp.find("\'") != string::npos) {
            start = temp.find("\'");
            end = temp.find("\'", start+1);
            if ((size_t) end == string::npos) {
                error = true;
                cerr << "Invalid command - Non-matching quotation mark on \'" << endl;
                return;
            }
        }
        
        inner_strings.push_back(temp.substr(start+1, end-start-1));
        
        string str_beg = temp.substr(0, start);
        string str_mid = "--str " + to_string(index);
        string str_end = temp.substr(end+1);
        temp = str_beg + str_mid + str_end;

        index++;
    }

    remove_se(temp);
    // INFO(temp);

	size_t i = 0;
	while ((i = temp.find(delim)) != string::npos) {
        string cur = trim(temp.substr(0, i));
        if (cur.find("--tkn") != string::npos) se_locations.push_back(commands.size());
		commands.push_back(new Command(cur, inner_strings, is_expansion));
		temp = trim(temp.substr(i+1));
	}
    if (temp.find("--tkn") != string::npos) se_locations.push_back(commands.size());
	commands.push_back(new Command(trim(temp), inner_strings, is_expansion));
}
