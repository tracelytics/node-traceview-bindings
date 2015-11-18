var spawn = require('child_process').spawn
var fs = require('fs')

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

function isDebian () {
  return fs.existsSync('/etc/debian_version')
}

function isRedHat () {
  return (
    fs.existsSync('/etc/redhat-release')
    || fs.existsSync('/etc/fedora-release')
    || fs.existsSync('/etc/system-release-cpe')
  )
}

function concat (stream, cb) {
  var data = []
  stream.stdout.on('data', data.push.bind(data))
  stream.on('error', cb)
  stream.on('close', function (err) {
    cb(err, data.map(function (v) { return v.toString() }).join(''))
  })
}

function detectDebian (cb) {
  var p = spawn('dpkg-query', [
    '-W',
    '-f',
    '\'${Version}\'',
    'gcc'
  ])

  concat(p, function (err, res) {
    if (err) return cb(new Error('gcc not found'))
    cb(null, res.split(':').pop().split('-').shift())
  })
}

function detectRedHat (cb) {
  var p = spawn('rpm', [
    '-q',
    'gcc'
  ])

  concat(p, function (err, res) {
    if (err) return cb(new Error('gcc not found'))
    var parts = res.split('-')
    parts.pop()
    cb(null, parts.pop())
  })
}

function detect (stream, cb) {
  if (stream.isTTY) {
    var spin = spinner(15, function (c) {
      stream.clearLine()
      stream.cursorTo(0)
      stream.write(c + ' detecting gcc')
    })
  }

  function done (err, res) {
    if (stream.isTTY) {
      spin.stop()
      stream.clearLine()
      stream.cursorTo(0)
    }

    if (err) return cb(err)

    // Must be at least 4.8
    var parts = res.split('.').map(Number)
    if (parts[0] < 4 || parts[1] < 8) {
      cb(new Error('TraceView requires gcc 4.8+'))
      return
    }

    cb(null, res)
  }

  if (isDebian()) {
    detectDebian(done)
  } else if (isRedHat()) {
    detectRedHat(done)
  } else {
    done(new Error('Unrecognized distro'))
  }
}

function build (stream, cb) {
  var p = spawn('node-gyp', ['rebuild'])

  if (stream.isTTY) {
    var spin = spinner(15, function (c) {
      stream.clearLine()
      stream.cursorTo(0)
      stream.write(c + ' building TraceView native bindings')
    })
  }

  p.on('close', function (err) {
    if (stream.isTTY) {
      spin.stop()
      stream.clearLine()
      stream.cursorTo(0)
    }

    if (err) {
      console.warn('TraceView oboe library not found, tracing disabled')
    } else {
      console.log('TraceView bindings built successfully')
    }

    cb()
  })
}

build.detect = detect

function fullBuild (cb) {
  detect(process.stdout, function (err, res) {
    if (err) {
      console.warn(err.message)
      return
    }

    console.log('Found gcc ' + res)

  	build(process.stdout, function (err) {
      if (err) {
        console.warn('TraceView oboe library not found, tracing disabled')
      } else {
        console.log('TraceView bindings built successfully')
      }
    })
  })
}

if ( ! module.parent) {
  fullBuild(function (err) {
    if (err) {
      console.error(err.message)
      process.exit(1)
    }
  })
} else {
	module.exports = fullBuild
}
