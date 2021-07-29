(: Name: internalcontextitem-12 :)
(: Description: context item expression where context item used in multiplication operation. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(3,4,5)[(xs:integer(.) * xs:integer(.)) gt 2]