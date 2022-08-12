#!/bin/bash

akcija=$1
shift
RANDOM=42;

function gcd
{  
    if [ $1 = $2 ]; then echo $1; fi
    if [ $2 = 0 ]; then exit $1; fi
    gcd $2 $(( $1 % $2 ))
}

function fib
{
    if [ $1 = 0 ]; then echo 0; return; fi 
    if [ $1 = 1 ]; then echo 1; return; fi
    n=1
    prev=1
    tmp=0
    for (( i = 2; i < $1; i++ )); do
    tmp=$n 
    (( n = n + prev ))
    prev=$tmp
    done
    echo $n
}

function tocke
{
    array=( $vrstica )
    vpisna=${array[0]} # 1. kos vrstice

    tri=${vpisna:2:1}; # 3. znak 
    cetrti=${vpisna:3:1}; # 4. znak

    tip_P="P";
    tip_p="p";
    
    if [ "${#array[@]}" -gt 4 ] # 5. kos vrstice je lahko p ali P
    then
        if [ "${array[4]}" = "$tip_p" ] || [ "${array[4]}" = "$tip_P" ]
        then
            P_p="DA";
        fi
    fi 

    sestevek=$(( ${array[1]} + ${array[2]} + ${array[3]} ))

    # izracun sestevka glede na pogoje
    if [ "$tri" == 1 ] && [ "$cetrti" == 4 ] && [ "$P_p" == "DA" ]
    then
        sestevek=$(( $sestevek / 2 ))
    elif [ "$P_p" == "DA" ]
    then
        sestevek=$(( $sestevek / 2 ))
    elif [ "$tri" == 1 ] && [ "$cetrti" == 4 ]
    then
        sestevek=$(( $sestevek + $RANDOM % 5 + 1 ))
    fi

    # koncni sestevek
    if [ "$sestevek" -gt 50 ]
    then
        sestevek=50
    fi

    # izpis vpisne, sestevek tock
    echo -e -n "${array[0]}: $sestevek\n"
    povprecje=$(( $povprecje + $sestevek ))
    students=$(( students + 2 - 1 ))
}

function drevo 
{
    zamik="$3"
    crtice="---"
    cd "$1"  # pojdi v current directory (podano kot 1. argument)
    
    # sprehod po direktoriju
    for element in *; # * ... karkoli
    do 
        # rekurzijo uporabimo vsakic, ko pridemo do direktorija --> vstopimo vanj in se pogreznemo v globino za 1
        if [ -d "$element" ]
        then
            echo "$3$crtice-DIR  $element"
            if [ $(($2 - 1)) -ge 1 ] # pogoj (globina se ni nic)
            then
                drevo "$element" $(($2 - 1)) "$3$crtice-" # rekurzija
            fi
        elif [ -f "$element" ]
        then
            echo "$3$crtice-FILE  $element" # zamik za globino rekurzije $3 (4x _)
        elif [ -L "$element" ]
        then
            echo "$3$crtice-LINK  $element"
        elif [ -c "$element" ]
        then
            echo "$3$crtice-CHAR  $element"
        elif [ -b "$element" ]
        then
            echo "$3$crtice-BLOCK  $element"
        elif [ -p "$element" ]
        then
            echo "$3$crtice-PIPE  $element"
        elif [ -b "$element" ]
        then
            echo "$3$crtice-SOCK  $element"
        fi
    done

    cd ..  # vrni se nazaj
}

function prostor 
{
    crtice="---"
    cd "$1"
    for element in *; 
    do 
        if test -d "$element";
        then
            tempT=$(stat --format=%s "$element")
            total=$(( $total + tempT ))
            totalBlocks=$(( $totalBlocks + $(stat --format=%b "$element" ) ))
            if [ $(($2 - 1)) -ge 1 ] 
            then
                prostor "$element" $(($2 - 1)) "$3$crtice-" # rekurzija 
            fi
        elif test -f "$element";
        then
            total=$(( $total + $(wc -c "$element" | awk '{print $1}')))
            totalBlocks=$(( $totalBlocks + $(stat --format=%b "$element" ) ))
        elif test -L "$element";
        then
            tempT=$(stat --format=%s "$element")
            total=$(( $total + tempT ))
            totalBlocks=$(( $totalBlocks + $(stat --format=%b "$element" ) ))
        elif test -c "$element";
        then
            tempT=$(stat --format=%s "$element")
            total=$(( $total + tempT ))
            totalBlocks=$(( $totalBlocks + $(stat --format=%b "$element" ) ))
        elif test -b "$element";
        then
            tempT=$(stat --format=%s "$element")
            total=$(( $total + tempT ))
            totalBlocks=$(( $totalBlocks + $(stat --format=%b "$element" ) ))
        elif test -p "$element";
        then
            tempT=$(stat --format=%s "$element")
            total=$(( $total + tempT ))
            totalBlocks=$(( $totalBlocks + $(stat --format=%b "$element" ) ))
        fi
        
    done

    cd ..
}

