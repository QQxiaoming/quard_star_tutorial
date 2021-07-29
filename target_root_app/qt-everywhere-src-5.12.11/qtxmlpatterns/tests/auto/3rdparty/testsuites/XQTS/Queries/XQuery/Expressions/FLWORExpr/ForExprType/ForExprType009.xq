(: Name: ForExprType009 :)
(: Description: FLWOR with type expression matching a user defined type :)

declare namespace xqt="http://www.w3.org/XQueryTestOrderBy";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $num as element(xqt:NegativeNumbers) in $input-context/xqt:DataValues/xqt:NegativeNumbers
return $num