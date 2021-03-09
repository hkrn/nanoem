nanoem_dir="$(cd "$(dirname "$BASH_SOURCE")"; cd ..; pwd -P)"

function nanoem_decode_command() {
  echo $1
  path=$nanoem_dir/emapp/resources/protobuf
  cat $1 | protoc -I $path --decode=nanoem.application.Command $path/application.proto
}

function nanoem_decode_effect() {
  echo $1
  path=$nanoem_dir/emapp/resources/protobuf
  cat $1 | protoc -I $path --decode=fx9.effect.Effect $path/effect.proto
}
export -f nanoem_decode_effect

function nanoem_decode_motion() {
  echo $1
  path=$nanoem_dir/nanoem/proto
  cat $1 | protoc -I $path --decode=nanoem.motion.Motion $path/motion.proto
}
export -f nanoem_decode_motion

function nanoem_decode_plugin() {
  echo $1
  path=$nanoem_dir/emapp/resources/protobuf
  cat $1 | protoc -I $path --decode=nanoem.application.plugin.UIWindow $path/plugin.proto
}
export -f nanoem_decode_plugin

function nanoem_decode_project() {
  echo $1
  path=$nanoem_dir/emapp/resources/protobuf
  cat $1 | protoc -I $path --decode=nanoem.project.Project $path/project.proto
}
export -f nanoem_decode_project

