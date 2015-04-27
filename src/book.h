#ifndef BOOK_H
#define BOOK_H

#include "position.h"

bool ReadPGN(const string& file_in, string& file_out);
void MakeFakeFile(const string& file_out);
bool ReadPGNAndSaveMatTable(const string& file_in, string& file_out);

#endif
