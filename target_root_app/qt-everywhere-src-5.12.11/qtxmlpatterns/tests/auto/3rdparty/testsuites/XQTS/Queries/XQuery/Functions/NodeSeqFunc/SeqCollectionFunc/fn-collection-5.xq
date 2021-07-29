(: Name: fn-collection-5 :)
(: Description: Count the number of nodes in the collection.:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

count(fn:collection($input-context))