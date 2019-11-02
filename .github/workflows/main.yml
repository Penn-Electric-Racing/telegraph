name: CI

on: [push]

jobs:
  build:
    runs-on: ubuntu-latest

    steps:
    - uses: actions/checkout@v1
    - name: Bazel Action
      uses: ngalaiko/bazel-action@1.1.0
      with:
        args: test //...
