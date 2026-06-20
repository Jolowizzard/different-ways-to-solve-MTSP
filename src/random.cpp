#include "random.h"

std::random_device rd;
std::mt19937 gen(rd());