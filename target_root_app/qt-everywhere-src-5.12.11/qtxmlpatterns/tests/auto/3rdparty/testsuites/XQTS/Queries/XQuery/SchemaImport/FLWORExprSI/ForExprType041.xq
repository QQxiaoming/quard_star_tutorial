(: Name: ForExprType041 :)
(: Description: FLWOR expressions with type declaration (element type). Wildcard match on user defined simple type on element :)

(: insert-start :)
import schema default element namespace "http://www.example.com/typedecl";

declare variable $input-context external;
(: insert-end :)

for $test as element( *, Enumeration ) in $input-context/root/UserDefinedSimpleType
return $test