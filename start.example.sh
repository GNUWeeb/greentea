#!/bin/sh

export LD_LIBRARY_PATH="${PWD}/submodules/tdlib/td/build/install/lib/";
export TGBOT_API_ID=123123123;
export TGBOT_API_HASH=4b05311c50c83a1894684662a95adcc5;
export TGBOT_DATA_DIR=storage/tgbot/62123123123;
./greentea;
