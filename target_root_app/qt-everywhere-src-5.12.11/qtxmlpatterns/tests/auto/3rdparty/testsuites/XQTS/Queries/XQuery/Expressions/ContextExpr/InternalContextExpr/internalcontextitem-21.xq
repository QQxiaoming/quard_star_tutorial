(: Name: internalcontextitem-21 :)
(: Description: context item expression where context item used with "max" function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(1,2,3)[fn:min((.,3)) eq 3]