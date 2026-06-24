#pragma once

#include <functional>
#include <string>
#include <vector>

#include "TextEditorFindReplace.h"
#include "TextEditor.h"
#include "imgui.h"

class TextEditorPanel {
public:
    using ConfirmPath = std::function<void(const std::string& path)>;

    void setup();

    /// Route file dialogs through the host (`Runtime::openFileDialog` /
    /// `saveFileDialog`) so IGFD stays centralized.
    void setDialogCallbacks(
        std::function<void(const std::string& key,
                           const std::string& title,
                           const std::string& filters,
                           ConfirmPath onConfirm)> openFile,
        std::function<void(const std::string& key,
                           const std::string& title,
                           const std::string& filters,
                           const std::string& defaultFileName,
                           ConfirmPath onConfirm)> saveFile);

    void draw(bool& visible);

    void           setText(const std::string& text,
                           TextEditor::LanguageDefinitionId lang = TextEditor::LanguageDefinitionId::None);
    std::string    getText() const;
    void           setLanguage(TextEditor::LanguageDefinitionId lang);

    /// Optional left sidebar entries (e.g. open file + custom actions).
    struct SidebarAction {
        std::string           label;
        std::function<void()> onClick;
    };
    struct SidebarEntry {
        std::string label;
        std::string path;
        bool        isActive = false;
        /// Right-click context menu actions (empty label or null onClick = skipped).
        std::vector<SidebarAction> actions;
    };
    void setSidebarEntries(std::vector<SidebarEntry> entries);

    /// Highlight a 0-based line (playback / print cursor). -1 clears.
    void setHighlightLine(int line);
    /// Move editor caret to a 0-based line without changing playback.
    void setCursorLine(int line);
    int  getCursorLine() const;
    int  getLineCount() const;
    int  getUndoIndex() const;

    /// When true, cursor moves in the editor update Plot Preview playback.
    void setSyncPlaybackFromCursor(bool enabled) { m_syncPlaybackFromCursor = enabled; }
    void setOnCursorLineChanged(std::function<void(int line)> cb) {
        m_onCursorLineChanged = std::move(cb);
    }

    /// Optional toolbar row drawn below the menu bar (e.g. domain-specific actions).
    void setToolbarDraw(std::function<void()> cb) { m_toolbarDraw = std::move(cb); }

    /// Open find (Ctrl+F) or find-and-replace (Ctrl+Shift+F, regex supported).
    void showFind(bool withReplace = false) { m_findReplace.showFind(withReplace); }
    void handleFindReplaceShortcuts() { m_findReplace.handleShortcuts(); }

    TextEditor&       editor() { return m_editor; }
    const TextEditor& editor() const { return m_editor; }

    /// Monospace font for the editor surface (JetBrains Mono via ImFonts).
    void setFont(ImFont* font) {
        m_font = font;
        m_editor.SetFont(font);
    }

private:
    TextEditor  m_editor;
    ImFont*     m_font = nullptr;
    std::string m_filePath;

    std::function<void(const std::string& key,
                       const std::string& title,
                       const std::string& filters,
                       ConfirmPath onConfirm)> m_openFile;
    std::function<void(const std::string& key,
                       const std::string& title,
                       const std::string& filters,
                       const std::string& defaultFileName,
                       ConfirmPath onConfirm)> m_saveFile;

    TextEditorFindReplace m_findReplace;

    std::vector<SidebarEntry> m_sidebarEntries;
    int                       m_sidebarSelected = -1;

    int  m_highlightLine = -1;
    bool m_syncPlaybackFromCursor = false;
    int  m_lastReportedCursorLine = -1;
    std::function<void(int)> m_onCursorLineChanged;
    std::function<void()>    m_toolbarDraw;
};
