#pragma once

#define MAX 256
class AOD_IO_CLASS;
class AE_IO_CLASS;
static const unsigned long long FBXframe1 = 1924423250;			// First frame (valore arbitrario del file FBX associato al frame numero 1)
extern int MaxThreadLimit;
extern AOD_IO_CLASS AOD_IO;
extern AE_IO_CLASS AE_IO;

using namespace std;

#include <cassert>
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <string>
#include <strstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <thread>
#include <atomic>
#include <mutex>
#include <algorithm>
#include <ctime>
#include "windows.h"
#include "picosha2.h"
#include "atlstr.h"
extern ofstream msg_file_stream;
#include "OutMsgInterface.h"
extern mutex mu;