(: Name: WhereExpr001 :)
(: Description: For+Where+Return - test existence of child elements in Where clause :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

if ( count( for $f in $input-context//* where $f/File return $f/File[1] ) = count( $input-context//File[1]) )
then <Same/>
else <notSame/>  

