(: Name: ForExprType051 :)
(: Description: FLWOR expressions with type declaration. Non-trivial value type promotion: List type :)

(: insert-start :)
import schema default element namespace "http://www.example.com/typedecl";

declare variable $input-context external;
(: insert-end :)

for $test as xs:string in exactly-one(data( $input-context/root/ListType ))
return $test
