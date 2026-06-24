#include "TextEditorPanel.h"

#include <imgui.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>

namespace fs = std::filesystem;

void TextEditorPanel::setup()
{
    m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::None);
    m_editor.SetPalette(TextEditor::PaletteId::Dark);
    m_editor.SetShowLineNumbersEnabled(true);
    m_editor.SetShowWhitespacesEnabled(false);
    m_editor.SetLineSpacing(1.0f);
}

void TextEditorPanel::setDialogCallbacks(
    std::function<void(const std::string& key,
                       const std::string& title,
                       const std::string& filters,
                       ConfirmPath onConfirm)> openFile,
    std::function<void(const std::string& key,
                       const std::string& title,
                       const std::string& filters,
                       const std::string& defaultFileName,
                       ConfirmPath onConfirm)> saveFile)
{
    m_openFile = std::move(openFile);
    m_saveFile = std::move(saveFile);
}

void TextEditorPanel::setText(const std::string& text,
                              TextEditor::LanguageDefinitionId lang)
{
    if (lang != TextEditor::LanguageDefinitionId::None)
        m_editor.SetLanguageDefinition(lang);
    m_editor.SetText(text);
}

std::string TextEditorPanel::getText() const
{
    return m_editor.GetText();
}

void TextEditorPanel::setLanguage(TextEditor::LanguageDefinitionId lang)
{
    m_editor.SetLanguageDefinition(lang);
}

int TextEditorPanel::getUndoIndex() const
{
    return m_editor.GetUndoIndex();
}

void TextEditorPanel::setSidebarEntries(std::vector<SidebarEntry> entries)
{
    m_sidebarEntries = std::move(entries);
    m_sidebarSelected = -1;
}

int TextEditorPanel::getLineCount() const
{
    return std::max(1, m_editor.GetLineCount());
}

int TextEditorPanel::getCursorLine() const
{
    int line = 0, col = 0;
    m_editor.GetCursorPosition(line, col);
    return line;
}

void TextEditorPanel::setCursorLine(int line)
{
    line = std::clamp(line, 0, getLineCount() - 1);
    m_editor.SetCursorPosition(line, 0);
}

void TextEditorPanel::setHighlightLine(int line)
{
    m_highlightLine = line;
    if (line < 0) {
        m_editor.ClearSelections();
        return;
    }
    line = std::clamp(line, 0, getLineCount() - 1);
    m_editor.SelectLine(line);
    m_editor.SetViewAtLine(line, TextEditor::SetViewAtLineMode::Centered);
}

