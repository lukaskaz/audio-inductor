#include "speechtexts.hpp"

#include <algorithm>
#include <ranges>
#include <stdexcept>
#include <unordered_map>

namespace tospeech
{

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
         {tts::language::english, "choose what to do"},
         {tts::language::german, "fertig zum laufen"},
     }},
    {task::voicecontrolmode,
     {
         {tts::language::polish,
          "inicjuje tryb sterowania głosowego, podaj komendę"},
         {tts::language::english,
          "initiating voice controle mode, say what to do"},
         {tts::language::german, "fertig zum laufen"},
     }},
    {task::testrunstart,
     {
         {tts::language::polish, "uwaga, rozpoczynam procedurę testową"},
         {tts::language::english, "starting test run"},
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
    {task::getval,
     {
         {tts::language::polish, "ustawione na "},
         {tts::language::english, "set to "},
         {tts::language::german, "fertig zum laufen"},
     }},
    {task::howmuch,
     {
         {tts::language::polish, "na ile?"},
         {tts::language::english, "how much?"},
         {tts::language::german, "fertig zum laufen"},
     }},
    {task::byhowmuch,
     {
         {tts::language::polish, "o ile?"},
         {tts::language::english, "by how much?"},
         {tts::language::german, "fertig zum laufen"},
     }},
    {task::keepcalm,
     {
         {tts::language::polish, "wyluzuj, jest git"},
         {tts::language::english, "just take it easy, boy"},
         {tts::language::german, "fertig zum laufen"},
     }},
    {task::ibehave,
     {
         {tts::language::polish, "będę grzeczna kurde!"},
         {tts::language::english, "I will behave I swear!"},
         {tts::language::german, "fertig zum laufen"},
     }},
    {task::dontgetit,
     {
         {tts::language::polish, "noż kurwa, nie rozumiem!"},
         {tts::language::english, "fuck it, I don't get it!"},
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

std::string gettext(task what, tts::language lang)
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

} // namespace tospeech

namespace tospoken
{

enum class rigor
{
    exact,
    loose
};

using langmap =
    const std::unordered_map<stt::language, std::pair<rigor, std::string>>;
const std::unordered_map<spoken, langmap> textspokenhmap = {
    {spoken::yes,
     {
         {stt::language::polish, {rigor::exact, "tak"}},
         {stt::language::english, {rigor::exact, "yes"}},
         {stt::language::german, {rigor::exact, "ja"}},
     }},
    {spoken::no,
     {
         {stt::language::polish, {rigor::exact, "nie"}},
         {stt::language::english, {rigor::exact, "no"}},
         {stt::language::german, {rigor::exact, "nein"}},
     }},
    {spoken::whatsvulg,
     {
         {stt::language::polish, {rigor::loose, "jaką masz wulgarność"}},
         {stt::language::english, {rigor::loose, "what is your vulgarity"}},
         {stt::language::german, {rigor::loose, "###"}},
     }},
    {spoken::decreasevulg,
     {
         {stt::language::polish, {rigor::loose, "zmniejsz wulgarność"}},
         {stt::language::english, {rigor::loose, "decrease vulgarity"}},
         {stt::language::german, {rigor::loose, "###"}},
     }},
    {spoken::wantless,
     {
         {stt::language::polish, {rigor::loose, "chcesz mniej"}},
         {stt::language::english, {rigor::loose, "you want less"}},
         {stt::language::german, {rigor::loose, "###"}},
     }},
    {spoken::runtest,
     {
         {stt::language::polish, {rigor::loose, "uruchom test"}},
         {stt::language::english, {rigor::loose, "run test"}},
         {stt::language::german, {rigor::loose, "###"}},
     }},
    {spoken::exitroutine,
     {
         {stt::language::polish, {rigor::exact, "zakończ"}},
         {stt::language::english, {rigor::exact, "complete"}},
         {stt::language::german, {rigor::exact, "###"}},
     }},
    {spoken::exitprogram,
     {
         {stt::language::polish, {rigor::exact, "zakończ program"}},
         {stt::language::english, {rigor::exact, "exit application"}},
         {stt::language::german, {rigor::exact, "###"}},
     }},
};

std::string strlower(std::string_view sv)
{
    std::string ret;
    std::ranges::for_each(sv, [&ret](char c) { ret += std::tolower(c); });
    return ret;
}

bool isnumber(std::string_view sv)
{
    return !sv.empty() &&
           std::ranges::all_of(sv, [](auto c) { return std::isdigit(c); });
}

spoken getspoken(std::string_view text, stt::language lang)
{
    if (isnumber(text))
        return spoken::numerical;
    auto ret = std::ranges::find_if(
        textspokenhmap,
        [&text, lang](const auto& map) {
            if (map.contains(lang))
            {
                auto& [rig, key] = map.at(lang);
                auto str = strlower(text);
                return rig == rigor::exact ? str == key
                                           : str.find(key) != std::string::npos;
            }
            return false;
        },
        &decltype(textspokenhmap)::value_type::second);
    return ret != textspokenhmap.end() ? ret->first : spoken::unknown;
}

} // namespace tospoken
