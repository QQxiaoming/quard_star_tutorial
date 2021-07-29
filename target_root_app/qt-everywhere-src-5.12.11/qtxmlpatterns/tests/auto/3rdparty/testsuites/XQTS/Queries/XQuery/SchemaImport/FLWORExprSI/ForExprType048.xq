(: Name: ForExprType048 :)
(: Description: FLWOR expressions with type declaration. Non-trivial value type promotion: anyAtomicType value type -> anyAtomicType :)

(: insert-start :)
import schema default element namespace "http://www.example.com/typedecl";

declare variable $input-context external;
(: insert-end :)

for $test as xs:anyAtomicType in data( $input-context/root/anyAtomicType/@att )
return $test