(: Name: internalcontextitem-16 :)
(: Description: context item expression where context item used in a boolean (and operator)expression. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(fn:true(),fn:false(),fn:true())[xs:boolean(.) and xs:boolean(.)]