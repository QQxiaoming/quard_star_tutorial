(: Name: ForExprType027 :)
(: Description: FLWOR expressions with type declaration (attribute type). Wildcard match on user defined simple type on attribute :)

(: insert-start :)
import schema default element namespace "http://www.example.com/typedecl";

declare variable $input-context external;
(: insert-end :)

for $test as attribute( *,Enumeration ) in $input-context/root/UserDefinedSimpleTypeAttribute/@att
return data( $test )
