(: Name: ForExpr029 :)
(: Description: Embedded FLOWR expression that binds same variable on boths expressions (two for clauses) :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $var in (1,2) for $var in (2,2) return $var * $var