(: Name: ForExprType022 :)
(: Description: FLWOR expressions with type declaration (attribute type). Simple attribute wildcard :)

(: insert-start :)
declare default element namespace "http://www.example.com/typedecl";

declare variable $input-context external;
(: insert-end :)

for $test as attribute(*) in ( $input-context/root/InterleaveType2/@integer, $input-context/root/InterleaveType2/@decimal )
return data( $test )