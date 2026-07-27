static Client *mango_frametail_client(uint64_t id) {
	Client *c;
	wl_list_for_each(c, &clients, link) {
		if (c->id == id)
			return c;
	}
	return NULL;
}

static void mango_frametail_log(void *data, const char *message) {
	(void)data;
	wlr_log(WLR_ERROR, "Frametail: %s", message);
}

static void mango_frametail_action(void *data, uint64_t window_id,
		const char *action, const struct frametail_decoration_event *event) {
	(void)data;
	if (frametail_staging)
		return;
	Client *c = mango_frametail_client(window_id);
	if (!c || c->iskilling)
		return;

	if (event && event->button != BTN_LEFT)
		return;
	if (strcmp(action, "close") == 0) {
		pending_kill_client(c);
	} else if (strcmp(action, "minimize") == 0) {
		set_minimized(c);
	} else if (strcmp(action, "maximize") == 0) {
		setmaximizescreen(c, !c->ismaximizescreen, true);
	} else if (strcmp(action, "fullscreen") == 0) {
		setfullscreen(c, !c->isfullscreen, true);
	} else if (strcmp(action, "move") == 0) {
		moveresize(&(Arg){.ui = CurMove});
	} else if (strcmp(action, "resize") == 0) {
		moveresize(&(Arg){.ui = CurResize});
	} else {
		wlr_log(WLR_INFO, "Frametail: unknown action '%s'", action);
	}
}

static bool mango_frametail_corner_resize(Client *c,
		struct wlr_pointer_button_event *event) {
	if (!c || !c->frametail_decoration || event->button != BTN_LEFT ||
		event->state != WL_POINTER_BUTTON_STATE_PRESSED || c->isfullscreen ||
		c->ismaximizescreen)
		return false;
	int32_t x, y;
	if (!wlr_scene_node_coords(&c->scene->node, &x, &y))
		return false;
	const int32_t grip = 10;
	int32_t local_x = (int32_t)round(cursor->x) - x;
	int32_t local_y = (int32_t)round(cursor->y) - y;
	bool left = local_x >= 0 && local_x < grip;
	bool right = local_x <= c->geom.width && local_x > c->geom.width - grip;
	bool top = local_y >= 0 && local_y < grip;
	bool bottom = local_y <= c->geom.height && local_y > c->geom.height - grip;
	if (!(left || right) || !(top || bottom))
		return false;
	resize_corner_override = (right ? 1 : 0) + (bottom ? 2 : 0);
	moveresize(&(Arg){.ui = CurResize});
	return cursor_mode == CurResize;
}

static struct frametail_lua_window mango_frametail_window(Client *c) {
	struct wlr_xdg_toplevel_icon_v1_buffer *selected = NULL, *candidate;
	int selected_distance = INT_MAX;
	if (c->icon) {
		wl_list_for_each(candidate, &c->icon->buffers, link) {
			int scale = candidate->scale > 0 ? candidate->scale : 1;
			int logical_size = candidate->buffer->width / scale;
			int distance = abs(logical_size - 24);
			if (distance < selected_distance) {
				selected = candidate;
				selected_distance = distance;
			}
		}
	}
	return (struct frametail_lua_window){
		.id = c->id,
		.title = client_get_title(c),
		.app_id = client_get_appid(c),
		.icon_name = c->icon ? c->icon->name : NULL,
		.icon_buffer = selected ? selected->buffer : NULL,
		.icon_width = selected ? selected->buffer->width /
			(selected->scale > 0 ? selected->scale : 1) : 0,
		.icon_height = selected ? selected->buffer->height /
			(selected->scale > 0 ? selected->scale : 1) : 0,
		.focused = c->isfocusing,
		.urgent = c->isurgent,
		.maximized = c->ismaximizescreen,
		.fullscreen = c->isfullscreen,
	};
}

