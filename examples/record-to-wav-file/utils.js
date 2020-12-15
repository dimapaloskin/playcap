const clamp = (val) => {
  if (val > 1) {
    return 1
  }

  if (val < -1) {
    return -1
  }

  return val
}

module.exports = {
  clamp,
}
