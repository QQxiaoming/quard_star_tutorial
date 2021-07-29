(: Name: internalcontextitem-19 :)
(: Description: context item expression where context item used with "avg" function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(1,2,3)[fn:avg((.,2,3)) gt 2]