'use strict';

const {isUtf8} = require('node:buffer');

// Stream chunks are byte boundaries, not character boundaries. Keep the exact
// bytes until the process closes, then decode the complete output once.
function captureOutput(stream) {
  const chunks = [];
  stream.on('data', chunk => {
    if (!(chunk instanceof Uint8Array))
      throw new TypeError('Process output must be captured as bytes');
    chunks.push(Buffer.from(chunk));
  });
  const bytes = () => Buffer.concat(chunks);
  return {
    bytes,
    text() {
      const output = bytes();
      if (!isUtf8(output)) throw new TypeError('Process output is not valid UTF-8');
      return output.toString('utf8');
    },
  };
}

module.exports = {captureOutput};
