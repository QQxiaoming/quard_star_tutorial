(: Name: ForExprType014 :)
(: Description: Type expression value is void. This will never match anything bound to a variable by a FLWOR statement :)

declare namespace xqt="http://www.w3.org/XQueryTestOrderBy";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $num as empty-sequence() in data( $input-context/xqt:DataValues/xqt:NegativeNumbers/xqt:orderData )
return $num
