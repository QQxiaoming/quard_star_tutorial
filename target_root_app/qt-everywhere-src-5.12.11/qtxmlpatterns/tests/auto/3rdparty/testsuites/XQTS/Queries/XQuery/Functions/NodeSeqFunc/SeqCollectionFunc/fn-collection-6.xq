(: Name: fn-collection-6 :)
(: Description: Return the titles in the collection ordered by the title.:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $x in fn:collection($input-context)//title
order by string($x)
return $x