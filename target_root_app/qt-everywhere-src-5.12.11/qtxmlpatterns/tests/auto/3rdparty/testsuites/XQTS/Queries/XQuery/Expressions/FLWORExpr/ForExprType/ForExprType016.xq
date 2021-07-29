(: Name: ForExprType016 :)
(: Description: Type declaration is a super type of the union type bound to the variable :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $num as xs:decimal in (xs:integer(1), xs:decimal(2), xs:integer(3))
return $num
