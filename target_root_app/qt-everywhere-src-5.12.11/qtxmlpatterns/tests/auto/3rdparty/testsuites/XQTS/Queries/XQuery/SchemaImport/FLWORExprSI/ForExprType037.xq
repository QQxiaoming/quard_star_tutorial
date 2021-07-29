(: Name: ForExprType037 :)
(: Description: FLWOR expressions with type declaration (element type). Simple element name test (no type) :)

(: insert-start :)
import schema default element namespace "http://www.example.com/typedecl";

declare variable $input-context external;
(: insert-end :)

for $test as element(decimal) in $input-context/root/InterleaveType/decimal
return $test