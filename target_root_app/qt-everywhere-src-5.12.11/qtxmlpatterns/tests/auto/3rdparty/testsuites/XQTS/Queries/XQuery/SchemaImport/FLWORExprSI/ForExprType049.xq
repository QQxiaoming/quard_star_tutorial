(: Name: ForExprType049 :)
(: Description: FLWOR expressions with type declaration. Non-trivial value type promotion: Interleave value type integer|decimal -> decimal :)

(: insert-start :)
import schema default element namespace "http://www.example.com/typedecl";

declare variable $input-context external;
(: insert-end :)

for $test as xs:decimal in data($input-context/root/InterleaveType/*)
return $test
