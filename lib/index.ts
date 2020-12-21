const playcap = require('bindings')('playcap.node')

export enum DeviceType {
  Playback = 1,
  Capture = 2,
  Duplex = 3,
};

export enum Backend {
  Wasapi = 0,
  Dsound = 1,
  Winmm = 2,
  Coreaudio = 3,
  Sndio = 4,
  Audio4 = 5,
  Oss = 6,
  Pulseaudio = 7,
  Alsa = 8,
  Jack = 9,
  Aaudio = 10,
  Opensl = 11,
  Webaudio = 12,
  Custom = 13,
  Null = 14,
};

export type DeviceInfo = {
  name: string;
  isDefault: boolean;
}

export type DeviceOptions = {
  playbackChannels?: number;
  captureChannels?: number;
  sampleRate?: number;
  playbackDeviceIndex?: number;
  captureDeviceIndex?: number;
  deviceType?: number;
}

type Device = {
  start: () => void;
  destroy: () => void;
}

export type DataCallback = (inputs: Float32Array[], outputs: Float32Array[]) => void;

export interface Context {
  getDevices(): DeviceInfo[];
  refreshDevices(): void;
  createDevice(options: DeviceOptions, cb: DataCallback): Device;
}

export var Context: {
  new(): Context
} = playcap.Context;

