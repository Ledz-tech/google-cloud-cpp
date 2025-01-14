#!/usr/bin/env bash
# Copyright 2019 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -eu

source "$(dirname "$0")/../../lib/init.sh"
source module /ci/lib/io.sh

if [[ $# -ne 2 ]]; then
  echo "Usage: $(basename "$0") <branch-name> <build-output>"
  exit 1
fi
readonly BRANCH="$1"
readonly BUILD_OUTPUT="$2"

# Exit successfully (and silently) if there are no documents to upload.
if [[ "${GENERATE_DOCS:-}" != "yes" ]]; then
  # No documentation generated by this build, skip upload.
  exit 0
fi

if [[ -n "${KOKORO_GITHUB_PULL_REQUEST_NUMBER:-}" ]]; then
  # Do not push new documentation on PR builds.
  exit 0
fi

# Allow the user to override the destination directory.
if [[ -n "${DOCS_SUBDIR:-}" ]]; then
  subdir="${DOCS_SUBDIR}/"
else
  subdir="latest/"
fi

echo "================================================================"
io::log "Uploading generated Doxygen docs to github.io [${subdir}]."

# The usual way to host documentation in ${GIT_NAME}.github.io/${PROJECT_NAME}
# is to create a branch (gh-pages) and post the documentation in that branch.
# We first do some general git configuration:

# Clone the gh-pages branch into a staging directory.
REPO_URL="$(git config remote.origin.url)"
readonly REPO_URL
if [[ ! -d cmake-out/github-io-staging ]]; then
  git clone -b gh-pages "${REPO_URL}" cmake-out/github-io-staging
else
  if [[ ! -d cmake-out/github-io-staging/.git ]]; then
    io::log_red "github-io-staging exists but it is not a git repository."
    exit 1
  fi
  (cd cmake-out/github-io-staging && git checkout gh-pages && git pull)
fi

readonly LIBRARIES=(common bigtable firestore pubsub storage spanner)
# Remove any previous content in the subdirectory used for this release. We will
# recover any unmodified files in a second.
(
  cd cmake-out/github-io-staging
  for lib in "${LIBRARIES[@]}"; do
    if [[ -d "${subdir}${lib}" ]]; then
      git rm -qfr --ignore-unmatch "${subdir}${lib}"
    fi
  done
)

io::log "Copy the build results into the gh-pages clone."
mkdir -p "cmake-out/github-io-staging/${subdir}"
for lib in "${LIBRARIES[@]}"; do
  if [[ "${lib}" == "common" ]]; then
    cp -r "${BUILD_OUTPUT}/google/cloud/html/." \
      "cmake-out/github-io-staging/${subdir}/common"
  else
    cp -r "${BUILD_OUTPUT}/google/cloud/${lib}/html/." \
      "cmake-out/github-io-staging/${subdir}${lib}"
  fi
done

cd cmake-out/github-io-staging
git config user.name "Google Cloud C++ Project Robot"
git config user.email "google-cloud-cpp-bot@users.noreply.github.com"
for lib in "${LIBRARIES[@]}"; do
  git add --all "${subdir}${lib}"
done

if git diff --quiet HEAD; then
  io::log "No changes to the documentation, skipping upload."
  exit 0
fi

git commit -q -m"Automatically generated documentation"

if [[ "${REPO_URL:0:8}" != "https://" ]]; then
  io::log "Repository is not in https:// format, attempting push to ${REPO_URL}"
  git push
  io::log "Documentation upload completed successfully"
  exit 0
fi

if [[ -z "${KOKORO_GFILE_DIR:-}" ]]; then
  echo "Will not upload documents as KOKORO_GFILE_DIR not set."
  exit 0
fi

if [[ ! -r "${KOKORO_GFILE_DIR}/github-io-upload-token" ]]; then
  echo "Will not upload documents as the upload token is not available."
  exit 0
fi

GH_TOKEN="$(cat "${KOKORO_GFILE_DIR}/github-io-upload-token")"
readonly GH_TOKEN

readonly REPO_REF=${REPO_URL/https:\/\//}
git push https://"${GH_TOKEN}@${REPO_REF}" gh-pages

io::log "Documentation upload completed successfully"
