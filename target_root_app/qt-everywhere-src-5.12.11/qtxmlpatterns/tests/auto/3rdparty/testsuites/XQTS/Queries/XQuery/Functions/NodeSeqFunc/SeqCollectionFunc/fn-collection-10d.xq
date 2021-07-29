(: Name: fn-collection-10 :)
(: Description: Test that two uses of fn:collection are stable.:)

(: insert-start :)
(: insert-end :)

let $c1 := fn:collection()
let $c2 := fn:collection()
for $c at $p in $c1
return $c is exactly-one($c2[$p])
