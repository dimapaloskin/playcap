const { Context, DeviceType } = require('../../dist')

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
