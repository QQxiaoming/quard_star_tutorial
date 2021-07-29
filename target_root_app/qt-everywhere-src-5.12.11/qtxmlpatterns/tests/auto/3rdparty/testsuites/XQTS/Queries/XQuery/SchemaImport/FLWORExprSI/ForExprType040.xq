(: Name: ForExprType040 :)
(: Description: FLWOR expressions with type declaration (element type). Match user defined simple type on element :)

(: insert-start :)
import schema default element namespace "http://www.example.com/typedecl";

declare variable $input-context external;
(: insert-end :)

for $test as element( UserDefinedSimpleType, Enumeration ) in $input-context/root/UserDefinedSimpleType
return $test