static bool mango_frametail_build(Client *c, int32_t content_width,
		int32_t content_height) {
	char error[512];
	struct frametail_lua_window window = mango_frametail_window(c);
	c->frametail_handle = frametail_lua_runtime_build(frametail_runtime, c->scene,
		&window, error, sizeof(error));
	if (!c->frametail_handle) {
		wlr_log(WLR_ERROR, "Frametail: failed to decorate '%s': %s",
			client_get_title(c), error);
		return false;
	}
	c->frametail_decoration = frametail_lua_decoration_get(c->frametail_handle);
	if (!c->frametail_decoration) {
		frametail_lua_decoration_destroy(c->frametail_handle);
		c->frametail_handle = NULL;
		return false;
	}
	c->frametail_extents =
		frametail_decoration_get_extents(c->frametail_decoration);
	c->bw = 0;
	c->geom.width = MANGO_MAX(1, content_width) +
		(c->isfullscreen ? 0 : c->frametail_extents.left +
			c->frametail_extents.right);
	c->geom.height = MANGO_MAX(1, content_height) +
		(c->isfullscreen ? 0 : c->frametail_extents.top +
			c->frametail_extents.bottom);
	mango_frametail_sync_geometry(c, c->geom);
	return true;
}

static void mango_frametail_create(Client *c) {
	if (!frametail_runtime || !config.frametail || c->frametail_handle ||
		client_is_unmanaged(c) || client_is_x11_popup(c))
		return;
	int32_t content_width = c->geom.width - 2 * (int32_t)c->bw;
	int32_t content_height = c->geom.height - 2 * (int32_t)c->bw;
	mango_frametail_build(c, content_width, content_height);
}

static void mango_frametail_destroy(Client *c) {
	if (frametail_hover_client == c)
		frametail_hover_client = NULL;
	if (frametail_pressed_client == c)
		frametail_pressed_client = NULL;
	if (c->frametail_handle)
		frametail_lua_decoration_destroy(c->frametail_handle);
	c->frametail_handle = NULL;
	c->frametail_decoration = NULL;
	c->frametail_extents = (struct frametail_extents){0};
}

static void mango_frametail_sync_geometry(Client *c, struct wlr_box frame) {
	if (!c->frametail_decoration)
		return;
	c->frametail_extents =
		frametail_decoration_get_extents(c->frametail_decoration);
	struct mango_frame_extents extents = client_frame_extents(c);
	int32_t width = GEZERO(frame.width - extents.left - extents.right);
	int32_t height = GEZERO(frame.height - extents.top - extents.bottom);
	struct wlr_scene_tree *tree =
		frametail_decoration_scene_tree(c->frametail_decoration);
	wlr_scene_node_set_enabled(&tree->node, !c->isfullscreen);
	wlr_scene_node_set_position(&tree->node, extents.left, extents.top);
	wlr_scene_node_set_position(&c->scene_surface->node, extents.left,
		extents.top);
	frametail_decoration_set_content_size(c->frametail_decoration, width, height);
	if (ISSCROLLTILED(c) && c->mon) {
		struct frametail_geometry clip = {
			.x = c->mon->m.x - frame.x - extents.left,
			.y = c->mon->m.y - frame.y - extents.top,
			.width = c->mon->m.width,
			.height = c->mon->m.height,
		};
		frametail_decoration_set_clip(c->frametail_decoration, &clip);
	} else {
		frametail_decoration_set_clip(c->frametail_decoration, NULL);
	}
	if (c->mon)
		frametail_decoration_set_scale(c->frametail_decoration,
			c->mon->wlr_output->scale);
}

static void mango_frametail_fallback(Client *c, struct wlr_box content) {
	if (frametail_hover_client == c)
		frametail_hover_client = NULL;
	if (frametail_pressed_client == c)
		frametail_pressed_client = NULL;
	c->frametail_decoration = NULL;
	c->frametail_extents = (struct frametail_extents){0};
	c->bw = c->isfullscreen || c->isnoborder ? 0 : config.borderpx;
	c->geom.x = content.x - (int32_t)c->bw;
	c->geom.y = content.y - (int32_t)c->bw;
	c->geom.width = MANGO_MAX(1, content.width) + 2 * (int32_t)c->bw;
	c->geom.height = MANGO_MAX(1, content.height) + 2 * (int32_t)c->bw;
	wlr_scene_node_set_position(&c->scene_surface->node, c->bw, c->bw);
	if (c->mon && client_surface(c)->mapped)
		resize(c, c->geom, 0);
}

