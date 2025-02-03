#include "external/raygui.h"

// #include "raylib.h"
#include "log.h"
#include "state.h"

struct GuiDialogState {
    Rectangle rect;
    unsigned int padding, line_height, line_spacing;
};

static struct GuiDialogState m_window = {{0}};

static Rectangle _row(int row, int w, int h) {
    return (Rectangle) {
        m_window.rect.x + m_window.padding,
        (row * (m_window.line_height + m_window.line_spacing)) + m_window.rect.y + 30 + m_window.padding,
        (w) ? w : m_window.rect.width - 100 - (2 * m_window.padding),
        (h) ? h : m_window.line_height
    };
};

void ui_init(State *state) {
    EXIT_IF(state == NULL, "no state");

    m_window.rect.x = 10;
    m_window.rect.y = 50;
    m_window.rect.width = 300;
    m_window.rect.height = 200;
    m_window.padding = 5;
    m_window.line_height = 20;
    m_window.line_spacing = 5;
}

void ui_update(State *state) {
    EXIT_IF(state == NULL, "no state");
}

void ui_draw(State *state) {
    EXIT_IF(state == NULL, "no state");
    // GuiPanel((Rectangle){ 320, 25, 225, 140 },NULL);

    if(state->ui_debug) {
        DrawFPS(10, 10);
    }

    if (GuiButton((Rectangle){ 10, state->height - 25, 20, 20 }, GuiIconText(ICON_GEAR, ""))) {
        state->ui_minimized = !state->ui_minimized;
    }

    if (state->ui_minimized) {
        state->ui_minimized = !GuiWindowBox(m_window.rect, "State controls");

        // paused
        GuiCheckBox(_row(0, m_window.line_height, 0), "Paused", &state->paused);

        // ui_debug
        GuiCheckBox(_row(1, m_window.line_height, 0), "Show debug", &state->ui_debug);

        // ui_borticles
        GuiCheckBox(_row(2, m_window.line_height, 0), "Show borticles", &state->ui_borticles);

        // ui_qtree
        GuiCheckBox(_row(3, m_window.line_height, 0), "Show quadtree", &state->ui_qtree);

        // pop_len
        float pop_len = (float) state->pop_len;
        GuiLabel(_row(4, 0, 0), "Population");
        GuiSliderBar(_row(5, 0, 0), NULL, TextFormat("%d (max:%d)", state->pop_len, state->pop_max), &pop_len, 0, state->pop_max);
        if (pop_len > 0.f && pop_len != state->pop_len) {
            state_set_pop_len(state, (int) round(pop_len));
        }

    }
}
