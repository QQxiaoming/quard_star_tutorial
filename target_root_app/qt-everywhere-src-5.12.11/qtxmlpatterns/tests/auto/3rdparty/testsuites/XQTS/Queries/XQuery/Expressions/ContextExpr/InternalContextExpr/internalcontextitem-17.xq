(: Name: internalcontextitem-17 :)
(: Description: context item expression where context item used in a boolean (or operator)expression. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(fn:true(),fn:false(),fn:true())[xs:boolean(.) or xs:boolean(.)]