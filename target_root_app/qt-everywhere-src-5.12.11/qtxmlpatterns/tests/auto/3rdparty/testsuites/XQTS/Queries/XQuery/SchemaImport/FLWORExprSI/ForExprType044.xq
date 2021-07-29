(: Name: ForExprType044 :)
(: Description: FLWOR expressions with type declaration (element type). List type :)

(: insert-start :)
import schema default element namespace "http://www.example.com/typedecl";

declare variable $input-context external;
(: insert-end :)

for $test as element(ListType,xs:string) in $input-context/root/ListType
return $test