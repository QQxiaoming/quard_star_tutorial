(: Name: ForExprType011 :)
(: Description: FLWOR with type expression referencing an unknown type :)

declare namespace xqt="http://www.w3.org/XQueryTestOrderBy";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $num as xs:undefinedType in data( $input-context/xqt:DataValues/xqt:NegativeNumbers/xqt:orderData )
return $num