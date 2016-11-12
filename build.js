var spawn = require('child_process').spawn

var chars = '⠋⠙⠹⠸⠼⠴⠦⠧⠇⠏'
var len = chars.length
function spinner (fps, fn) {
  var frame = 0

  var t = setInterval(function () {
    fn(chars[frame++ % len])
  }, 1000 / fps)

  return {
    stop: function () { clearInterval(t) }
  }
}

function detect (cb) {
  var p = spawn('sh', ['./detect.sh'])
  p.stdout.pipe(process.stdout)
  p.stderr.pipe(process.stderr)
  p.on('close', function (err) {
    if (err) {
      console.warn('Environment detection failed, TraceView native bindings may be unable to build')
    }
    build(cb)
  })
}

function build (cb) {
  var p = spawn('node-gyp', ['rebuild'])

  if (process.stdout.isTTY) {
    var spin = spinner(15, function (c) {
      process.stdout.clearLine()
      process.stdout.cursorTo(0)
      process.stdout.write(c + ' building TraceView native bindings')
    })
  }

  if (process.env.TRACEVIEW_BUILD_VERBOSE) {
    p.stdout.pipe(process.stdout)
    p.stderr.pipe(process.stderr)
  }

  p.on('close', function (err) {
    if (process.stdout.isTTY) {
      spin.stop()
      process.stdout.clearLine()
      process.stdout.cursorTo(0)
    }

    if (err) {
      console.warn('Unable to build TraceView native bindings')
    } else {
      console.log('TraceView bindings built successfully')
    }

    cb()
  })
}

if ( ! module.parent) {
	detect(function () {})
} else {
	module.exports = build
}
