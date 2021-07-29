(: Name: internalcontextitem-7 :)
(: Description: context item expression where context item is an xs:boolean. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(fn:true(),fn:false(),fn:true())[xs:boolean(.)]