#pragma once

#include "ofMain.h"
#include "ofxImGui.h"
#include "ofxImGuiTextEdit.h"
#include <array>

class ofApp : public ofBaseApp {
public:
    void setup();
    void draw();

    ofxImGui::Gui gui;
    TextEditor editor;

    // --- language samples ---
    static constexpr int kNumLanguages = 6;
    struct LangEntry {
        const char* label;
        TextEditor::LanguageDefinitionId id;
        const char* sample;
    };
    std::array<LangEntry, kNumLanguages> languages;
    int selectedLanguage = 0;

    // --- palette ---
    static constexpr int kNumPalettes = 4;
    struct PaletteEntry { const char* label; TextEditor::PaletteId id; };
    std::array<PaletteEntry, kNumPalettes> palettes;
    int selectedPalette = 0;

    // --- display options (mirrored from editor for UI) ---
    bool showLineNumbers   = true;
    bool showWhitespaces   = true;
    bool readOnly          = false;
    bool autoIndent        = true;
    bool shortTabs         = false;
    int  tabSize           = 4;
    float lineSpacing      = 1.0f;

private:
    void applyLanguage(int idx);
    void applyPalette(int idx);
    void drawControlsPanel();
    void drawEditorPanel();
};
