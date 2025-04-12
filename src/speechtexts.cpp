#include "speechtexts.hpp"

#include <stdexcept>
#include <unordered_map>

using langmap = const std::unordered_map<tts::language, std::string>;
const std::unordered_map<task, langmap> taskspeechmap = {
    {task::initiatating,
     {
         {tts::language::polish, "rozpoczynam inicjalizację"},
         {tts::language::english, "initializing"},
         {tts::language::german, "initiieren"},
     }},
    {task::ready,
     {
         {tts::language::polish, "interpreter muzyki gotowy do działania"},
         {tts::language::english, "music interpreter ready for action"},
         {tts::language::german, "fertig zum laufen"},
     }},
    {task::selectmode,
     {
         {tts::language::polish, "czekam na wybór zadania"},
         {tts::language::english, "waiting to choose what to do"},
         {tts::language::german, "fertig zum laufen"},
     }},
    {task::testrunstart,
     {
         {tts::language::polish,
          "uwaga, rozpoczynam procedurę testową, pasuje?"},
         {tts::language::english, "starting test run, you ok with it?"},
         {tts::language::german, "fertig zum laufen"},
     }},
    {task::testrunend,
     {
         {tts::language::polish, "no i zakończyłam procedurę testową, hura!"},
         {tts::language::english,
          "and so i have completed the test run, yippee!"},
         {tts::language::german, "fertig zum laufen"},
     }},
    {task::analyzestart,
     {
         {tts::language::polish, "startuję z interpreterem muzyki"},
         {tts::language::english, "starting music interpretation"},
         {tts::language::german, "fertig zum laufen"},
     }},
    {task::analyzeend,
     {
         {tts::language::polish,
          "przerwałeś analizę dzwięku, a tak ładnie grało kurczę"},
         {tts::language::english, "you have interrupted the analysis, too bad"},
         {tts::language::german, "fertig zum laufen"},
     }},
    {task::programexit,
     {
         {tts::language::polish,
          "Buuu nie chcem, ale jak trzeba to zamykam się :("},
         {tts::language::english,
          "Boo i dont want to exit, but if you insist :("},
         {tts::language::german, "fertig zum laufen"},
     }}};

std::string getspeechtext(task what, tts::language lang)
{
    if (taskspeechmap.contains(what))
    {
        const auto& langmap = taskspeechmap.at(what);
        if (langmap.contains(lang))
            return langmap.at(lang);
        throw std::runtime_error("Given language for speech not available");
    }
    throw std::runtime_error("Given task for speech not available");
}
