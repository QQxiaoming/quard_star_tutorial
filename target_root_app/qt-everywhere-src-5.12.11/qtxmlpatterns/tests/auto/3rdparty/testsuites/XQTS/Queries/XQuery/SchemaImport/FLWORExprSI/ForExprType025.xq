(: Name: ForExprType025 :)
(: Description: FLWOR expressions with type declaration (attribute type). Interleave type integer|decimal -> decimal :)

(: insert-start :)
import schema default element namespace "http://www.example.com/typedecl";

declare variable $input-context external;
(: insert-end :)

for $test as attribute(*,xs:decimal) in ( $input-context/root/InterleaveType2/@integer, $input-context/root/InterleaveType2/@decimal )
return data( $test )
