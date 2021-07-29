(: Name: ForExpr028 :)
(: Description: FLWOR expression iterating over constructed XML :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $num in ( <one/>, <two/>, <three/> )
return $num