(: Name: ForExprType038 :)
(: Description: FLWOR expressions with type declaration (element type). Match complex, user defined type :)

(: insert-start :)
import schema default element namespace "http://www.example.com/typedecl";

declare variable $input-context external;
(: insert-end :)

for $test as element(InterleaveType,InterleaveType) in $input-context/root/InterleaveType
return $test