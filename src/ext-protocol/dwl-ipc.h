#include "dwl-ipc-unstable-v2-protocol.h"

static void dwl_ipc_output_printstatus_to(DwlIpcOutput *ipc_output);

static void dwl_ipc_manager_resource_destroy(struct wl_resource *resource) {
	(void)resource;
}

static void dwl_ipc_output_resource_destroy(struct wl_resource *resource) {
	DwlIpcOutput *ipc_output = wl_resource_get_user_data(resource);
	if (!ipc_output)
		return;
	wl_list_remove(&ipc_output->link);
	free(ipc_output);
}

static void dwl_ipc_manager_release(struct wl_client *client,
		struct wl_resource *resource) {
	(void)client;
	wl_resource_destroy(resource);
}

static void dwl_ipc_output_release(struct wl_client *client,
		struct wl_resource *resource) {
	(void)client;
	wl_resource_destroy(resource);
}

static void dwl_ipc_output_set_tags(struct wl_client *client,
		struct wl_resource *resource, uint32_t tagmask, uint32_t toggle_tagset) {
	(void)client;
	DwlIpcOutput *ipc_output = wl_resource_get_user_data(resource);
	if (ipc_output) {
		if (!toggle_tagset)
			ipc_output->mon->seltags ^= 1;
		view_in_mon(&(Arg){.ui = tagmask & TAGMASK}, true, ipc_output->mon, true);
	}
}

static void dwl_ipc_output_set_client_tags(struct wl_client *client,
		struct wl_resource *resource, uint32_t and_tags, uint32_t xor_tags) {
	(void)client;
	DwlIpcOutput *ipc_output = wl_resource_get_user_data(resource);
	Client *selected = ipc_output ? focustop(ipc_output->mon) : NULL;
	if (!selected)
		return;
	uint32_t tags = (selected->tags & and_tags) ^ xor_tags;
	if (!tags)
		return;
	selected->tags = tags;
	focusclient(focustop(ipc_output->mon), 1);
	arrange(ipc_output->mon, false, false);
	printstatus(IPC_WATCH_ARRANGGE);
}

static void dwl_ipc_output_set_layout(struct wl_client *client,
		struct wl_resource *resource, uint32_t index) {
	(void)client;
	DwlIpcOutput *ipc_output = wl_resource_get_user_data(resource);
	if (!ipc_output)
		return;
	if (index >= LENGTH(layouts))
		index = 0;
	Monitor *m = ipc_output->mon;
	m->pertag->ltidxs[m->pertag->curtag] = &layouts[index];
	clear_fullscreen_and_maximized_state(m);
	arrange(m, false, false);
	printstatus(IPC_WATCH_ARRANGGE);
}

static void dwl_ipc_output_quit(struct wl_client *client,
		struct wl_resource *resource) {
	(void)client;
	(void)resource;
	quit(&(Arg){0});
}

static void dwl_ipc_output_dispatch(struct wl_client *client,
		struct wl_resource *resource, const char *dispatch, const char *arg1,
		const char *arg2, const char *arg3, const char *arg4, const char *arg5) {
	(void)client;
	(void)resource;
	Arg arg = {0};
	int32_t (*func)(const Arg *) = parse_func_name((char *)dispatch, &arg,
		(char *)arg1, (char *)arg2, (char *)arg3, (char *)arg4, (char *)arg5);
	if (func)
		func(&arg);
	free(arg.v);
	free(arg.v2);
	free(arg.v3);
}

static const struct zdwl_ipc_output_v2_interface dwl_output_implementation = {
	.release = dwl_ipc_output_release,
	.set_tags = dwl_ipc_output_set_tags,
	.set_client_tags = dwl_ipc_output_set_client_tags,
	.set_layout = dwl_ipc_output_set_layout,
	.quit = dwl_ipc_output_quit,
	.dispatch = dwl_ipc_output_dispatch,
};

static void dwl_ipc_manager_get_output(struct wl_client *client,
		struct wl_resource *resource, uint32_t id, struct wl_resource *output) {
	struct wlr_output *wlr_output = wlr_output_from_resource(output);
	Monitor *m = wlr_output ? wlr_output->data : NULL;
	if (!m)
		return;
	struct wl_resource *output_resource = wl_resource_create(client,
		&zdwl_ipc_output_v2_interface, wl_resource_get_version(resource), id);
	if (!output_resource) {
		wl_client_post_no_memory(client);
		return;
	}
	DwlIpcOutput *ipc_output = ecalloc(1, sizeof(*ipc_output));
	ipc_output->resource = output_resource;
	ipc_output->mon = m;
	wl_resource_set_implementation(output_resource, &dwl_output_implementation,
		ipc_output, dwl_ipc_output_resource_destroy);
	wl_list_insert(&m->dwl_ipc_outputs, &ipc_output->link);
	dwl_ipc_output_printstatus_to(ipc_output);
}

