### Build Docker image

```bash
./build_docker_image.sh
```

### Build locally

```bash
meson setup build .
meson compile -C build
```

### Usage examples

```bash
jsonbomb -v -X POST -H "Cookie: yummy_cookie=choco; tasty_cookie=strawberry" https://www.google.com
```


