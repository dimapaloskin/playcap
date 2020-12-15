const fs = require('fs')
const getFileHeaders = require('wav-headers');
const { Context, DeviceType } = require('../../dist')

const { clamp } = require('./utils')

const filename = './out.wav'
const SAMPLE_RATE = 44100
const BIT_DEPTH = 16
const scaler = Math.pow(2, BIT_DEPTH) / 2

const stream = fs.createWriteStream(filename)

const header = getFileHeaders({
  channels: 1,
  sampleRate: SAMPLE_RATE,
  bitDepth: BIT_DEPTH,
})

stream.write(header, 'binary')

const ctx = new Context()

const device = ctx.createDevice({
  sampleRate: SAMPLE_RATE,
  deviceType: DeviceType.Capture,
  captureChannels: 1,
}, (input) => {
  const buf = Buffer.alloc(input[0].length * (BIT_DEPTH / 8))

  for (let i = 0; i < input[0].length; i++) {
    const sample = clamp(input[0][i])
    const val = (sample * scaler)|0
    buf.writeInt16LE(val, i * 2)
  }

  stream.write(buf, 'binary')
})

device.start()

process.on('SIGINT', () => {
  stream.end()
  device.destroy()

  console.log(`Recorded to ${filename}`)

  process.exit()
})