static void mango_frametail_update(Client *c) {
	if (!c || !c->frametail_handle)
		return;
	struct frametail_extents old_extents = c->frametail_extents;
	struct wlr_box content = client_content_box(c, c->geom);
	struct frametail_lua_window window = mango_frametail_window(c);
	frametail_lua_decoration_update_window(c->frametail_handle, &window);
	c->frametail_decoration = frametail_lua_decoration_get(c->frametail_handle);
	if (!c->frametail_decoration) {
		mango_frametail_fallback(c, content);
		return;
	}
	struct frametail_extents extents =
		frametail_decoration_get_extents(c->frametail_decoration);
	if (memcmp(&old_extents, &extents, sizeof(old_extents)) != 0) {
		c->geom.x = content.x - extents.left;
		c->geom.y = content.y - extents.top;
		c->geom.width = MANGO_MAX(1, content.width) + extents.left + extents.right;
		c->geom.height = MANGO_MAX(1, content.height) + extents.top + extents.bottom;
	}
	mango_frametail_sync_geometry(c, c->geom);
	if (c->mon && client_surface(c)->mapped &&
		memcmp(&old_extents, &c->frametail_extents, sizeof(old_extents)) != 0)
		resize(c, c->geom, 0);
}

static bool mango_frametail_reload(void) {
	Client *c;
	if (!config.frametail) {
		wl_list_for_each(c, &clients, link) {
			if (!c->frametail_handle)
				continue;
			struct wlr_box content = client_content_box(c, c->geom);
			mango_frametail_destroy(c);
			c->bw = c->isfullscreen || c->isnoborder ? 0 : config.borderpx;
			c->geom.x = content.x - (int32_t)c->bw;
			c->geom.y = content.y - (int32_t)c->bw;
			c->geom.width = content.width + 2 * (int32_t)c->bw;
			c->geom.height = content.height + 2 * (int32_t)c->bw;
			resize(c, c->geom, 0);
		}
		if (frametail_runtime)
			frametail_lua_runtime_destroy(frametail_runtime);
		frametail_runtime = NULL;
		return true;
	}

	if (!config.frametail_theme || !config.frametail_theme[0]) {
		wlr_log(WLR_ERROR,
			"Frametail is enabled but frametail_theme is not configured");
		return false;
	}
	struct frametail_lua_runtime *staged_runtime =
		frametail_lua_runtime_create(&(struct frametail_lua_host){
			.action = mango_frametail_action,
			.log = mango_frametail_log,
		});
	if (!staged_runtime) {
		wlr_log(WLR_ERROR, "Frametail: failed to create Lua runtime");
		return false;
	}
	char error[512];
	if (!frametail_lua_runtime_load_file(staged_runtime,
			config.frametail_theme, error, sizeof(error))) {
		wlr_log(WLR_ERROR, "Frametail: failed to load '%s': %s",
			config.frametail_theme, error);
		frametail_lua_runtime_destroy(staged_runtime);
		return false;
	}
	frametail_staging = true;

	struct staged_decoration {
		Client *client;
		struct frametail_lua_decoration *handle;
		struct frametail_decoration *decoration;
		struct frametail_extents extents;
		struct wlr_box content;
	};
	size_t capacity = wl_list_length(&clients);
	struct staged_decoration *staged =
		calloc(capacity ? capacity : 1, sizeof(*staged));
	if (!staged) {
		frametail_staging = false;
		frametail_lua_runtime_destroy(staged_runtime);
		return false;
	}
	size_t count = 0;
	wl_list_for_each(c, &clients, link) {
		if (client_is_unmanaged(c) || client_is_x11_popup(c))
			continue;
		struct frametail_lua_window window = mango_frametail_window(c);
		struct frametail_lua_decoration *handle = frametail_lua_runtime_build(
			staged_runtime, c->scene, &window, error, sizeof(error));
		if (!handle) {
			wlr_log(WLR_ERROR, "Frametail: failed to stage '%s': %s",
				client_get_title(c), error);
			for (size_t i = 0; i < count; i++)
				frametail_lua_decoration_destroy(staged[i].handle);
			free(staged);
			frametail_staging = false;
			frametail_lua_runtime_destroy(staged_runtime);
			return false;
		}
		struct frametail_decoration *decoration =
			frametail_lua_decoration_get(handle);
		if (!decoration) {
			frametail_lua_decoration_destroy(handle);
			for (size_t i = 0; i < count; i++)
				frametail_lua_decoration_destroy(staged[i].handle);
			free(staged);
			frametail_staging = false;
			frametail_lua_runtime_destroy(staged_runtime);
			return false;
		}
		wlr_scene_node_set_enabled(
			&frametail_decoration_scene_tree(decoration)->node, false);
		staged[count++] = (struct staged_decoration){
			.client = c,
			.handle = handle,
			.decoration = decoration,
			.extents = frametail_decoration_get_extents(decoration),
			.content = client_content_box(c, c->geom),
		};
	}
	frametail_staging = false;

	struct frametail_lua_runtime *old_runtime = frametail_runtime;
	frametail_runtime = staged_runtime;
	for (size_t i = 0; i < count; i++) {
		c = staged[i].client;
		mango_frametail_destroy(c);
		c->frametail_handle = staged[i].handle;
		c->frametail_decoration = staged[i].decoration;
		c->frametail_extents = staged[i].extents;
		c->bw = 0;
		c->geom.x = staged[i].content.x - c->frametail_extents.left;
		c->geom.y = staged[i].content.y - c->frametail_extents.top;
		c->geom.width = MANGO_MAX(1, staged[i].content.width) +
			(c->isfullscreen ? 0 : c->frametail_extents.left +
				c->frametail_extents.right);
		c->geom.height = MANGO_MAX(1, staged[i].content.height) +
			(c->isfullscreen ? 0 : c->frametail_extents.top +
				c->frametail_extents.bottom);
		mango_frametail_sync_geometry(c, c->geom);
		resize(c, c->geom, 0);
	}
	free(staged);
	if (old_runtime)
		frametail_lua_runtime_destroy(old_runtime);
	return true;
}

