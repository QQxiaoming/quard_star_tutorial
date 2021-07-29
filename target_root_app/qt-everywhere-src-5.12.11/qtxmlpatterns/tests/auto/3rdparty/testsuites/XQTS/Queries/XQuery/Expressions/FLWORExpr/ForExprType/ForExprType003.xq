(: Name: ForExprType003 :)
(: Description: FLWOR expressions with type declaration (PI type) :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $PI as processing-instruction() in $input-context//processing-instruction()
return $PI