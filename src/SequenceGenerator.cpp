#include "SequenceGenerator.h"
#include <atomic>

static std::atomic_int s_seq(0);

int GenerateSequence()
{
    return ++s_seq;
}