(: Name: internalcontextitem-18 :)
(: Description: context item expression where context item used with string-length function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

("ABC", "DEF","A")[fn:string-length(.) gt 2]