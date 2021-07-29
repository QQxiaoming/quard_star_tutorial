(: Name: internalcontextitem-20 :)
(: Description: context item expression where context item used with "min" function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(1,2,3)[fn:min((.,2)) eq 2]