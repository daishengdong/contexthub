#
# Copyright (C) 2016 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

APP = nanoapp_sign
SRC = nanoapp_sign.c ../../firmware/src/rsa.c ../../firmware/src/sha2.c
CC ?= gcc

$(APP): $(SRC) Makefile
	$(CC) -o $(APP) -O2 $(SRC) -I../../firmware/inc -DRSA_SUPPORT_PRIV_OP_BIGRAM

clean:
	rm $(APP)
