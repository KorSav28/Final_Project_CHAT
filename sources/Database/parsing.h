#pragma once
#include <string>

using namespace std;

bool startWith(string tmpl, string& s);

bool startWith(string tmpl, const char* s);

int findFirst(string tmpl, string& s);

string splitBy(string tmpl, string& s);

void skipuntil(string tmpl, string& s);

string segment(string from, string to, string s);

bool isLetter(char c);

bool isDigit(char c);

bool correctName(string s);