if [ "$akcija" = "pomoc" ]
then
    echo "Uporaba: $0 akcija parametri"

elif [ "$akcija" = "status" ]
then
    a=$1
    b=$2
    gcd $a $b

elif [ "$akcija" = "leto" ]
then
    for year in $*; do
        maybe=$(  (( !(year % 4) && ( year % 100 || !(year % 400) ) )) && echo "je" || echo "ni" )
        echo "Leto $year $maybe prestopno."
    done

elif [ "$akcija" = "fib" ]
then
    for n in $*; do
        echo $n: $( fib $n )
    done

elif [ "$akcija" = "userinfo" ]
then
for user in $*; do
    echo -n "$user: "
    
    error="err"
    obstaja_podimenik="obstaja "
    enaka="enaka "
    
    uporabnikObstaja=$(grep -c "^$user:" /etc/passwd) # ce uporabnik ne obstaja takoj izpisi error
    
    if [ $uporabnikObstaja -eq 0 ]
    then
        echo "$error"
    else
        
        if [[ "$(id -u $user)" -eq "$(id -g $user)" ]]
        then 
            echo -n "$enaka"
        fi

        # obstaja podimenik z imenom uporabnika v imenikih ... 
        if [ "$(ls /home | grep ^$user)" == "$user" ] || [ "$(ls /home/uni &>/dev/null | grep ^$user)" == "$user" ]
        then 
            echo -n "$obstaja_podimenik"
        fi

        # stevilo skupin, ki jim pripada uporabnik
        numGroups=$(id -G $user | wc -w)
        echo -n "$numGroups "
        printf '\n'
    fi 
done 

elif [ "$akcija" = "tocke" ]
then
    in="/dev/stdin"
    while IFS= read -r vrstica # read line by line
    do
        # vrstico z # ignoriramo 
        [[ "$vrstica" =~ ^#.* ]] && continue
            tocke $vrstica # klic funkcije tocke
        done < "$in"
    echo -n -e "St. studentov: $students\n"
    echo -n -e "Povprecne tocke: $(($povprecje / $students))\n"

elif [ "$akcija" = "drevo" ]
then
    presledek="   "
    echo "DIR"$presledek"   $1" # imenik ... zacetni imenik obiskovanje, privzeta vrednost je trenutni delovni imenik

    if [ "$#" != 0 ] # st. argumentov je enako 0
    then
        imenik="$1"
     #  echo "imenik1: $imenik"
    else
        imenik=$(pwd)
     #  echo "imenik2: $imenik"
    fi
        
    if [ -z "$2" ] # ne-obstoj argumenta 2
    then
        globina=3 # privzeta globina je 3
      # echo "globina1: $globina"
    else
        globina="$2"  #globina ... 2. argument
      # echo "globina2: $globina"
    fi
    drevo "$imenik" "$globina"

elif [ "$akcija" = "prostor" ]
then  
    if [ "$#" != 0 ] 
    then
        imenik="$1"
    else
        imenik=$(pwd)
    fi
        
    if [ -z "$2" ] 
    then
        globina=3 
    else
        globina="$2"
    fi

    prostor "$imenik" "$globina"
    
    total=$(( $total + $(stat -c %s "$imenik" )))
    totalBlocks=$(( $totalBlocks + $(stat -c %b "$imenik" ) ))
    
    echo -n -e "Velikost: $total\n"
    echo -n -e "Blokov: $totalBlocks\n"
    echo -n -e "Prostor: $(( $totalBlocks * 512 ))\n"
else 
    echo "Napacna uporaba skripte!"
    echo "Uporaba: $0 akcija parametri"
    exit 42
fi
