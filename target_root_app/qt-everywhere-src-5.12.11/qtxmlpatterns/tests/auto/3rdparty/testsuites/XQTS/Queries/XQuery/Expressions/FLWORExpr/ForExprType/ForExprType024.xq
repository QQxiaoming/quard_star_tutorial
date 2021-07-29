(: Name: ForExprType024 :)
(: Description: FLWOR expressions with type declaration (attribute type). anySimpleType attribute :)

(: insert-start :)
declare default element namespace "http://www.example.com/typedecl";

declare variable $input-context external;
(: insert-end :)

for $test as attribute(att, xs:anySimpleType) in $input-context/root/anyAtomicType/@att
return data( $test )
