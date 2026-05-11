#include "ofApp.h"

// ─── sample code snippets ────────────────────────────────────────────────────

static const char* kSampleCpp = R"(#include "ofApp.h"

// A basic openFrameworks app
void ofApp::setup() {
    ofSetWindowTitle("ofxImGuiTextEdit demo");
    ofBackground(30, 30, 30);
    ofSetFrameRate(60);
}

void ofApp::update() {
    float t = ofGetElapsedTimef();
    position.x = ofGetWidth()  * 0.5f + std::cos(t) * 200.0f;
    position.y = ofGetHeight() * 0.5f + std::sin(t) * 200.0f;
}

void ofApp::draw() {
    ofSetColor(255, 180, 60);
    ofDrawCircle(position, radius);

    ofSetColor(255);
    ofDrawBitmapString("fps: " + ofToString(ofGetFrameRate(), 1), 10, 20);
}
)";

static const char* kSamplePython = R"(import math

def lerp(a: float, b: float, t: float) -> float:
    """Linear interpolation between a and b."""
    return a + (b - a) * t

class Particle:
    def __init__(self, x: float, y: float):
        self.x = x
        self.y = y
        self.vx = 0.0
        self.vy = 0.0

    def update(self, dt: float) -> None:
        self.x += self.vx * dt
        self.y += self.vy * dt

    def distance_to(self, other: "Particle") -> float:
        dx = self.x - other.x
        dy = self.y - other.y
        return math.sqrt(dx * dx + dy * dy)

particles = [Particle(i * 10.0, 0.0) for i in range(8)]
)";

static const char* kSampleGlsl = R"(#version 150

uniform float time;
uniform vec2  resolution;

out vec4 fragColor;

float sdCircle(vec2 p, float r) {
    return length(p) - r;
}

void main() {
    vec2 uv = (gl_FragCoord.xy / resolution) * 2.0 - 1.0;
    uv.x *= resolution.x / resolution.y;

    float d  = sdCircle(uv, 0.4 + 0.1 * sin(time * 2.0));
    float aa = fwidth(d);
    float c  = smoothstep(aa, -aa, d);

    vec3 col = mix(vec3(0.08, 0.08, 0.12), vec3(1.0, 0.55, 0.2), c);
    fragColor = vec4(col, 1.0);
}
)";

static const char* kSampleLua = R"(-- Simple easing library in Lua

local ease = {}

function ease.linear(t)
    return t
end

function ease.inQuad(t)
    return t * t
end

function ease.outQuad(t)
    return t * (2 - t)
end

function ease.inOutCubic(t)
    if t < 0.5 then
        return 4 * t * t * t
    else
        return (t - 1) * (2 * t - 2) * (2 * t - 2) + 1
    end
end

-- Tween a value from `from` to `to` over `duration` seconds
function ease.tween(from, to, elapsed, duration, fn)
    local t = math.min(elapsed / duration, 1.0)
    return from + (to - from) * fn(t)
end

return ease
)";

static const char* kSampleJson = R"({
    "app": {
        "title": "ofxImGuiTextEdit Demo",
        "version": "1.0.0",
        "window": {
            "width": 1400,
            "height": 800,
            "fullscreen": false
        }
    },
    "editor": {
        "palette": "dark",
        "tabSize": 4,
        "lineSpacing": 1.0,
        "showLineNumbers": true,
        "showWhitespaces": true,
        "autoIndent": true
    },
    "languages": ["C++", "Python", "GLSL", "Lua", "JSON", "SQL"]
}
)";

static const char* kSampleSql = R"(-- Query project metadata and recent activity

CREATE TABLE IF NOT EXISTS projects (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    name        TEXT    NOT NULL,
    language    TEXT    NOT NULL DEFAULT 'cpp',
    created_at  TEXT    NOT NULL DEFAULT (datetime('now')),
    updated_at  TEXT
);

INSERT INTO projects (name, language) VALUES
    ('ofxImGuiTextEdit demo', 'cpp'),
    ('shader playground',     'glsl'),
    ('lua scripting',         'lua');

