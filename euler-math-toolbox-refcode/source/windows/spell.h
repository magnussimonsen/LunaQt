#ifndef SPELL_H
#define SPELL_H

void dospellcheck ();
void doaddwords ();
void dounmarkspellerrors ();
int findword (char *word);
void addword (char *word);
void sortwords ();
void resetspell ();

#endif