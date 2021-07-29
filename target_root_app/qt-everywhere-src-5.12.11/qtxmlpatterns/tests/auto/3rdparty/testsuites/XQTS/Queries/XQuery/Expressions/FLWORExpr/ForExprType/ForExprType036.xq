(: Name: ForExprType036 :)
(: Description: FLWOR expressions with type declaration (element type). Simple element wildcard :)

(: insert-start :)
declare default element namespace "http://www.example.com/typedecl";

declare variable $input-context external;
(: insert-end :)

for $test as element(*) in $input-context/root/InterleaveType/*
return $test