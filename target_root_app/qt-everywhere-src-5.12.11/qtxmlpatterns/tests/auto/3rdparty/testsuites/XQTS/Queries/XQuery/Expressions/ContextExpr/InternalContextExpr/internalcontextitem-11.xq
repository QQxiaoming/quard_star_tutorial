(: Name: internalcontextitem-11 :)
(: Description: context item expression where context item used in subtraction operation. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(3,4,5)[(xs:integer(5) - xs:integer(.)) gt 1]