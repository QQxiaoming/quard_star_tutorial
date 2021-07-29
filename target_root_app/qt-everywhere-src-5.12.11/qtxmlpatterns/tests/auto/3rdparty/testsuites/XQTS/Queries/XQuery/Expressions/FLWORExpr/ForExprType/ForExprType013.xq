(: Name: ForExprType013 :)
(: Description: Type declaration containing multiple 'as' keywords :)

declare namespace xqt="http://www.w3.org/XQueryTestOrderBy";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $num as xs:decimal as xs:decimal in data( $input-context/xqt:DataValues/xqt:NegativeNumbers/xqt:orderData )
return $num
