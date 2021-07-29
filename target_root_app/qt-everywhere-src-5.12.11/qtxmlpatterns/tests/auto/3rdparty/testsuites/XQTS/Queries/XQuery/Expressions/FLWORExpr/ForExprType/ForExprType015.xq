(: Name: ForExprType015 :)
(: Description: Type declaration is a super type :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $num as xs:decimal in (xs:integer(1), xs:integer(2), xs:integer(3))
return $num
