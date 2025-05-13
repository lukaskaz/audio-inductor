#pragma once

#include "speech/stt/interfaces/textfromvoice.hpp"
#include "speech/tts/interfaces/texttovoice.hpp"

#include <string>

namespace tospeech
{

enum class task
{
    initiatating,
    ready,
    selectmode,
    voicecontrolmode,
    testrunstart,
    testrunend,
    analyzestart,
    analyzeend,
    howmuch,
    byhowmuch,
    getval,
    ibehave,
    keepcalm,
    dontgetit,
    exitingapp,
    programexit
};

std::string gettext(task, tts::language);

} // namespace tospeech

namespace tospoken
{

enum class spoken
{
    yes,
    no,
    numerical,
    whatsvulg,
    decreasevulg,
    wantless,
    runtest,
    exitroutine,
    exitprogram,
    unknown
};

spoken getspoken(std::string_view, stt::language);

} // namespace tospoken
