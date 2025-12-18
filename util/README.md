### Util

These are here to make developing on raspberry pi easier.

To configure:
```sh
cp util/.microtone_env "$HOME/.microtone_env"
chmod 644 ~/.microtone_env

sudo cp setup_devenv.sh /usr/local/bin/setup_devenv
sudo cp pull_microtone.sh /usr/local/bin/pull_microtone
sudo cp build_microtone.sh /usr/local/bin/build_microtone
sudo cp run_microtone.sh /usr/local/bin/run_microtone
sudo cp dev_microtone.sh /usr/local/bin/dev_microtone

chmod +x /usr/local/bin/setup_devenv \
         /usr/local/bin/pull_microtone.sh \
         /usr/local/bin/build_microtone.sh \
         /usr/local/bin/run_microtone.sh \
         /usr/local/bin/dev_microtone.sh
```

Run this once to set up the development environment:
```sh
setup_devenv
```

And this to iterate on upstream changes:
```sh
dev_microtone
```
