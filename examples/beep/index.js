const { Context, DeviceType } = require('../../dist')

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
