#include "ScreenLog.h"
#include "Utils.h"
#include "CFont.h"
#include "CTimer.h"

ScreenLog::ScreenLog()
{
    scrollOffset = 0.0f;

    Init();
}

void ScreenLog::Init()
{
    // load settings from ini file
    auto config = GetConfigFilename();

    level = (eLogLevel)GetPrivateProfileInt("ScreenLog", "Level", (UINT)eLogLevel::None, config.c_str());
    maxMessages = GetPrivateProfileInt("ScreenLog", "MessagesMax", 40, config.c_str());
    timeDisplay = GetPrivateProfileInt("ScreenLog", "MessageTime", 6000, config.c_str());
    timeFadeout = 3000;
    fontSize = 0.01f * GetPrivateProfileInt("ScreenLog", "FontSize", 60, config.c_str());
}

void ScreenLog::Add(eLogLevel level, const char* msg)
{
    if (level > this->level)
    {
        return;
    }

    entries.emplace_front(level, msg, timeDisplay);

    if (entries.size() > maxMessages)
    {
        entries.resize(maxMessages);
    }

    // update scroll pos
    float sizeY = fontSize * RsGlobal.maximumHeight / 448.0f;
    size_t lines = CountLines(std::string(msg));
    scrollOffset += 18.0f * lines * sizeY;
}

void ScreenLog::Draw()
{
    // scroll animation
    static DWORD prevTime;
    DWORD currTime = GetTickCount(); // game independent
    if (scrollOffset > 0.001f)
    {
        float delta = 0.01f * (currTime - prevTime);
        scrollOffset *= max(0.9f - delta, 0.0f);
    }
    else
        scrollOffset = 0.0f;
    prevTime = currTime;

    // clean up expired entries
    while(!entries.empty())
    {
        if(entries.back().timeLeft < (-0.001f * timeFadeout))
            entries.pop_back();
        else
            break;
    }

    if (entries.empty())
    {
        scrollOffset = 0.0f;
        return; // nothing to print
    }

    CFont::SetBackground(false, false);
    CFont::SetWrapx(99999999.0f); // no line wrap
    CFont::SetFontStyle(FONT_SUBTITLES);
    CFont::SetEdge(1);
    CFont::SetProportional(true);

    const float aspect = (float)RsGlobal.maximumWidth / RsGlobal.maximumHeight;
    float sizeX = fontSize * 0.55f * RsGlobal.maximumWidth / 640.0f / aspect;
    float sizeY = fontSize * RsGlobal.maximumHeight / 448.0f;
    CFont::SetScale(sizeX, sizeY);

    CFont::SetOrientation(ALIGN_LEFT);
    float posX = 15.0f * sizeX;
    float posY = 7.0f * sizeY - scrollOffset;

    // count total lines
    int lines = 0;
    for (auto& entry : entries)
    {
        lines += CountLines(entry.msg);
    }

    float elapsed = 0.001f * (CTimer::m_snTimeInMilliseconds - CTimer::m_snPreviousTimeInMilliseconds);
    float rowTime = -0.001f * timeFadeout;
    for(auto it = entries.rbegin(); it != entries.rend(); it++)
    {
        auto& entry = *it;

        if(entry.timeLeft > 0.0f && entry.timeLeft < elapsed)
            entry.timeLeft = 0.0f; // do not skip fade
        else
            entry.timeLeft -= elapsed;

        rowTime = max(rowTime, entry.timeLeft); // carred on from older entries
        
        BYTE alpha = 255;
        if (rowTime < 0)
        {
            float fadeProgress = -rowTime / (0.001f * timeFadeout);
            fadeProgress = std::clamp(fadeProgress, 0.0f, 1.0f);
            fadeProgress = 1.0f - fadeProgress; // fade out
            fadeProgress = sqrtf(fadeProgress);
            alpha = (BYTE)(fadeProgress * 0xFF);
        };

        auto color = fontColor[(size_t)entry.level];
        alpha = min(alpha, color.a);
        color.a = alpha;

        CFont::SetColor(color);

        alpha = std::clamp(int(alpha * alpha) / 255, 0, 255); // corrected for fadeout
        CFont::SetDropColor(CRGBA(0, 0, 0, alpha));

        lines -= CountLines(entry.msg);
        float y = posY + 18.0f * sizeY * lines;
        CFont::PrintString(posX, y, entry.msg.c_str());
    }
}

void ScreenLog::DrawLine(const char* msg, size_t row)
{
    CFont::SetBackground(false, false);
    CFont::SetWrapx(99999999.0f); // no line wrap
    CFont::SetFontStyle(FONT_SUBTITLES);
    CFont::SetEdge(1);
    CFont::SetProportional(true);

    const float aspect = (float)RsGlobal.maximumWidth / RsGlobal.maximumHeight;
    float sizeX = fontSize * 0.55f * RsGlobal.maximumWidth / 640.0f / aspect;
    float sizeY = fontSize * RsGlobal.maximumHeight / 448.0f;
    CFont::SetScale(sizeX, sizeY);

    CFont::SetOrientation(ALIGN_RIGHT);
    float posX = (float)RsGlobal.maximumWidth - 15.0f * sizeX;
    
    //if(FrontEndMenuManager.m_bHudOn)
    float posY = 0.25f * RsGlobal.maximumHeight;
    posY += 18.0f * sizeY * row;

    auto color = fontColor[(size_t)eLogLevel::Error];
    CFont::SetColor(color);
    CFont::SetDropColor(CRGBA(0, 0, 0, color.a));

    CFont::PrintString(posX, posY, msg);
}

size_t ScreenLog::CountLines(std::string& msg)
{
    size_t lines = 1;

    size_t pos = 0;
    while ((pos = msg.find("~n~", pos)) != std::string::npos)
    {
        lines++;
        pos += 3; // pattern length
    }

    lines += std::count(msg.begin(), msg.end(), '\n');

    return lines;
}

DWORD ScreenLog::GetTime()
{
    //return GetTickCount();
    return CTimer::m_snPreviousTimeInMillisecondsNonClipped;
}
