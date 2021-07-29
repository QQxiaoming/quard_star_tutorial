(: Name: ForExpr031 :)
(: Description: FLOWR expression that binds same variable on both ends of "at" :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)


for $var at $var in (1, 2) return $var * $var