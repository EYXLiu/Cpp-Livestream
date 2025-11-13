# To run
*This app is solely built for Macos* <br/>
Make sure to have `OpenCV` and `OpenSSL` installed <br/>
Run the following 
```
mkdir build
cd build
cmake ..
make
./ls
```
Endpoints can be found at `http://localhost:8080` for http and `ws://localhost:9090` for websocket <br/>

# Features
Multithreaded video capturing, processing, and streaming pipelines <br/>
Optional ML integration using OpenCV or custom ML models <br/>
Real-time streaming over HTTP and WS at 30FPS <br/>
Thread safe frame pipelines <br/>
Designed for multiple concurrent clients <br/>

# ML Integration
Place the OpenCV model into repository, change Model constructor to `Model mdl(<path to model>);` <br/>
- Requires file to be .XML but it is possible to change code to accomodate for DNN files (`.prototxt` + `.caffemodel`, `.pb`, `.pbtext`, `.onnx`, `.cfg` + `.weights`) <br/>
- Dynamic casting will be implemented in the future
