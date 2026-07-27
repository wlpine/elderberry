local function icon(kind)
    local pixels = {}
    for y = 0, 15 do
        for x = 0, 15 do
            local paint
            if kind == "minimize" then
                paint = x >= 3 and x <= 12 and y >= 11 and y <= 12
            elseif kind == "maximize" then
				paint = x >= 3 and x <= 12 and y >= 3 and y <= 12 and
					(x <= 4 or x >= 11 or y <= 4 or y >= 11)
			elseif kind == "restore" then
				paint = (x >= 3 and x <= 10 and y >= 5 and y <= 12 and
					(x <= 4 or x >= 9 or y <= 6 or y >= 11)) or
					(x >= 6 and x <= 12 and y >= 3 and y <= 9 and
					(y <= 4 or x >= 11))
            else
                paint = x >= 3 and x <= 12 and y >= 3 and y <= 12 and
                    (math.abs(x - y) <= 1 or math.abs(15 - x - y) <= 1)
            end
            pixels[#pixels + 1] = paint and
                string.char(242, 245, 249, 255) or string.char(0, 0, 0, 0)
        end
    end
    return frametail.texture_argb(16, 16, table.concat(pixels))
end

return function(window)
    local decoration = frametail.decoration {
        border = { placement = "content", width = 2,
            color = "#f0a030ff", radius = 9 },
        content_radius = 9,
        shadow = { size = 14, sigma = 10, color = "#00000088" },
    }
	local top = decoration:add_bar {
        edge = "top", thickness = 38, padding = 8,
        background = "#171a22f2", radius = { 9, 9, 0, 0 },
        on_press = "move",
	}
	local app_icon = top:add(frametail.custom {
		zone = "start", extent = 24, texture = window.icon,
	})
    local title = top:add(frametail.text {
        flex = true, text = window.title,
        font = "Sans Bold 10", foreground = "#f2f5f9ff",
    })
    top:add(frametail.button {
        zone = "end", extent = 32, texture = icon("minimize"),
        radius = 6, hover = "#ffffff20", on_click = "minimize",
    })
	local maximize_icon = icon("maximize")
	local restore_icon = icon("restore")
	local maximize = top:add(frametail.button {
		zone = "end", extent = 32, texture = maximize_icon,
        radius = 6, hover = "#ffffff20", on_click = "maximize",
    })
    top:add(frametail.button {
        zone = "end", extent = 32, texture = icon("close"),
        radius = 6, hover = "#e74c3cee", on_click = "close",
    })
	local function update(updated)
		title:set_text(updated.title)
		app_icon:set_texture(updated.icon)
		maximize:set_texture(updated.maximized and restore_icon or maximize_icon)
		local border = updated.urgent and "#e74c3cff" or
			(updated.focused and "#f0a030ff" or "#596273ff")
		decoration:set_frame_style { border = { color = border } }
		top:set_style { background = updated.focused and
			"#171a22f2" or "#11141bf2" }
		title:set_style { foreground = updated.focused and
			"#f2f5f9ff" or "#9da5b4ff" }
        top:set_visible(not updated.fullscreen)
	end
	update(window)
	return decoration, update
end