SELECT
    p.name,
    p.language,
    COUNT(f.id)  AS file_count,
    MAX(f.size)  AS largest_file
FROM  projects p
LEFT  JOIN files f ON f.project_id = p.id
WHERE p.language IN ('cpp', 'glsl')
GROUP BY p.id
ORDER BY p.updated_at DESC
LIMIT 20;
)";

// ─── setup ───────────────────────────────────────────────────────────────────

void ofApp::setup() {
    gui.setup();

    languages = {{
        { "C++",    TextEditor::LanguageDefinitionId::Cpp,    kSampleCpp    },
        { "Python", TextEditor::LanguageDefinitionId::Python, kSamplePython },
        { "GLSL",   TextEditor::LanguageDefinitionId::Glsl,   kSampleGlsl   },
        { "Lua",    TextEditor::LanguageDefinitionId::Lua,    kSampleLua    },
        { "JSON",   TextEditor::LanguageDefinitionId::Json,   kSampleJson   },
        { "SQL",    TextEditor::LanguageDefinitionId::Sql,    kSampleSql    },
    }};

    palettes = {{
        { "Dark",       TextEditor::PaletteId::Dark       },
        { "Light",      TextEditor::PaletteId::Light      },
        { "Mariana",    TextEditor::PaletteId::Mariana    },
        { "Retro Blue", TextEditor::PaletteId::RetroBlue  },
    }};

    applyLanguage(0);
    applyPalette(0);

    editor.SetShowLineNumbersEnabled(showLineNumbers);
    editor.SetShowWhitespacesEnabled(showWhitespaces);
    editor.SetAutoIndentEnabled(autoIndent);
    editor.SetShortTabsEnabled(shortTabs);
    editor.SetTabSize(tabSize);
    editor.SetLineSpacing(lineSpacing);
}

// ─── draw ────────────────────────────────────────────────────────────────────

void ofApp::draw() {
    ofBackground(25, 25, 35);

    gui.begin();

    // One full-screen host window — no decorations, no background
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2((float)ofGetWidth(), (float)ofGetHeight()), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("##host", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoBackground);
    ImGui::PopStyleVar();

    drawControlsPanel();
    ImGui::SameLine(0, 0);
    drawEditorPanel();

    ImGui::End();
    gui.end();
}

// ─── controls panel ──────────────────────────────────────────────────────────

