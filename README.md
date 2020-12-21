# PLAY🔊CAP🎙

> Native access to speaker and microphone for Node.js.

Playcap is a module that provides native access to the microphone and speaker in a separate thread(s).
Playcap is built on top of the [miniaudio](https://miniaud.io/) C library,
so it supports a lot of [platform-specific backends](https://miniaud.io/docs/manual/index.html#Backends).
Playcap working only with 32-bit floating-point format with `[-1; 1]` range.

## Roadmap

- [x] add the ability to select a backend
- [ ] add the ability to stop (basically pause) device
- [ ] add the ability to enable metadata for data callback(e.g. sampleRate, delta, currentTime)
- [ ] add the ability to use fixed sample buffer size

## Installation

Install `playcap` using npm:

```bash
npm install playcap
```

## Introduction

First of all, if you are not familiar with the basics of DSP, (sample rate, bit depth, amplitude, frequency and so on) you can watch very good introduction videos in [this youtube playlist](https://www.youtube.com/watch?v=Ov3GXhorrJE&list=PLLgJJsrdwhPwLOC5aNH8hLTlQYOeESORS). If you want go further I highly recommend the [Hack Audio](https://www.routledge.com/Hack-Audio-An-Introduction-to-Computer-Programming-and-Digital-Signal-Processing/Tarr/p/book/9781138497559) book.

### Your first beep to start somewhere

Just a simple example that will beep for a second and then will be terminated.

```js
const { Context, DeviceType } = require('playcap')

const SAMPLE_RATE = 44100;

const ctx = new Context()

const delta = 1.0 / SAMPLE_RATE
let currentTime = 0

const device = ctx.createDevice({
  sampleRate: SAMPLE_RATE,
  deviceType: DeviceType.Playback,
}, (input, output) => {
  for (let i = 0; i < output[0].length; i++) {
    const sample = Math.sin(Math.PI * 2 * 440 * currentTime)

    for (let channel = 0; channel < output.length; channel++) {
      output[channel][i] = sample
    }

    currentTime += delta
  }
})

device.start()

setTimeout(() => {
  device.destroy()
}, 1000)
```

You can notice several concepts in this code.  First of all, it is `Context`. It represents an audio backend, for example, it can be CoreAudio for MacOs or ALSA for Linux. There is also another concept - `Device`. `Device` is an abstraction for a physical device that can capture *or/and* play audio. One `Context` can have multiple `Devices`, but only one `Context` for a particular `Device`. The last concept is `DeviceType` which determines how we will work with the device. In this case we use `Playback` type which tells us that we are only interested in the output.

The overall chain is quite simple. We create `Context`, then we create a `Device` based on `Context` and pass `data callback` (the function which will process audio) to the created device.

The `data callback` is a function that takes `input` in `output`. And the input and output are just arrays of `Float32Array`. Each `Float32Array` represents the audio channel,  so the `output` array for 2 channels device will contain two `Float32Array` elements. You can notice that we don't have any information about the device in data callback (e.g. sample rate, delta, current time), so you can manage it manually (it will be probably added in future versions).

### Dealing with device types

There are three device types:
  - Playback
  - Capture
  - Duplex

We have already met the first of them. The `Playback` device type means we are not interesting inputs, so it always will be zeros in the input`Float32Array`.


In contrast to the `Playback` type, the `Capture` type only provides us with input data.  Writing to `output` elements will not affect anything. This will be useful, for example, if you want to record your banjo🪕 playing to an audio file.


The last one is quite interesting. `Duplex` device type provides access to both the input and output. So `input` elements will contain audio data from the microphone and `output` elements will provide writing access to the speaker. You can use this type if you want to hear how loud it is when your finger strikes the microphone membrane. One important note. You can have completely different physical devices, for example, external mic and speakers, but you can combine them into one virtual duplex device.

Even more easier example

```js
const { Context, DeviceType } = require('playcap')

const ctx = new Context()

const device = ctx.createDevice({
  sampleRate: 44100,
  captureChannels: 1,
  deviceType: DeviceType.Duplex,
}, (input, output) => {
  for (let i = 0; i < input[0].length; i++) {
    for (let channel = 0; channel < output.length; channel++) {
      output[channel][i] = input[0][i]
    }
  }
})

device.start()

process.on('SIGINT', () => {
  device.destroy() // don't forget do that :)
})
```

We just copy numbers from one input channel to *N* output channels, that's it.


## Api

#### Exports
  - `Context`
  - `DeviceType`
  - `Backend`

#### Context

`new Context(backend: Backend): Context`

**Options**:

  - `backend: Backend` - backend type

**Methods**:
  - `getDevices(): DeviceInfo[]` - returns list of devices
  - `refreshDevices(): void` - refresh list of devices
  - `createDevice(options: DeviceOptions, dataCallback: DataCallback): Device` - returns `Device` instance

**Returns**: `Context` instance

---

#### DeviceType
  - `DeviceType.Playback`
  - `DeviceType.Capture`
  - `DeviceType.Duplex`

---

#### Backend
  - `Backend.Wasapi`
  - `Backend.Dsound`
  - `Backend.Winmm`
  - `Backend.Coreaudio`
  - `Backend.Sndio`
  - `Backend.Audio4`
  - `Backend.Oss`
  - `Backend.Pulseaudio`
  - `Backend.Alsa`
  - `Backend.Jack`
  - `Backend.Aaudio`
  - `Backend.Opensl`
  - `Backend.Webaudio`
  - `Backend.Custom`
  - `Backend.Null`

---

#### DeviceOptions
  - `playbackChannels: number` - The number of output channels. Decided automatically if not specified
  - `captureChannels: number` - The number of input channels. Decided automatically if not specified
  - `sampleRate: number` - device sample rate
  - `playbackDeviceIndex: number` - index of playback device. The list of devices can be obtained by `getDevices` method. Decided automatically if not specified
  - `captureDeviceIndex: number` - index of capture device. The list of devices can be obtained by `getDevices` method. Decided automatically if not specified
  - `deviceType: DeviceType` - device type: `Playback`, `Capture` or `Duplex`

---

#### DeviceInfo
  - `name: string` - device name
  - `isDefault: boolean` - device is the default device
---

#### DataCallback
Data processing callback

`(inputs: Float32Array[], outputs: Float32Array[]) => void;`

---

#### Device

  **Methods**
  - `start(): void` - run device
  - `destroy(): void` - destroy device. **DO NOT** use this method inside data callback, it will cause deadlock.
