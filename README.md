This branch is using espressif brookesia component as a main UI/UX, which is really cool component.


## Description
This is demo of ble mesh provisioner on esp32 P4. 
Provisioner has been tested with 10x esp32-s3 mesh nodes with mixed model types.

Working features:
- scan for unprovisioned mesh devices and provisioning
- one net and app key are added during provisioning
- reset node - remove from mesh and provisioner list
- list provisioned nodes
- list elements and models in node view
- in model view
    - bind and unbind app key 0
    - subscribe to group address (3 hardcoded groups possible 0xC000-0xC002)
    - view and remove subscribed groups - after reset esp32-p4 info about subs is lost, but nodes still keep that info
    - perform action per model type (on/off, level etc)
- list models and control its state, also possible to control state with group address
    - on/off server
    - level server
    - hsl server light
    - location server (local and global) - only set option, missing get
    - time server - send current time from provisioner to all time setup server nodes (hardcoded address 0xFFFF)

Tested on esp-idf v5.5+ commit 0f0068fff3ab159f082133aadfa9baf4fc0c7b8d
Since its easy to control mesh nodes by default mesh is with persistent store data enabled.

## Models
All models should be possible to bind with app key 0 and subscribed to group address. Only few types has its own controls:
- [x] on/off model
- [x] level model
- [x] light HSL model
- [x] position model
- [ ] scenes


## TODO
To make this demo more complex here is list of todo features:
- configuration for config client model


## Issues
There is one small issue with `esp_hosted` component:
- `35 | #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 3, 0)` - the fix i am using now is to change this line with `#if 1`

Also esp-idf is not compatible with C++ in `net.h:475`
```
struct bt_mesh_net_rx rx = { .ctx.recv_rssi = rssi };

to

struct bt_mesh_net_rx rx = {  };
rx.ctx.recv_rssi = rssi;
```
