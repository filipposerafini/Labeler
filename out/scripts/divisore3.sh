#! /bin/bash
function dividi(){ #$1=file originale, $2=file 1, $3=file 2
	(
		while read l ; do
			echo $l >> "$2"
			read l && echo $l >> "$3"
		done
	) < "$1"
}

cd out/ImageSets/Main
dividi elenco.txt trainval.txt testset.txt
dividi trainval.txt train.txt val.txt
dividi testset.txt test.txt altro.txt
for l in A B C D ; do
	dividi $l"_elenco.txt" $l"_trainval.txt" $l"_testset.txt"
        dividi $l"_trainval.txt" $l"_train.txt" $l"_val.txt"
	dividi $l"_testset.txt" $l"_test.txt" /dev/null
done


