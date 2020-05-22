#!/bin/bash

idf_version="${IDF_VERSION:-v3.2.3-kudzu-dev}"
toolchain_version="${TOOLCHAIN_VERSION:-1.22.0-80-g6c4433a-5.2.0}"

script_path="$( cd "$(dirname "$0")" ; pwd -P )"
dev_path="${script_path}/.dev-3.2.3"
sdk_path="${dev_path}/sdk"
tmp_path="${dev_path}/tmp"

toolchain_path="${sdk_path}/xtensa-esp32-elf"
toolchain_url="https://dl.espressif.com/dl/xtensa-esp32-elf-osx-${toolchain_version}.tar.gz"

idf_path="${sdk_path}/idf"
idf_url="https://github.com/kudzutechnologies/esp-idf"

activate_sh="${dev_path}/bin/activate"

function setup_toolchain {
  if [ ! -d "${toolchain_path}" ]; then
    echo "INFO: Installing xtensa ${toolchain_version} toolchain..." >&2
    curl -o "${tmp_path}/toolchain.tar.gz" "${toolchain_url}" >&2 || return 1
    tar -C "${sdk_path}" -vxf "${tmp_path}/toolchain.tar.gz" >&2 || return 1
    rm "${tmp_path}/toolchain.tar.gz" || return 1
    echo -e "\n# IDF Toolchain\nexport PATH=\"${toolchain_path}/bin:\$PATH\"" >> ${activate_sh}
  else
    echo "INFO: xtensa toolchain already installed" >&2
  fi

  return 0
}

function setup_sdk {
  if [ ! -d "${idf_path}" ]; then
    echo "INFO: Installing IDF ${idf_version} SDK..." >&2
    git clone -b "${idf_version}" --recursive "${idf_url}" "${idf_path}" >&2
    git -C "${idf_path}" submodule update --init --recursive >&2
    (
      source "${activate_sh}"
      pip install -r "${idf_path}/requirements.txt" >&2
      pip install colorama hexdump >&2
    ) || return 1
    echo -e "\n# IDF SDK\nexport IDF_PATH=\"${idf_path}\"" >> ${activate_sh}
  else
    echo "INFO: IDF SDK already installed" >&2
  fi
}

function setup_devdir {
  if [ ! -d "${dev_path}" ]; then
    if ! which python3 >/dev/null; then
      echo "ERROR: This script requires 'python3' to be installed"
      exit 1
    fi
    if ! python3 --version | grep '\b3\.[456789]' >/dev/null; then
      echo "ERROR: This script requires python3 >= 3.4"
      exit 1
    fi
    if ! python3 -m venv "${dev_path}" >&2; then
      echo "ERROR: Missing 'venv' module in your python3 installation"
      exit 1
    fi
    mkdir -p "${tmp_path}" "${sdk_path}"
  fi
}

setup_devdir
setup_toolchain
setup_sdk

echo "-------"
echo "Activate with: source $activate_sh"
