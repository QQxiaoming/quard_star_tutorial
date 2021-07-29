(: Name: fn-collection-4 :)
(: Description: Count the number of nodes in the collection.:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

count(fn:collection($input-context))