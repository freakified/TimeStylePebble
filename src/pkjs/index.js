var platform = 'basalt';
try { platform = Pebble.getActiveWatchInfo().platform; } catch (err) {}

if (platform === 'aplite') {
  require('./legacy/index');
} else {
  require('./app');
}
