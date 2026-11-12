#pragma once

#include <string>

// True for the second and later bytes of a UTF-8 character.
bool isUtf8Continuation(char c);

// Removes a last character whose bytes are not all there, e.g. cut by a length limit.
void trimIncompleteUtf8(std::string& text);