static bool mango_frametail_pointer_motion(Client *c) {
	if (frametail_hover_client && frametail_hover_client != c &&
		frametail_hover_client->frametail_decoration)
		frametail_decoration_pointer_leave(
			frametail_hover_client->frametail_decoration);
	frametail_hover_client = NULL;
	if (!c || !c->frametail_decoration || c->isfullscreen)
		return false;
	struct wlr_scene_tree *tree =
		frametail_decoration_scene_tree(c->frametail_decoration);
	int32_t lx, ly;
	if (!wlr_scene_node_coords(&tree->node, &lx, &ly))
		return false;
	bool handled = frametail_decoration_pointer_motion(c->frametail_decoration,
		cursor->x - lx, cursor->y - ly);
	if (handled)
		frametail_hover_client = c;
	return handled;
}

static bool mango_frametail_pointer_button(Client *c,
		struct wlr_pointer_button_event *event) {
	bool owned_release = event->state == WL_POINTER_BUTTON_STATE_RELEASED &&
		frametail_pressed_client;
	if (owned_release)
		c = frametail_pressed_client;
	if (!c || !c->frametail_decoration || c->isfullscreen) {
		if (owned_release && c && c->frametail_decoration)
			frametail_decoration_pointer_button(c->frametail_decoration,
				INT32_MAX, INT32_MAX, event->button, false,
				event->time_msec, 0);
		if (owned_release)
			frametail_pressed_client = NULL;
		return owned_release;
	}
	struct wlr_scene_tree *tree =
		frametail_decoration_scene_tree(c->frametail_decoration);
	int32_t lx, ly;
	if (!wlr_scene_node_coords(&tree->node, &lx, &ly)) {
		if (owned_release) {
			frametail_decoration_pointer_button(c->frametail_decoration,
				INT32_MAX, INT32_MAX, event->button, false,
				event->time_msec, 0);
			frametail_pressed_client = NULL;
		}
		return owned_release;
	}
	struct wlr_box content = client_content_box(c, c->geom);
	bool handled = frametail_decoration_pointer_button(c->frametail_decoration,
		cursor->x - lx, cursor->y - ly, event->button,
		event->state == WL_POINTER_BUTTON_STATE_PRESSED, event->time_msec, 0);
	if (event->state == WL_POINTER_BUTTON_STATE_PRESSED && handled)
		frametail_pressed_client = c;
	if (event->state == WL_POINTER_BUTTON_STATE_RELEASED)
		frametail_pressed_client = NULL;
	c->frametail_decoration = frametail_lua_decoration_get(c->frametail_handle);
	if (!c->frametail_decoration)
		mango_frametail_fallback(c, content);
	return handled;
}
