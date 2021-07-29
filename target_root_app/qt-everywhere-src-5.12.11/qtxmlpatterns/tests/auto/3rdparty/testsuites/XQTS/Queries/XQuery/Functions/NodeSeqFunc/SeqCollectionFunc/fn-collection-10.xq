(: Name: fn-collection-10 :)
(: Description: Test that two uses of fn:collection are stable.:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

let $c1 := fn:collection($input-context)
let $c2 := fn:collection($input-context)
for $c at $p in $c1
return $c is exactly-one($c2[$p])
