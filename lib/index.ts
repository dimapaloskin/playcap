const playcap = require('bindings')('playcap.node')

export enum DeviceType {
  Playback = 1,
  Capture = 2,
  Duplex = 3,
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

