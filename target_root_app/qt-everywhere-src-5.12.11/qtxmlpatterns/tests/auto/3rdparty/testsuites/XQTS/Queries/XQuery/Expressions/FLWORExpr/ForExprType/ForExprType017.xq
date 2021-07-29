(: Name: ForExprType017 :)
(: Description: Bound sequence is a union type containing a single instance that does not match the type declaration :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $num as xs:double in (xs:int("1"), xs:integer("2"), xs:string("3"))
return $num
