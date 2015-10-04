#!/bin/bash

cd $(dirname "$0") || exit 1

cur_dir=`pwd`

proto_file_dir=(proto storage conf)

PT=
for dir in ${proto_file_dir[@]}
do
	PT=$PT" --proto_path="$dir
done

new_file_dir=pb/new
old_file_dir=pb

mkdir -p $new_file_dir
mkdir -p $old_file_dir

function gen()
{
	dir=$1
	
	find $dir -maxdepth 1 -iname "*.proto" | while read proto_file;
	do
		base_name=`basename $proto_file .proto`
		#protoc --proto_path=$dir --proto_path=$cur_dir --cpp_out=$new_file_dir $proto_file
		protoc $PT --cpp_out=$new_file_dir $proto_file

		if [ $? -ne 0 ]; then
			echo "process $proto_file error!"
			exit 1;
		fi

		new_file=$new_file_dir/$base_name.pb.h
		old_file=$old_file_dir/$base_name.pb.h
		diff $new_file $old_file > /dev/null 2>&1 || mv -v $new_file $old_file

		new_file=$new_file_dir/$base_name.pb.cc
		old_file=$old_file_dir/$base_name.pb.cc
		diff $new_file $old_file > /dev/null 2>&1 || mv -v $new_file $old_file
	done
}

for dir in ${proto_file_dir[@]}
do
	echo "generating protocol files in $dir ..."
	gen $dir
done


rm -rf $new_file_dir
