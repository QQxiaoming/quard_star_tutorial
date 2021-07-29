(: Name: internalcontextitem-9 :)
(: Description: context item expression where context item is an empty sequence. :)
(: uses fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(((),(),())[xs:string(.)])