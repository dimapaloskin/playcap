{
    "targets": [{
        "target_name": "playcap",
        "include_dirs": ["./miniaudio"],
        "sources": [
            "./src/addon.c",
            "./src/napi_utils.c",
            "./src/utils.c",
            "./src/context.c",
            "./src/device.c"
        ]
    }]
}
