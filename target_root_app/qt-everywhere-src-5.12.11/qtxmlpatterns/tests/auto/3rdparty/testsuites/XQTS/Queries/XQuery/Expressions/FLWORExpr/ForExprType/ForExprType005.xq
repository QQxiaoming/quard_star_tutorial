(: Name: ForExprType005 :)
(: Description: FLWOR expressions with type declaration (simple type) :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $int as xs:int in (xs:int(1), xs:int(2))
return $int