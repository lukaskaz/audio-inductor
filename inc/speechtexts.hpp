#pragma once

#include "speech/tts/interfaces/texttovoice.hpp"

#include <string>

enum class task
{
    initiatating,
    ready,
    selectmode,
    testrunstart,
    testrunend,
    analyzestart,
    analyzeend,
    programexit
};

std::string getspeechtext(task, tts::language);
