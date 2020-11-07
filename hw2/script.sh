#!/bin/bash

if ! [[ -d ${1} ]]
then
	echo "${1} is not a valid directory."
	exit
fi

while IFS= read -r -d $'\0' name
do
	logFile="${1}/${name}.log"

	if ! [[ -f "${logFile}" ]]
	then
		echo ".log file for directory ${name} doesn't exist"
		continue
	fi

	counter=0
	firstLine=""
	description=""

	picArray=()
	while read picname
	do
		picArray+=("${picname}")
	done < <(find "${1}/${name}" -type f -name "*.jpg" -printf "%p\0" -exec stat -c %Z {} \; | sort -t '\0' -k 2 -n | cut -d '' -f 1)

	while read line
	do
		if [[ ${firstLine} == "" ]]
		then
			firstLine=${line}
			continue
		fi

		if [[ ${description} == "" ]]
		then
			description=${line}
			continue
		fi

		if [[ ${line} == "" ]]
		then
			dateString=${firstLine% *}
			coordinatesString=${firstLine#* }

			parsedDescription=$(echo -e "${description}" | tr A-Z a-z | sed 's/[^[0-9a-z ]*//g' | tr -s " " | tr " " "_")

			newPic="${1}/${name}/$(date -d @${dateString} +%Y-%m-%dT%H%M)_${parsedDescription}.jpg"
			mv ${picArray[${counter}]} ${newPic}

			if [[ $(type exiftool) ]]
			then
				latitude=${coordinatesString%,*}
				longitude=${coordinatesString#*,}
				exiftool -overwrite_original -GPSLatitude=${latitude} -GPSLongitude=${longitude} ${newPic}
			fi

			firstLine=""
			description=""
			counter=$(( counter + 1 ))
		fi
	done < "${logFile}"

	if [[ ${line} != "" ]]
	then
		dateString=${firstLine% *}
		coordinatesString=${firstLine#* }

		parsedDescription=$(echo -e "${description}" | tr A-Z a-z | sed 's/[^[0-9a-z ]*//g' | tr -s " " | tr " " "_")

		newPic="${1}/${name}/$(date -d @${dateString} +%Y-%m-%dT%H%M)_${parsedDescription}.jpg"
		mv ${picArray[${counter}]} ${newPic}

		if [[ $(type exiftool) ]]
		then
			latitude=${coordinatesString%,*}
			longitude=${coordinatesString#*,}
			exiftool -overwrite_original -GPSLatitude=${latitude} -GPSLongitude=${longitude} ${newPic}
		fi
	fi

done < <(find "${1}" -mindepth 1 -maxdepth 1 -type d -printf "%f\0")
