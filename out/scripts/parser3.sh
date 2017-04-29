#! /bin/bash
CARTELLA="FRUTTA"
DATABASE="tesi UNIBO"
CLASSI=(A B C D)
LARGHEZZA=300
ALTEZZA=300

if (($# == 0)) ; then
	CSV=$(ls -1 | egrep -i "\.csv$" | head -n 1)
	if ! [ $CSV ] ; then
		echo Nessun file .csv trovato nella cartella corrente
		exit
	fi
	echo $CSV
elif ! [ -f $1 ] ; then
	CSV=$(ls -1 | egrep -i "\.csv$")
	echo $1 non esiste
	exit
else
	CSV=$1
fi

function leggi_csv() {
	sed 's/\r//' < "$CSV"
}

[ -d out ] || mkdir out
rm -rf out/Annotations
rm -rf out/ImageSets
mkdir -p out/Annotations
mkdir -p out/ImageSets/Main

leggi_csv | (
	IFS=';'
	while read foto xc yc w2 h2 classe ; do
		
		if [ ! -f "out/Annotations/$foto.xml" ] ; then
			echo "<annotation>
				<folder>$CARTELLA</folder>
				<filename>$foto.jpg</filename>
				<source> 
					<database>$DATABASE</database>
				</source>
				<size>
					<width>$LARGHEZZA</width>
					<height>$ALTEZZA</height>
					<depth>3</depth>
				</size>
				<segmented>0</segmented>" > "out/Annotations/$foto.xml"
			#echo $foto > out/ImageSets/Main/${a}_elenco.txt
			echo $foto >> "out/ImageSets/Main/elenco.txt" 
		fi
		echo "	<object>
		<name>"${CLASSI[$classe]}"</name>
		<pose>Unspecified</pose>
		<truncated>0</truncated>
		<difficult>0</difficult>
		<bndbox>
			<xmin>"$(($xc - $w2))"</xmin>
			<ymin>"$(($yc - $h2))"</ymin>
			<xmax>"$(($xc + $w2))"</xmax>
			<ymax>"$(($yc + $h2))"</ymax>
		</bndbox>
	</object>" >> "out/Annotations/$foto.xml"
	done
)
(
	cd out/Annotations
	for f in * ; do
		echo "</annotation>" >> "$f"
	done
)
for i in ${!CLASSI[@]} ; do
	(
		while read foto ; do
			if leggi_csv | egrep "^$foto;" | egrep -q ";"$i"$" ; then
				echo $foto 1
			else
				echo $foto -1
			fi
		done
	) < out/ImageSets/Main/elenco.txt > out/ImageSets/Main/${CLASSI[$i]}_elenco.txt
done
#ls -1 out/JPEGImages | cut -d . -f 1 | while read foto ; do
#	leggi_csv | egrep -q "^$foto;" || echo $foto >> out/ImageSets/Main/test.txt
#done
bash divisore3.sh
rm out/ImageSets/Main/*elenco.txt
