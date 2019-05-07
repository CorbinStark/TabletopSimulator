#ifndef MAP_H
#define MAP_H

#include "globals.h"
#include "bahamut.h"

enum GameState {
	STATE_IDLE,
	STATE_CHARSHEET,
	STATE_STANDSHEET,
	STATE_TOKEN_TRANSITION,
	STATE_TOKEN,
	STATE_SQUARE,
	STATE_CIRCLE,
	STATE_ROLL_PROMPT,
	STATE_ROLL
};

struct StatusBar {
	i32 current;
	i32 max;
};

struct Token {
	u16 imgindex;
	StatusBar bar1;
	StatusBar bar2;
	StatusBar bar3;
	std::string name;
	vec4 tintColor;
	bool anchorToTile;
	i32 xPos;
	i32 yPos;
};

struct RectShape {
	u8 thickness;
	Rect dim;
	vec4 bgcolor;
	vec4 fgcolor;
};

typedef std::vector<Token> TokenList;
typedef std::vector<RectShape> RectList;

struct Map {
	u32 width;
	u32 height;
	f32 xPos;
	f32 yPos;
	bool grid;
	bool fow;
	vec4 bgColor;
	vec4 gridColor;
	TokenList tokens;
	RectList rects;
	Sound music;

	i16 selected;
};

static inline
void clamp(i32& input, i32 min, i32 max) {
	if (input > max) input = max;
	if (input < min) input = min;
}

static inline
i32 roundUp(i32 numToRound, i32 multiple) {
	if (multiple == 0)
		return numToRound;

	int remainder = abs(numToRound) % multiple;
	if (remainder == 0)
		return numToRound;

	if (numToRound < 0)
		return -(abs(numToRound) - remainder);
	else
		return numToRound + multiple - remainder;
}

INTERNAL inline
Map load_map(const char* path) {
	Map map = { 0 };

	map.selected = -1;

	return map;
}

INTERNAL inline
void save_map(Map* map, const char* path) {

}

INTERNAL inline
void draw_status_bar(RenderBatch* batch, i32 x, i32 y, StatusBar bar, vec4 color) {
	if (bar.max != 0) {
		draw_rectangle(batch, x - 1, y - 1, TILESIZE + 4, 16, BLACK);
		draw_rectangle(batch, x + 2, y + 2, TILESIZE - 2, 10, WHITE);
		f32 width = (f32)((f32)bar.current / (f32)bar.max) * (TILESIZE - 2);
		draw_rectangle(batch, x + 2, y + 2, width, 10, color);
	}
}

INTERNAL inline
bool draw_icon_button(RenderBatch* batch, Texture* tex, const u16 xPos, const u16 yPos, const f64 zoom) {
	const Rect button = rect(xPos, yPos, tex->width, tex->height);
	vec2 mouse_pos = get_mouse_pos();
	mouse_pos.x /= zoom;
	mouse_pos.y /= zoom;
	const bool collided = colliding(button, mouse_pos.x, mouse_pos.y);
	const bool buttonReleased = is_button_released(MOUSE_BUTTON_LEFT);

	if (collided) {
		draw_texture(batch, *tex, xPos, yPos);
	}
	else
		draw_texture(batch, *tex, xPos, yPos, V4(200, 200, 200, 200)); //highlight if mouse is on button

	return collided & buttonReleased;
}

INTERNAL inline
void update_map(RenderBatch* batch, Map* map, Socket* socket, GameState& state, f64 zoom) {
	vec2 mousePos = get_mouse_pos();
	mousePos.x /= zoom;
	mousePos.y /= zoom;

	if (is_button_released(MOUSE_BUTTON_RIGHT))
		map->selected = -1;

	if (map->selected != -1) {
		bool mouseInsideMap = false;
		bool hoveredButton = false;
		Token* current = &map->tokens[map->selected];

		vec2 tile = V2(roundUp(mousePos.x - map->xPos, TILESIZE) - TILESIZE, roundUp(mousePos.y - map->yPos, TILESIZE) - TILESIZE);
		Rect button = rect(map->xPos + current->xPos, map->yPos + current->yPos + TILESIZE, settings_icon.width, settings_icon.height);
		if (tile.x / TILESIZE >= 0 && tile.y / TILESIZE >= 0 && tile.x / TILESIZE < map->width && tile.y / TILESIZE < map->height) mouseInsideMap = true;
		if (colliding(button, mousePos.x, mousePos.y)) hoveredButton = true;

		if (mouseInsideMap && !hoveredButton)
			draw_rectangle(batch, tile.x + map->xPos, tile.y + map->yPos, TILESIZE, TILESIZE, V4(150, 40, 150, 120));

		if (draw_icon_button(batch, &settings_icon, map->xPos + current->xPos, map->yPos + current->yPos + TILESIZE, zoom)) {
			state = STATE_TOKEN_TRANSITION;
		}

		if (is_button_released(MOUSE_BUTTON_LEFT) && mouseInsideMap && !hoveredButton) {
			current->xPos = tile.x;
			current->yPos = tile.y;
			std::string command = "move|";
			command.append(std::to_string(map->selected));
			command.append("|");
			command.append(std::to_string(tile.x));
			command.append("|");
			command.append(std::to_string(tile.y));
			command.append("\n");
			socket->write_some(boost::asio::buffer(command, command.size()));
		}
	}
}

