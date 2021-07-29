(: Name: ForExprType042 :)
(: Description: FLWOR expressions with type declaration (element type). Interleave value type integer|decimal -> decimal :)

(: insert-start :)
import schema default element namespace "http://www.example.com/typedecl";

declare variable $input-context external;
(: insert-end :)

for $test as element(*,xs:decimal) in $input-context/root/InterleaveType/*
return $test