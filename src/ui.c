#include "raylib.h"
#include "external/raygui.h"

#include "log.h"
#include "state.h"

struct GuiDialogState {
    Rectangle rect;
    unsigned int padding, line_height, line_spacing;
};

static struct GuiDialogState m_window = {{0}};
const char *algo_options = NULL;
int algo_active = 0; //TODO active

char grav_txt[32] = "\0";
float grav_g = 0.f;
int grav_edit = 0;

char sel_txt[1024] = "\0";

static Rectangle _grid(struct GuiDialogState window, unsigned int col, unsigned int row, unsigned int w, unsigned int h) {
    unsigned int col_width = (window.rect.width/2) - (3 * window.padding); // l + m + r

    unsigned int off_x = window.rect.x + window.padding;
    unsigned int off_y = window.rect.y + 30 + window.padding;

    return (Rectangle) {
        off_x + (col * col_width),
        off_y + (row * (window.line_height + window.line_spacing)),
        (w) ? w : col_width,
        (h) ? h : window.line_height
    };
};

void ui_init(State *state) {
    EXIT_IF(state == NULL, "no state");

    m_window.rect.x = 10;
    m_window.rect.y = 50;
    m_window.rect.width = 300;

    m_window.padding = 5;
    m_window.line_height = 20;
    m_window.line_spacing = 5;

    m_window.rect.height = 30 + (8 * (m_window.line_height + m_window.line_spacing )) + (2 * m_window.padding);

    // Note @see raylib rtext.c currently (v 5.5) TextJoin() is using stack memory
    // and should not be freed: static char buffer[MAX_TEXT_BUFFER_LENGTH]
    algo_options = TextJoin(algorithms, ALGO_LEN, "\n");

    grav_g =  state->grav_g;
    snprintf(grav_txt, sizeof(grav_txt), "%2.2f", grav_g);
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

        /* first column */

        // paused
        GuiCheckBox(_grid(m_window, 0, 0, m_window.line_height, 0), "Paused", &state->paused);

        // ui_debug
        GuiCheckBox(_grid(m_window, 0, 1, m_window.line_height, 0), "Show debug", &state->ui_debug);

        // ui_borticles
        GuiCheckBox(_grid(m_window, 0, 2, m_window.line_height, 0), "Show borticles", &state->ui_borticles);

        // ui_qtree
        GuiCheckBox(_grid(m_window, 0, 3, m_window.line_height, 0), "Show quadtree", &state->ui_qtree);

        // pop_len
        float pop_len = (float) state->pop_len;
        GuiLabel(_grid(m_window, 0, 4, 0, 0), "Population");
        GuiSliderBar(_grid(m_window, 0, 5, 0, 0), NULL, TextFormat("%d (max:%d)", state->pop_len, state->pop_max), &pop_len, 0, state->pop_max);
        if (pop_len > 0.f && pop_len != state->pop_len) {
            state_set_pop_len(state, (int) round(pop_len));
        }

        // grav_g
        GuiLabel(_grid(m_window, 0, 6, 0, 0), "Gravitational constant");
        GuiSlider(_grid(m_window, 0, 7, 0, 0), NULL, TextFormat("%2.f (max:%2.f)", state->grav_g, 20.f), &grav_g, 0.f, 20.f);
        if (grav_g != state->grav_g) {
            state->grav_g = grav_g;
        }

        /* second column */

        // algorithms
        int old = algo_active;
        GuiToggleGroup(_grid(m_window, 1, 0, 0 ,0), algo_options, &algo_active); // fn currently always returns 0, so checks are useless
        if (algo_active != old) {
            state->algorithms = (1 << algo_active); // singl assignment not stacking algorithms (yet)
        }
    }

    if (state->selected) {
        snprintf(sel_txt, 1024,
            "id: %d\n"
            "pos: {%.2f, %.2f}\n"
            "vel: {%.2f, %.2f}\n"
            "acc: {%.2f, %.2f}\n"
            "size: %.2f\n",
            state->selected->id,
            state->selected->pos.x, state->selected->pos.y,
            state->selected->vel.x, state->selected->vel.y,
            state->selected->acc.x, state->selected->acc.y,
            state->selected->size
        );

        Rectangle cont = (Rectangle){
            state->selected->pos.x + 10,
            state->selected->pos.y + 10,
            150,
            100
        };

        int ret = GuiPanel(cont, NULL);
        GuiTextBox((Rectangle){
            cont.x + 1,
            cont.y + 1,
            cont.width - 2,
            cont.height - 2,
        }, sel_txt, 1024, false);
    }
}

void ui_destroy(State *state) {}
