(: Name: ForExprType010 :)
(: Description: FLWOR with type expression matching a type from the schema :)

import schema namespace xqt="http://www.w3.org/XQueryTestOrderBy";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $num as xs:decimal in data( $input-context/xqt:DataValues/xqt:NegativeNumbers/xqt:orderData )
return $num