static const struct zdwl_ipc_manager_v2_interface dwl_manager_implementation = {
	.release = dwl_ipc_manager_release,
	.get_output = dwl_ipc_manager_get_output,
};

static void dwl_ipc_manager_bind(struct wl_client *client, void *data,
		uint32_t version, uint32_t id) {
	(void)data;
	struct wl_resource *resource = wl_resource_create(client,
		&zdwl_ipc_manager_v2_interface, version, id);
	if (!resource) {
		wl_client_post_no_memory(client);
		return;
	}
	wl_resource_set_implementation(resource, &dwl_manager_implementation, NULL,
		dwl_ipc_manager_resource_destroy);
	zdwl_ipc_manager_v2_send_tags(resource, LENGTH(tags));
	for (size_t i = 0; i < LENGTH(layouts); i++)
		zdwl_ipc_manager_v2_send_layout(resource, layouts[i].symbol);
}

static void dwl_ipc_output_printstatus(Monitor *m) {
	DwlIpcOutput *ipc_output;
	wl_list_for_each(ipc_output, &m->dwl_ipc_outputs, link)
		dwl_ipc_output_printstatus_to(ipc_output);
}

static void dwl_ipc_output_printstatus_to(DwlIpcOutput *ipc_output) {
	Monitor *m = ipc_output->mon;
	Client *focused = focustop(m);
	zdwl_ipc_output_v2_send_active(ipc_output->resource, m == selmon);
	for (uint32_t tag = 0; tag < LENGTH(tags); tag++) {
		uint32_t mask = 1u << tag;
		uint32_t state = mask & m->tagset[m->seltags] ?
			ZDWL_IPC_OUTPUT_V2_TAG_STATE_ACTIVE : 0;
		uint32_t count = 0, focused_on_tag = 0;
		Client *c;
		wl_list_for_each(c, &clients, link) {
			if (c->mon != m || !(c->tags & mask))
				continue;
			count++;
			focused_on_tag |= c == focused;
			if (c->isurgent)
				state |= ZDWL_IPC_OUTPUT_V2_TAG_STATE_URGENT;
		}
		zdwl_ipc_output_v2_send_tag(ipc_output->resource, tag, state, count,
			focused_on_tag);
	}
	const Layout *layout = m->pertag->ltidxs[m->pertag->curtag];
	const char *symbol = m->isoverview ? overviewlayout.symbol : layout->symbol;
	zdwl_ipc_output_v2_send_layout(ipc_output->resource, layout - layouts);
	zdwl_ipc_output_v2_send_title(ipc_output->resource,
		focused ? client_get_title(focused) : "");
	zdwl_ipc_output_v2_send_appid(ipc_output->resource,
		focused ? client_get_appid(focused) : "");
	zdwl_ipc_output_v2_send_layout_symbol(ipc_output->resource, symbol);
	if (wl_resource_get_version(ipc_output->resource) >= 2) {
		zdwl_ipc_output_v2_send_fullscreen(ipc_output->resource,
			focused ? focused->isfullscreen : 0);
		zdwl_ipc_output_v2_send_floating(ipc_output->resource,
			focused ? focused->isfloating : 0);
		zdwl_ipc_output_v2_send_x(ipc_output->resource, focused ? focused->geom.x : 0);
		zdwl_ipc_output_v2_send_y(ipc_output->resource, focused ? focused->geom.y : 0);
		zdwl_ipc_output_v2_send_width(ipc_output->resource,
			focused ? focused->geom.width : 0);
		zdwl_ipc_output_v2_send_height(ipc_output->resource,
			focused ? focused->geom.height : 0);
		zdwl_ipc_output_v2_send_last_layer(ipc_output->resource,
			m->last_open_surface);
		struct wlr_keyboard *keyboard = &kb_group->wlr_group->keyboard;
		xkb_layout_index_t current = xkb_state_serialize_layout(
			keyboard->xkb_state, XKB_STATE_LAYOUT_EFFECTIVE);
		char kb_layout[32];
		get_layout_abbr(kb_layout,
			xkb_keymap_layout_get_name(keyboard->keymap, current));
		zdwl_ipc_output_v2_send_kb_layout(ipc_output->resource, kb_layout);
		zdwl_ipc_output_v2_send_keymode(ipc_output->resource, keymode.mode);
		zdwl_ipc_output_v2_send_scalefactor(ipc_output->resource,
			(uint32_t)(m->wlr_output->scale * 100));
	}
	zdwl_ipc_output_v2_send_frame(ipc_output->resource);
}
