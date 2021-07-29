(: Name: ForExprType039 :)
(: Description: FLWOR expressions with type declaration (element type). Wildcard for name on matching complex, user defined type :)

(: insert-start :)
import schema default element namespace "http://www.example.com/typedecl";

declare variable $input-context external;
(: insert-end :)

for $test as element(*,InterleaveType) in $input-context/root/InterleaveType
return $test