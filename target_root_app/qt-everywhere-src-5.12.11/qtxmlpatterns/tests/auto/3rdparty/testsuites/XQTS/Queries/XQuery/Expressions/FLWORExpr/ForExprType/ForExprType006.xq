(: Name: ForExprType006 :)
(: Description: Wrong type in type declaration :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $int as xs:string in (xs:int(1), xs:int(2))
return $int