void TextEditorPanel::draw(bool& visible)
{
    ImGui::SetNextWindowSize(ImVec2(820, 580), ImGuiCond_FirstUseEver);
    ImGuiWindowFlags winFlags = ImGuiWindowFlags_MenuBar
                              | ImGuiWindowFlags_NoFocusOnAppearing;
    if (!ImGui::Begin("Code Editor###ofxkit.window.code_editor", &visible, winFlags)) {
        ImGui::End();
        return;
    }

    auto detectLanguage = [this](const std::string& path) {
        std::string ext = fs::path(path).extension().string();
        if (!ext.empty() && ext.front() == '.')
            ext.erase(0, 1);
        if (ext == "glsl" || ext == "vert" || ext == "frag")
            m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Glsl);
        else if (ext == "hlsl")
            m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Hlsl);
        else if (ext == "cpp" || ext == "c" || ext == "h" || ext == "hpp")
            m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Cpp);
        else if (ext == "cs")
            m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Cs);
        else if (ext == "lua")
            m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Lua);
        else if (ext == "py")
            m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Python);
        else if (ext == "json")
            m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Json);
        else if (ext == "xml" || ext == "svg")
            m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Xml);
        else if (ext == "sql")
            m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Sql);
        else if (ext == "gcode" || ext == "nc" || ext == "cnc" || ext == "tap")
            m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Gcode);
        else if (ext == "md" || ext == "markdown")
            m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Markdown);
        else
            m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::None);
    };

    // Returns {filter string, default filename} based on the current language.
    auto getDialogParams = [this]() -> std::pair<std::string, std::string> {
        auto lang = m_editor.GetLanguageDefinition();
        if (lang == TextEditor::LanguageDefinitionId::Gcode)
            return {"G-code{.gcode,.nc,.cnc,.tap},All Files{.*}", "untitled.gcode"};
        if (lang == TextEditor::LanguageDefinitionId::Glsl)
            return {"Shaders{.glsl,.vert,.frag,.hlsl},All Files{.*}", "untitled.glsl"};
        if (lang == TextEditor::LanguageDefinitionId::Hlsl)
            return {"Shaders{.hlsl,.vert,.frag,.glsl},All Files{.*}", "untitled.hlsl"};
        if (lang == TextEditor::LanguageDefinitionId::Cpp || lang == TextEditor::LanguageDefinitionId::C)
            return {"C/C++{.cpp,.c,.h,.hpp},All Files{.*}", "untitled.cpp"};
        if (lang == TextEditor::LanguageDefinitionId::Cs)
            return {"C#{.cs},All Files{.*}", "untitled.cs"};
        if (lang == TextEditor::LanguageDefinitionId::Python)
            return {"Python{.py},All Files{.*}", "untitled.py"};
        if (lang == TextEditor::LanguageDefinitionId::Lua)
            return {"Lua{.lua},All Files{.*}", "untitled.lua"};
        if (lang == TextEditor::LanguageDefinitionId::Json)
            return {"JSON{.json},All Files{.*}", "untitled.json"};
        if (lang == TextEditor::LanguageDefinitionId::Xml)
            return {"XML{.xml,.svg},All Files{.*}", "untitled.xml"};
        if (lang == TextEditor::LanguageDefinitionId::Sql)
            return {"SQL{.sql},All Files{.*}", "untitled.sql"};
        if (lang == TextEditor::LanguageDefinitionId::Markdown)
            return {"Markdown{.md,.markdown},All Files{.*}", "untitled.md"};
        return {"Source{.cpp,.h,.hpp,.c,.glsl,.vert,.frag,.hlsl,.py,.lua},"
                "G-code{.gcode,.nc,.cnc},"
                "Data{.json,.xml,.yaml,.txt,.md},"
                "All Files{.*}", "untitled.txt"};
    };

    auto doOpen = [&] {
        if (!m_openFile)
            return;
        m_openFile("code_open", "Open File",
                   "Source{.cpp,.h,.hpp,.c,.cs,.py,.lua,.js,.ts},"
                   "Shaders{.glsl,.vert,.frag,.hlsl},"
                   "G-code{.gcode,.nc,.cnc,.tap},"
                   "Data{.json,.xml,.yaml,.txt,.md},"
                   "All Files{.*}",
                   [this, detectLanguage](const std::string& path) {
                       std::ifstream ifs(path);
                       if (ifs) {
                           m_filePath = path;
                           m_editor.SetText(std::string(
                               std::istreambuf_iterator<char>(ifs),
                               std::istreambuf_iterator<char>()));
                           detectLanguage(path);
                       }
                   });
    };

    auto doSave = [&] {
        if (!m_saveFile) {
            if (!m_filePath.empty()) {
                std::ofstream ofs(m_filePath);
                if (ofs)
                    ofs << m_editor.GetText();
            }
            return;
        }
        if (m_filePath.empty()) {
            auto [filter, defaultName] = getDialogParams();
            m_saveFile("code_save", "Save As", filter, defaultName,
                       [this](const std::string& path) {
                           m_filePath = path;
                           std::ofstream ofs(path);
                           if (ofs)
                               ofs << m_editor.GetText();
                       });
        } else {
            std::ofstream ofs(m_filePath);
            if (ofs)
                ofs << m_editor.GetText();
        }
    };

    auto doSaveAs = [&] {
        if (!m_saveFile)
            return;
        auto [filter, defaultName] = getDialogParams();
        const std::string fname = m_filePath.empty()
                                      ? defaultName
                                      : fs::path(m_filePath).filename().string();
        m_saveFile("code_save_as", "Save As", filter, fname,
                   [this](const std::string& path) {
                       m_filePath = path;
                       std::ofstream ofs(path);
                       if (ofs)
                           ofs << m_editor.GetText();
                   });
    };

    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
        if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_N)) {
            m_editor.SetText("");
            m_filePath = "";
        }
        if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_O))
            doOpen();
        if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_S)) {
            if (ImGui::GetIO().KeyShift)
                doSaveAs();
            else
                doSave();
        }
        handleFindReplaceShortcuts();
    }

    if (ImGui::BeginMenuBar()) {

        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) {
                m_editor.SetText("");
                m_filePath = "";
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Open...", "Ctrl+O"))
                doOpen();
            ImGui::Separator();
            bool noPath = m_filePath.empty();
            ImGui::BeginDisabled(noPath || m_editor.IsReadOnlyEnabled());
            if (ImGui::MenuItem("Save", "Ctrl+S"))
                doSave();
            ImGui::EndDisabled();
            if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
                doSaveAs();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            bool ro = m_editor.IsReadOnlyEnabled();
            ImGui::BeginDisabled(!m_editor.CanUndo() || ro);
            if (ImGui::MenuItem("Undo", "Ctrl+Z"))
                m_editor.Undo();
            ImGui::EndDisabled();
            ImGui::BeginDisabled(!m_editor.CanRedo() || ro);
            if (ImGui::MenuItem("Redo", "Ctrl+Y"))
                m_editor.Redo();
            ImGui::EndDisabled();
            ImGui::Separator();
            if (ImGui::MenuItem("Find", "Ctrl+F"))
                showFind(false);
            if (ImGui::MenuItem("Find & Replace", "Ctrl+Shift+F"))
                showFind(true);
            ImGui::Separator();
            if (ImGui::MenuItem("Read Only", nullptr, ro))
                m_editor.SetReadOnlyEnabled(!ro);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            if (ImGui::BeginMenu("Language")) {
                const struct {
                    const char* label;
                    TextEditor::LanguageDefinitionId id;
                } kLangs[] = {
                    {"None", TextEditor::LanguageDefinitionId::None},
                    {"C++", TextEditor::LanguageDefinitionId::Cpp},
                    {"C", TextEditor::LanguageDefinitionId::C},
                    {"C#", TextEditor::LanguageDefinitionId::Cs},
                    {"Python", TextEditor::LanguageDefinitionId::Python},
                    {"Lua", TextEditor::LanguageDefinitionId::Lua},
                    {"JSON", TextEditor::LanguageDefinitionId::Json},
                    {"XML", TextEditor::LanguageDefinitionId::Xml},
                    {"SQL", TextEditor::LanguageDefinitionId::Sql},
                    {"AngelScript", TextEditor::LanguageDefinitionId::AngelScript},
                    {"GLSL", TextEditor::LanguageDefinitionId::Glsl},
                    {"HLSL", TextEditor::LanguageDefinitionId::Hlsl},
                    {"G-code", TextEditor::LanguageDefinitionId::Gcode},
                    {"Markdown", TextEditor::LanguageDefinitionId::Markdown},
                };
                auto curLang = m_editor.GetLanguageDefinition();
                for (auto& l : kLangs)
                    if (ImGui::MenuItem(l.label, nullptr, curLang == l.id))
                        m_editor.SetLanguageDefinition(l.id);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Colour Theme")) {
                const struct {
                    const char*           label;
                    TextEditor::PaletteId id;
                } kPals[] = {
                    {"Dark", TextEditor::PaletteId::Dark},
                    {"Light", TextEditor::PaletteId::Light},
                    {"Mariana", TextEditor::PaletteId::Mariana},
                    {"Retro Blue", TextEditor::PaletteId::RetroBlue},
                };
                auto curPal = m_editor.GetPalette();
                for (auto& p : kPals)
                    if (ImGui::MenuItem(p.label, nullptr, curPal == p.id))
                        m_editor.SetPalette(p.id);
                ImGui::EndMenu();
            }
            ImGui::Separator();
            bool lineNums = m_editor.IsShowLineNumbersEnabled();
            if (ImGui::MenuItem("Line Numbers", nullptr, lineNums))
                m_editor.SetShowLineNumbersEnabled(!lineNums);
            bool autoInd = m_editor.IsAutoIndentEnabled();
            if (ImGui::MenuItem("Auto Indent", nullptr, autoInd))
                m_editor.SetAutoIndentEnabled(!autoInd);
            bool showWS = m_editor.IsShowWhitespacesEnabled();
            if (ImGui::MenuItem("Show Whitespace", nullptr, showWS))
                m_editor.SetShowWhitespacesEnabled(!showWS);
            bool shortTabs = m_editor.IsShortTabsEnabled();
            if (ImGui::MenuItem("Short Tabs", nullptr, shortTabs))
                m_editor.SetShortTabsEnabled(!shortTabs);
            ImGui::Separator();
            {
                int tabSz = m_editor.GetTabSize();
                ImGui::SetNextItemWidth(60.f);
                if (ImGui::DragInt("Tab Size", &tabSz, 1.f, 1, 8))
                    m_editor.SetTabSize(tabSz);
            }
            {
                float ls = m_editor.GetLineSpacing();
                ImGui::SetNextItemWidth(60.f);
                if (ImGui::DragFloat("Line Spacing", &ls, 0.05f, 1.0f, 2.0f, "%.2f"))
                    m_editor.SetLineSpacing(ls);
            }
            ImGui::EndMenu();
        }

        if (!m_filePath.empty()) {
            const std::string fname = fs::path(m_filePath).filename().string();
            float           fW      = ImGui::CalcTextSize(fname.c_str()).x + 8.f;
            float           avail     = ImGui::GetContentRegionAvail().x;
            if (avail > fW)
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + avail - fW);
            ImGui::TextDisabled("%s", fname.c_str());
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", m_filePath.c_str());
        }

        ImGui::EndMenuBar();
    }

    if (m_toolbarDraw) {
        m_toolbarDraw();
        ImGui::Separator();
    }

    m_findReplace.draw(m_editor);

    const float statusH = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y;
    const float sidebarW = m_sidebarEntries.empty() ? 0.f : 180.f;

    if (sidebarW > 0.f) {
        ImGui::BeginChild("##code_sidebar", ImVec2(sidebarW, -statusH), ImGuiChildFlags_Borders);
        for (int i = 0; i < (int)m_sidebarEntries.size(); ++i) {
            auto& e = m_sidebarEntries[i];
            const bool active = e.isActive || m_sidebarSelected == i;
            ImGui::PushID(i);
            if (ImGui::Selectable(e.label.c_str(), active, ImGuiSelectableFlags_AllowOverlap)) {
                m_sidebarSelected = i;
                if (!e.path.empty()) {
                    std::ifstream ifs(e.path);
                    if (ifs) {
                        m_filePath = e.path;
                        m_editor.SetText(std::string(
                            std::istreambuf_iterator<char>(ifs),
                            std::istreambuf_iterator<char>()));
                        m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Gcode);
                    }
                }
            }
            if (!e.path.empty() && ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", e.path.c_str());

            // Right-click context menu
            if (ImGui::BeginPopupContextItem("##sidebar_ctx")) {
                for (const auto& action : e.actions) {
                    if (action.label.empty() || !action.onClick)
                        continue;
                    if (ImGui::MenuItem(action.label.c_str()))
                        action.onClick();
                }
                ImGui::EndPopup();
            }
            ImGui::PopID();
        }
        ImGui::EndChild();
        ImGui::SameLine();
    }

    ImVec2 editorSize = ImGui::GetContentRegionAvail();
    editorSize.y -= statusH;

    m_editor.Render("##code", ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows), editorSize);

    if (m_syncPlaybackFromCursor && ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
        const int curLine = getCursorLine();
        if (curLine != m_lastReportedCursorLine) {
            m_lastReportedCursorLine = curLine;
            if (m_onCursorLineChanged) m_onCursorLineChanged(curLine);
        }
    }

    ImGui::Separator();
    int curLine = 0, curCol = 0;
    m_editor.GetCursorPosition(curLine, curCol);
    ImGui::TextDisabled("Ln %d, Col %d   |   %d lines   |   %s%s", curLine + 1, curCol + 1,
                        m_editor.GetLineCount(), m_editor.GetLanguageDefinitionName(),
                        m_editor.IsReadOnlyEnabled() ? "   [Read Only]" : "");

    ImGui::End();
}