INTERNAL inline
void draw_map(RenderBatch* batch, Map* map, f64 zoom) {
	vec2 mousePos = get_mouse_pos();
	mousePos.x /= zoom;
	mousePos.y /= zoom;

#define PADDING 3
	i32 x0 = (-map->xPos / (TILESIZE));
	i32 x1 = (-map->xPos / (TILESIZE)) + ((get_window_width() / zoom) / TILESIZE) + PADDING;
	i32 y0 = (-map->yPos / (TILESIZE));
	i32 y1 = (-map->yPos / (TILESIZE)) + ((get_window_height() / zoom) / TILESIZE) + PADDING;
	clamp(x0, 0, map->width);
	clamp(y0, 0, map->height);
	clamp(x1, 0, map->width);
	clamp(y1, 0, map->height);
#undef PADDING

	for (u32 x = x0; x < x1; ++x) {
		for (u32 y = y0; y < y1; ++y) {
			draw_rectangle(batch, (map->xPos) + (x * TILESIZE), (map->yPos) + (y * TILESIZE), TILESIZE, TILESIZE, map->gridColor);
			draw_rectangle(batch, (map->xPos) + ((x * TILESIZE) + 4), (map->yPos) + ((y * TILESIZE) + 4), TILESIZE - 8, TILESIZE - 8, map->bgColor);
		}
	}

	for (u16 i = 0; i < map->rects.size(); ++i) {
		RectShape* curr = &map->rects[i];
		draw_rectangle(batch, curr->dim.x, curr->dim.y, curr->dim.width, curr->dim.height, curr->bgcolor);
		draw_rectangle(batch,
			curr->dim.x + curr->thickness,
			curr->dim.y + curr->thickness,
			curr->dim.width - curr->thickness * 2,
			curr->dim.height - curr->thickness * 2,
			curr->fgcolor
		);
	}
}

INTERNAL inline
void draw_tokens(RenderBatch* batch, Map* map, f64 zoom) {
	vec2 mousePos = get_mouse_pos();
	mousePos.x /= zoom;
	mousePos.y /= zoom;

	for (u32 i = 0; i < map->tokens.size(); ++i) {
		Token* current = &map->tokens[i];

		if (map->selected == i) {
			const u8 highlightSize = 8;
			draw_rectangle(batch, map->xPos + current->xPos - highlightSize,
				map->yPos + current->yPos - highlightSize,
				TILESIZE + (highlightSize * 2),
				TILESIZE + (highlightSize * 2),
				SKYBLUE
			);
		}

		Rect tokenRect = rect(map->xPos + current->xPos, map->yPos + current->yPos, TILESIZE, TILESIZE);
		if (colliding(tokenRect, mousePos.x, mousePos.y)) {
			if (is_button_released(MOUSE_BUTTON_LEFT))
				map->selected = i;
			draw_texture(batch, tokenimages[current->imgindex], map->xPos + current->xPos, map->yPos + current->yPos, V4(172, 261, 255, 255));
		}
		else {
			draw_texture(batch, tokenimages[current->imgindex], map->xPos + current->xPos, map->yPos + current->yPos);
		}
		draw_status_bar(batch, map->xPos + current->xPos, map->yPos + current->yPos - 5, current->bar1, GREEN);
		draw_status_bar(batch, map->xPos + current->xPos, map->yPos + current->yPos - 25, current->bar2, RED);
		draw_status_bar(batch, map->xPos + current->xPos, map->yPos + current->yPos - 45, current->bar3, BLUE);
		draw_text(batch, &BODY_FONT, current->name, map->xPos + current->xPos + 30, map->yPos + current->yPos + TILESIZE + 15, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
	}
}

#endif