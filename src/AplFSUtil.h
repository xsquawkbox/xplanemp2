//
// Created by kuroneko on 5/03/2018.
//

#ifndef APLFSUTIL_H
#define APLFSUTIL_H

#ifdef APL
int Posix2HFSPath(const char *path, char *result, int resultLen);
int HFS2PosixPath(const char *path, char *result, int resultLen);
#endif

#endif //APLFSUTIL_H