void ofApp::drawControlsPanel() {
    ImGui::BeginChild("Controls", ImVec2(270, 0), ImGuiChildFlags_Borders);

    // ── Language ──────────────────────────────────────────────────
    ImGui::SeparatorText("Language");
    for (int i = 0; i < kNumLanguages; ++i) {
        bool sel = (selectedLanguage == i);
        if (ImGui::RadioButton(languages[i].label, sel)) {
            selectedLanguage = i;
            applyLanguage(i);
        }
        if (i % 2 == 0 && i + 1 < kNumLanguages) ImGui::SameLine();
    }

    // ── Palette ───────────────────────────────────────────────────
    ImGui::SeparatorText("Palette");
    for (int i = 0; i < kNumPalettes; ++i) {
        bool sel = (selectedPalette == i);
        if (ImGui::RadioButton(palettes[i].label, sel)) {
            selectedPalette = i;
            applyPalette(i);
        }
        if (i % 2 == 0 && i + 1 < kNumPalettes) ImGui::SameLine();
    }

    // ── Display options ───────────────────────────────────────────
    ImGui::SeparatorText("Display");

    if (ImGui::Checkbox("Line numbers", &showLineNumbers))
        editor.SetShowLineNumbersEnabled(showLineNumbers);

    if (ImGui::Checkbox("Whitespaces", &showWhitespaces))
        editor.SetShowWhitespacesEnabled(showWhitespaces);

    if (ImGui::Checkbox("Short tabs", &shortTabs))
        editor.SetShortTabsEnabled(shortTabs);

    ImGui::SetNextItemWidth(120.0f);
    if (ImGui::SliderInt("Tab size", &tabSize, 2, 8))
        editor.SetTabSize(tabSize);

    ImGui::SetNextItemWidth(120.0f);
    if (ImGui::SliderFloat("Line spacing", &lineSpacing, 1.0f, 2.0f, "%.2f"))
        editor.SetLineSpacing(lineSpacing);

    // ── Editing ───────────────────────────────────────────────────
    ImGui::SeparatorText("Editing");

    if (ImGui::Checkbox("Read-only", &readOnly))
        editor.SetReadOnlyEnabled(readOnly);

    if (ImGui::Checkbox("Auto-indent", &autoIndent))
        editor.SetAutoIndentEnabled(autoIndent);

    // ── Undo / Redo ───────────────────────────────────────────────
    ImGui::SeparatorText("Undo / Redo");

    ImGui::BeginDisabled(!editor.CanUndo());
    if (ImGui::Button("Undo", ImVec2(80, 0))) editor.Undo();
    ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::BeginDisabled(!editor.CanRedo());
    if (ImGui::Button("Redo", ImVec2(80, 0))) editor.Redo();
    ImGui::EndDisabled();

    ImGui::Text("Undo steps: %d", editor.GetUndoIndex());

    // ── Selection helpers ─────────────────────────────────────────
    ImGui::SeparatorText("Selection");

    if (ImGui::Button("Select all", ImVec2(120, 0)))
        editor.SelectAll();

    if (ImGui::Button("Select line 1", ImVec2(120, 0)))
        editor.SelectLine(0);

    if (ImGui::Button("Select lines 1-3", ImVec2(120, 0)))
        editor.SelectRegion(0, 0, 2, 0);

    if (ImGui::Button("Clear selection", ImVec2(120, 0)))
        editor.ClearSelections();

    bool hasSel = editor.AnyCursorHasSelection();
    ImGui::TextDisabled("Has selection: %s", hasSel ? "yes" : "no");

    // ── Cursor info ───────────────────────────────────────────────
    ImGui::SeparatorText("Cursor");

    int curLine = 0, curCol = 0;
    editor.GetCursorPosition(curLine, curCol);
    ImGui::Text("Line %d, Col %d", curLine + 1, curCol + 1);
    ImGui::Text("Total lines: %d", editor.GetLineCount());
    ImGui::Text("Visible: %d – %d",
        editor.GetFirstVisibleLine() + 1,
        editor.GetLastVisibleLine() + 1);

    // ── Jump to line ──────────────────────────────────────────────
    ImGui::SeparatorText("Jump");

    static int jumpLine = 1;
    ImGui::SetNextItemWidth(80.0f);
    ImGui::InputInt("##line", &jumpLine, 1, 5);
    jumpLine = std::max(1, std::min(jumpLine, editor.GetLineCount()));
    ImGui::SameLine();
    if (ImGui::Button("Go")) {
        editor.SetCursorPosition(jumpLine - 1, 0);
        editor.SetViewAtLine(jumpLine - 1, TextEditor::SetViewAtLineMode::Centered);
    }

    ImGui::EndChild();
}

// ─── editor panel ────────────────────────────────────────────────────────────

void ofApp::drawEditorPanel() {
    // ImVec2(0,0) = fill all remaining space in the host window
    ImGui::BeginChild("Editor", ImVec2(0, 0), ImGuiChildFlags_Borders);

    ImGui::TextDisabled("%s  |  %s  |  %d lines",
        languages[selectedLanguage].label,
        palettes[selectedPalette].label,
        editor.GetLineCount());
    ImGui::Separator();

    ImVec2 avail = ImGui::GetContentRegionAvail();
    editor.Render("TextEditor", false, avail, false);

    ImGui::EndChild();
}

// ─── helpers ─────────────────────────────────────────────────────────────────

void ofApp::applyLanguage(int idx) {
    editor.SetLanguageDefinition(languages[idx].id);
    editor.SetText(languages[idx].sample);
}

void ofApp::applyPalette(int idx) {
    editor.SetPalette(palettes[idx].id);
}
