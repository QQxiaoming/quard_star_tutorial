(: Name: ForExpr030 :)
(: Description: Embedded FLOWR expression that binds same variable on boths expressions (use commas) :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $var in (1,2), $var in (2,2) return $var * $var